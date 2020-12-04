#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include <sys/time.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>

enum run_mode {SERV = 1, CLI = 2, ATTR = 3};
enum time_type {AVG = 0, SEND = 1, RECV = 2};

/*
 * Note: Effetct of blocking/non-blocking on msgsnd() and msgrcv().
 *
 * If blocking is set, flag IPC_NOWAIT is specified during call to
 * msgsnd() or msgrcv() depending on whether it's client or server.
 *
 * Specifically, IPC_NOWAIT has the following effects
 *
 *                            - client -
 * Performs non-blocking send. If a message queue is full, msgsnd()
 * blocks until enough space becomes available to allow the message to
 * be placed on the queue. However, if the flag is spefied, then msgsnd()
 * returns immediately with the error EAGAIN
 *
 *                            - server-
 * Performs non-blocking recv so, when specified, msgrcv() returns immediately
 * if no message matching msg_type is in the queue with erro ENOMSG (not EAGAIN)
 *
 * Both blocking msgsnd() and msgrcv(), when interrupted by a signal handler,
 * fail with the error EINTR, regardless of the setting of SA_RESTART flag when
 * the signal handler was established.
 *
 */

/*
 * Note on MQ_FLAG for IPC message queue:
 * use 0 | IPC_NOWAIT | MSG_EXCEPT | MSG_NOERROR
 * IPC_NOWAIT:  performs non-blocking receive
 * MSG_EXCEPT:  effective if msg_type > 0
 * MSG_NOERROR: truncates message if it exceeds size limit
 *
 */

#define MAX_FILE_NAME_LEN  32
#define MSG_TYPE           1
#define ITER_COUNT         1000000
#define MAX_SIZE           1024
#define MESSAGE_LEN        512
#define QUEUE_PERM         0666
#define PAYLOAD_SIZE       MAX_SIZE - sizeof(long)
#define PATH               "/var/tmp/progfile"
#define PROJECT_ID         65
#define MQ_KEY_FILE        "mq_key_file"

/* these vars are used to calculate average send time only */
clock_t start, end;
double cpu_time_used_avg = 0;
double cpu_time_used;

typedef struct mesg_buffer {
    long msg_type;
    char payload[PAYLOAD_SIZE];
} ds_message;

static inline const char *get_error_str(int error_no) {
    switch (error_no) {
        case EAGAIN:
            return "EAGAIN";
        case EACCES:
            return "EACCES";
        case EFAULT:
            return "EFAULT";
        case EIDRM:
            return "EIDRM";
        case EINTR:
            return "EINTR";
        case EINVAL:
            return "EINVAL";
        case ENOMEM:
            return "ENOMEM";
        case E2BIG:  // msgrcv only
            return "E2BIG";
        case ENOSYS: // msgrcv only
            return "ENOSYS";
        default:
            return "ERROR TYPE NOT FOUND";
    } // switch
}

static inline int store_key(key_t key) {
    FILE *fp;
    fp = fopen(MQ_KEY_FILE, "w");
    if (fp == NULL) {
        printf("[ERROR] failed writing key to file %s\n", MQ_KEY_FILE);
        return -1;
    }
    fprintf(fp, "%d", key);
    fclose(fp);
    return 0;
}

static inline key_t retrieve_key() {
    FILE *fp;
    key_t key;
    int file_exists = -1;
    do {
        file_exists = access(MQ_KEY_FILE, F_OK);
    } while (file_exists);
    fp = fopen(MQ_KEY_FILE, "r");
    if (fp == NULL) {
        printf("[ERROR] failed retrieving key from file %s\n", MQ_KEY_FILE);
        return -1;
    }
    fscanf(fp, "%d", &key);
    fclose(fp);
    return key;
}

