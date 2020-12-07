#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <mqueue.h>
#include <time.h>
#include <sys/time.h>
#include <getopt.h>

enum run_mode {SERV = 1, CLI = 2, ATTR = 3};
enum time_type {AVG = 0, SEND = 1, RECV = 2};

#define MAX_FILE_NAME_LEN  32
#define MESSAGE_LEN        512
#define ITER_COUNT         1000000
#define QUEUE_NAME         "/test_posix_queue"
#define QUEUE_PERM         0660

#define CHECK(x) \
    do { \
        if (!(x)) { \
            fprintf(stderr, "%s:%d: ", __func__, __LINE__); \
            perror(#x); \
            exit(-1); \
        } \
    } while (0) \

/* Note: Effect of blocking/non-blocking on mq_send() and mq_receive()
 *
 * The flag value O_NONBLOCK causes the queue to be opened in non-blocking mode.
 * If a subsequent call to mq_receive() or mq_send() can't be performed without
 * blocking, the call will fail immediately with the error EAGAIN.
 *
 */

static inline const char *get_error_str(int errno_no) {
    switch (errno_no) {
        case EINVAL:
            return "EINVAL";

        /* errors returned by mq_open() */
        case EACCES:
            return "EACCES";
        case EEXIST:
            return "EEXIST";
        case ENOMEM:
            return "ENOMEM";
        case ENOSPC:
           return "ENOSPC";
        case EMFILE:
           return "EMFILE";
        case ENOENT:
           return "ENOENT";

        /* errors returned by mq_send() and mq_receive() */
        case EAGAIN:
           return "EAGAIN";
        case EBADF:
           return "EBADF";
        case EINTR:
           return "EINTR";
        case EMSGSIZE:
           return "EMSGSIZE";
        case ETIMEDOUT:
           return "ETIMEDOUT";
    } // switch
}

void write_timestamp_to_file(enum time_type type, const char *out_file, const struct timeval *timestamp, int exe_cnt) {
    int iter = 0;
    FILE *out_fp = fopen(out_file, "w");
    if (out_fp == NULL) {
        printf("[ERROR] failed opening file %s\n", out_file);
    }
    if (type == AVG) {
        fprintf(out_fp, "%ld %ld\n", timestamp->tv_sec, timestamp->tv_usec);
    } else if (type == SEND || type == RECV) {
        for (; iter != exe_cnt; ++iter) {
            fprintf(out_fp, "%ld %ld\n", timestamp[iter].tv_sec, timestamp[iter].tv_usec);
        }
    }
    fclose(out_fp);
    return;
}

int mq_run_server(int exe_cnt, enum time_type type, const char *out_file, const int msg_size, const int max_msgs, int blocking, int debug)
{
    struct timeval *recv_time = NULL;
    mqd_t mq;
    struct mq_attr attr;
    char *buffer;
    int iter = 0;
    if (type == AVG) {
        recv_time = malloc(sizeof(struct timeval));
    } else if (type == RECV) {
        recv_time = malloc(exe_cnt * sizeof(struct timeval));
    }
    buffer = malloc(msg_size);
    if (buffer == NULL || recv_time == NULL) {
        perror("malloc");
        return 1;
    }

    /* initialize the queue attributes */
    attr.mq_flags |= blocking == 1 ? 0 : O_NONBLOCK;
    attr.mq_maxmsg = max_msgs;
    attr.mq_msgsize = msg_size;
    attr.mq_curmsgs = 0;

    /* create the message queue */
    mq = mq_open(QUEUE_NAME, O_CREAT | O_RDONLY, QUEUE_PERM, &attr);
    if (debug) {
        if (mq < 0) {
            printf("[%s] failed starting server\n", __FILE__);
            perror("mq_open");
            return -1;
        } else {
            printf("[%s] start server\n", __FILE__);
        }
    }
    CHECK((mqd_t)-1 != mq);

    if (type == RECV) {
        for (; iter < exe_cnt;) {
            size_t bytes_read = -1;
            bytes_read = mq_receive(mq, buffer, msg_size, NULL);
            gettimeofday(&recv_time[iter], NULL);
            if (bytes_read == MESSAGE_LEN) {
                if (debug) {
                    printf("[%s] Received: %s\n", __FILE__, buffer);
                }
                ++iter;
            } else if (bytes_read == 0) {
                // nothing in queue just keep looping
            } else {
                if (!blocking && errno == EAGAIN) {
                    continue;
                }
                printf("mq_receive: [%s] %s\n", get_error_str(errno), strerror(errno));
                return 1;
            }
        } // for
    } else if (type == AVG) {
        for (; iter < exe_cnt;) {
            size_t bytes_read = -1;
            bytes_read = mq_receive(mq, buffer, msg_size, NULL);
            if (bytes_read == MESSAGE_LEN) {
                if (debug) {
                    printf("[%s] Received: %s\n", __FILE__, buffer);
                }
                ++iter;
            } else if (bytes_read == 0) {
                // nothing in queue just keep looping
            } else {
                if (!blocking && errno == EAGAIN) {
                    continue;
                }
                printf("mq_receive: [%s] %s\n", get_error_str(errno), strerror(errno));
                return 1;
            }
        } // for
        gettimeofday(recv_time, NULL);
    }
    write_timestamp_to_file(type, out_file, recv_time, exe_cnt);

    /* cleanup */
    free(recv_time);
    free(buffer);
    CHECK((mqd_t)-1 != mq_close(mq));
    //CHECK((mqd_t)-1 != mq_unlink(QUEUE_NAME));

    return 0;
}

int mq_run_client(int exe_cnt, enum time_type type, const char *out_file, const int msg_size, const int max_msgs, int blocking, int debug)
{
    struct mq_attr attr;
    struct timeval *send_time = NULL;
    mqd_t mq = -1;
    char *buffer;
    int iter = 0, i = 0;
    int ret = -1;

    if (type == AVG) { // used for computing average send time
        send_time = malloc(sizeof(struct timeval));
    } else if (type == SEND) { // used for timestamping send time
        send_time = malloc(exe_cnt * sizeof(struct timeval));
    }
    buffer = malloc(msg_size);
    if (buffer == NULL || send_time == NULL) {
        perror("malloc");
        return 1;
    }
    memset(buffer, '!', MESSAGE_LEN);

    /* initialize queue attributes */
    attr.mq_flags |= blocking == 1 ? 0 : O_NONBLOCK;
    attr.mq_maxmsg = max_msgs;
    attr.mq_msgsize = msg_size;
    attr.mq_curmsgs = 0;

    /* open existing message queue */
    do {
        mq = mq_open(QUEUE_NAME, O_CREAT | O_WRONLY, QUEUE_PERM, &attr);
        if (mq < 0) {
            printf("%s: failed opening mq with %s\n", __func__, get_error_str(errno));
        }
    } while (mq < 0);

    CHECK((mqd_t)-1 != mq);
    if (debug) {
        if (mq < 0) {
            printf("[%s] failed starting client\n", __FILE__);
            return -1;
        } else {
            printf("[%s] start client\n", __FILE__);
        }
    }

    if (type == AVG) {
        gettimeofday(send_time, NULL);
        for (; iter < exe_cnt;) {
            ret = mq_send(mq, buffer, MESSAGE_LEN, 0);
            if (ret == 0) {
                ++iter;
            } else {
                if (!blocking && errno == EAGAIN) {
                    continue;
                }
                printf("mq_send: [%s] %s\n", get_error_str(errno), strerror(errno));
                return -1;
            }
        } // for
    } else if (type == SEND) {
        for (; iter < exe_cnt;) {
            gettimeofday(&send_time[iter], NULL);
            ret = mq_send(mq, buffer, MESSAGE_LEN, 0);
            if (ret == 0) {
                ++iter;
            } else {
                if (!blocking && errno == EAGAIN) {
                    continue;
                }
                printf("mq_send: [%s] %s\n", get_error_str(errno), strerror(errno));
                return -1;
            }
        } // for
    } else {
        printf("[ERROR] client incompatible with type recv, use send/avg instead\n");
        return 1;
    }
    write_timestamp_to_file(type, out_file, send_time, exe_cnt);

    /* cleanup */
    free(send_time);
    CHECK((mqd_t)-1 != mq_close(mq));
    mq_unlink(QUEUE_NAME);
    return 0;
}

void mq_print_attr()
{
    struct mq_attr attr;
    mqd_t q = mq_open("/mqtest",O_RDWR|O_CREAT,0660,NULL);
    if(q == -1) {
        perror("mq_open");
        exit(-1);
    }
    if (mq_getattr(q, &attr) == -1) {
        perror("mq_getattr");
        exit(-1);
    }
    printf("mq_flags %ld\n",  attr.mq_flags);
    printf("mq_maxmsg %ld\n", attr.mq_maxmsg);
    printf("mq_msgsize %ld\n",attr.mq_msgsize);
    printf("mq_curmsgs %ld\n",attr.mq_curmsgs);
    mq_close(q);
    mq_unlink("/mqtest");
}

static void usage(const char *prog) {
    printf("-------------------------------------------------------------------------\n");
    printf("%s --execution-count <execCnt> --mode <mode> --timing-type <type> --message-size <size> --max-num-mesg <num> --output-file <fileName> [--blocking] [--debug-flag]\n", prog);
    printf("mode:\n");
    printf("\tserv - run as server\n");
    printf("\tcli  - run as client\n");
    printf("\tattr - check attribute of the message queue\n");
    printf("timing-type:\n");
    printf("\tsend - if run as client, timestamp right before send\n");
    printf("\tavg  - if run as client, compute average send time\n");
    printf("\trecv - only used for server, timestamp after reception\n");
    printf("-------------------------------------------------------------------------\n");
    printf("Example usage for server: %s -e 100 -m serv -t recv -s 1024 -n 10 -o posixRecv.out\n", prog);
    printf("Example usage for client: %s -e 100 -m cli -t send -s 1024 -n 10 -o posixSend.out\n", prog);
    printf("-------------------------------------------------------------------------\n");
}

int main(int argc, char **argv)
{
    struct option longOpts[] = {
        { "execution-count", required_argument, NULL, 'e' },
        { "mode", required_argument, NULL, 'm'            },
        { "timing-type", required_argument, NULL, 't'     },
        { "message-size", required_argument, NULL, 's'    },
        { "max-num-mesg", required_argument, NULL, 'n'    },
        { "output-file", required_argument, NULL, 'o'     },
        { "blocking", no_argument, NULL, 'b'              },
        { "debug_flag", no_argument, NULL, 'g'            },
        { "help", no_argument, NULL, 'h'                  },
        {  NULL, 0, NULL, '\0'}};
    int opt_idx = 0, option = 0, exe_cnt = 0, ret = 0;
    char out_file[MAX_FILE_NAME_LEN];
    int msg_size;
    int max_msgs;
    enum run_mode mode;
    enum time_type type;
    int blocking = 0;
    int debug_flag = 0;
    int opt_flag = 0;
    while ((option = getopt_long(argc, argv, "e:m:t:s:n:o:bgh", longOpts,
                &opt_idx)) != -1) {
        opt_flag = 1;
        switch (option) {
            case 'e':
                exe_cnt = atoi(optarg);
                break;
            case 'm':
                if (strcmp("serv", optarg) == 0) {
                    mode = SERV;
                } else if (strcmp("cli", optarg) == 0) {
                    mode = CLI;
                } else if (strcmp("attr", optarg) == 0) {
                    mode = ATTR;
                } else {
                    printf("Unsupported mode, please use serv/cli/attr\n");
                    exit(0);
                }
                break;
            case 't':
                if (strcmp("send", optarg) == 0) {
                    type = SEND;
                } else if (strcmp("avg", optarg) == 0) {
                    type = AVG;
                } else if (strcmp("recv", optarg) == 0) {
                    type = RECV;
                } else {
                    printf("Unsupported mode, please use send/avg/recv\n");
                    exit(0);
                }
                break;
            case 's':
                msg_size = atoi(optarg);
                break;
            case 'n':
                max_msgs = atoi(optarg);
                break;
            case 'o':
                strcpy(out_file, optarg);
                break;
            case 'b':
                blocking = 1;
                break;
            case 'g':
                debug_flag = 1;
                break;
            case 'h':
                usage(argv[0]);
                exit(0);
            default:
                usage(argv[0]);
                exit(0);
        } // switch
    } // while
    if (!opt_flag) {
        usage(argv[0]);
        return 1;
    }
    if (mode == SERV) {
        ret = mq_run_server(exe_cnt, type, out_file, msg_size, max_msgs, blocking, debug_flag);
    } else if (mode == CLI) {
        ret = mq_run_client(exe_cnt, type, out_file, msg_size, max_msgs, blocking, debug_flag);
    } else if (mode == ATTR) {

    } else {
        printf("[ERROR] You should never see this, something is terribly wrong\n");
        return 1;
    }
    if (ret != 0) {
        printf("[ERROR] Something went wrong in this run!\n");
    }
    return 0;
}