int mq_run_client(enum time_type type, const char *out_file, int exe_cnt, int blocking, int debug) {
    struct timeval *send_time = malloc(exe_cnt * sizeof(struct timeval));
    FILE *out_fp;
    key_t key;
    ds_message message;
    int mqid;
    int mq_flag = blocking == 1 ? 0 : IPC_NOWAIT;
    int iter = 0, i = 0, ret = -1, error_code = 0;

    key = retrieve_key();
    if (key == -1) {
        return -1;
    }

    /* msgget retrieves mqid from given key */
    mqid = msgget(key, QUEUE_PERM | IPC_CREAT);
    if (mqid == -1) {
        printf("%s: mqid=-1 %s\n", __func__, strerror(errno));
        return -1;
    }

    message.msg_type = MSG_TYPE; // must be greater than 0
    memset(message.payload, '!', MESSAGE_LEN);
    if (type == AVG) {
        for (; iter < exe_cnt; iter++) {
            start = clock();
            for (i = 0; i != ITER_COUNT; i++) {
                msgsnd(mqid, &message, MESSAGE_LEN, 0);
                end = clock();
                cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
                fprintf(out_fp, "send time = %f\n", cpu_time_used);
                cpu_time_used_avg += cpu_time_used;
            }
            fprintf(out_fp, "average send time = %f\n", cpu_time_used_avg / exe_cnt);
        }
    } else if (type == SEND) {
        for (; iter < exe_cnt;) {
            error_code = 0;
            gettimeofday(&send_time[iter], NULL);
            ret = msgsnd(mqid, &message, MESSAGE_LEN, mq_flag);
            error_code = errno;
            if (ret == 0) {
                ++iter;
            } else { // msgsnd returns error
                if (!blocking && error_code == EAGAIN) {
                    continue;
                }
                /* all other types of error are considered bugs and are handled here */
                printf("msgsnd: [%s] %s\n", get_error_str(error_code), strerror(error_code));
                return -1;
            }
        }
        /* write send timestamp to file */
        out_fp = fopen(out_file, "w");
        if (out_fp == NULL) {
            printf("[ERROR] failed opening file %s\n", out_file);
            return -1;
        }
        for (iter = 0; iter != exe_cnt; ++iter) {
            fprintf(out_fp, "%ld %ld\n", send_time[iter].tv_sec, send_time[iter].tv_usec);
        }
        fclose(out_fp);
        free(send_time);
    } else {
        printf("[ERROR] client incompatible with type recv, use send/avg instead\n");
        return 1;
    }
    return 0;
}

/* code fragments written for single queue use only */
static inline void display_mq_ds_info() {
    struct msginfo info;
    struct msqid_ds ds;
    int mq;
    if ((mq = msgctl(0, MSG_INFO, (struct msqid_ds *) &info)) < 0) {
        printf("[ERROR] msgctl info\n");
    } else if (msgctl(mq, MSG_STAT, &ds) < 0) {
        printf("[ERROR] msgctl MSG_STAT\n");
    } else {
        printf("---------------------------------------------------------\n");
        printf("[INFO] Current number of queues existent on system: %d\n", info.msgpool);
        printf("[INFO] Maximum number of entries in message map: %d\n", info.msgmap);
        printf("[INFO] Maximum number of bytes that can be written to queue: %d\n", info.msgmnb);
        printf("[INFO] Maximum bytes for single message: %d\n", info.msgmax);
        printf("[MQ DS] Current number of messages in queue: %lu\n", ds.msg_qnum);
        printf("[MQ DS] Current number of bytes in queue %lu\n", ds.__msg_cbytes);
        printf("[MQ DS] Maximum number of bytes allowed in queue: %lu\n", ds.msg_qbytes);
        printf("---------------------------------------------------------\n");
    }
}

int mq_run_server(int exe_cnt, enum time_type type, const char *out_file, int blocking, int debug) {
    struct timeval *recv_time = malloc(exe_cnt * sizeof(struct timeval));
    FILE *out_fp;
    ds_message message;
    key_t key;
    int mqid;
    int iter = 0;
    int mq_flag = blocking == 1 ? 0 : IPC_NOWAIT;
    int ret = -1;

    /* ftok to generate unique key */
    key = ftok(PATH, PROJECT_ID);
    if (key == -1) {
        printf("%s: key=-1 %s\n", __func__, strerror(errno));
        return -1;
    } else {
        do {
            ret = store_key(key);
        } while (ret != 0);
    }
    /* msgget creates a message queue and returns identifier */
    mqid = msgget(key, QUEUE_PERM | IPC_CREAT | IPC_EXCL);
    if (mqid == -1) {
        printf("%s: mqid=-1 %s\n", __func__, strerror(errno));
        return -1;
    }
    while (iter != exe_cnt) {
        ssize_t bytes_read = -1;
        int error_code = 0;
        /* msg_type = 0 simply retrieves the first message in queue */
        ret = msgrcv(mqid, &message, MESSAGE_LEN, 0, mq_flag);
        gettimeofday(&recv_time[iter], NULL);
        error_code = errno;
        if (ret == MESSAGE_LEN) {
            if (debug) {
                printf("Received: %s \n", message.payload);
            }
            ++iter;
        } else if (ret == 0) {
            // nothing in queue just keep looping
        } else if (ret > 0) {
           printf("[ERROR] %s: partial read detected: %d\n", __func__, ret);
        } else { // msgrcv returns error
            if (!blocking && error_code == ENOMSG) {
                continue;
            }
            /* all other types of error are considered bugs and are handled here */
            printf("msgrcv: [%s] %s\n", get_error_str(error_code), strerror(error_code));
            break;
        }
    }
    /* write timestamps to file */
    out_fp = fopen(out_file, "w");
    if (out_fp == NULL) {
        printf("[ERROR] failed opening file %s\n", out_file);
        return -1;
    }
    for (iter = 0; iter != exe_cnt; ++iter) {
        fprintf(out_fp, "%ld %ld\n", recv_time[iter].tv_sec, recv_time[iter].tv_usec);
    }
    /* cleanup */
    fclose(out_fp);
    free(recv_time);
    if (debug) {
        display_mq_ds_info();
        printf("Server removing queue now!\n");
    }
    msgctl(mqid, IPC_RMID, NULL);
    remove(MQ_KEY_FILE);
    return 0;
}

void mq_print_attr()
{

}

static void usage(const char *prog) {
    printf("-------------------------------------------------------------------------\n");
    printf("%s --execution-count <execCnt> --mode <mode> --timing-type <type> --output-file <fileName> [--blocking] [--debug-flag]\n", prog);
    printf("mode:\n");
    printf("\tserv - run as server\n");
    printf("\tcli  - run as client\n");
    printf("\tattr - check attribute of the message queue\n");
    printf("timing-type:\n");
    printf("\tsend - if run as client, timestamp right before send\n");
    printf("\tavg  - if run as client, compute average send time\n");
    printf("\trecv - only used for server, timestamp after reception\n");
    printf("-------------------------------------------------------------------------\n");
    printf("Example usage for server with blocking recv: %s -e 100 -m serv -t recv -o posixRecv.out -b\n", prog);
    printf("Example usage for client with blocking send: %s -e 100 -m cli -t send -o posixSend.out -b\n", prog);
    printf("-------------------------------------------------------------------------\n");
}

int main(int argc, char **argv)
{
    struct option longOpts[] = {
        { "execution-count", required_argument, NULL, 'e' },
        { "mode", required_argument, NULL, 'm'            },
        { "timing-type", required_argument, NULL, 't'     },
        { "output-file", required_argument, NULL, 'o'     },
        { "blocking", no_argument, NULL, 'b'              },
        { "debug_flag", no_argument, NULL, 'g'            },
        { "help", no_argument, NULL, 'h'                  },
        {  NULL, 0, NULL, '\0'}};
    int opt_idx = 0, option = 0, exe_cnt = 0, ret = 0;
    char out_file[MAX_FILE_NAME_LEN];
    enum run_mode mode;
    enum time_type type;
    int blocking = 0;
    int debug_flag = 0;
    int opt_flag = 0;
    while ((option = getopt_long(argc, argv, "e:m:t:o:bgh", longOpts,
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
                    printf("[ERROR] Unsupported mode, please use serv/cli/attr\n");
                    return 1;
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
                    printf("[ERROR] Unsupported mode, please use send/avg/recv\n");
                    return 1;
                }
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
                return 0;
            default:
                usage(argv[0]);
                return 1;
        } // switch
    } // while
    if (!opt_flag) {
        usage(argv[0]);
        return 1;
    }
    if (mode == SERV) {
        ret = mq_run_server(exe_cnt, type, out_file, blocking, debug_flag);
    } else if (mode == CLI) {
        ret = mq_run_client(type, out_file, exe_cnt, blocking, debug_flag);
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
