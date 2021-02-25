#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>

#include "utils.h"
#include "binary_sem.h"

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

#define MSG_TYPE           1
#define MAX_SIZE           1024
#define QUEUE_PERM         0666
#define PAYLOAD_SIZE       MAX_SIZE - sizeof(long)
#define PATH               "/tmp/sysV.tmp"
#define PROJECT_ID         65
#define MQ_KEY_FILE        "/tmp/mq_key_file"
#define SEM_ID_FILE       "/tmp/sem_key_file"

/* these vars are used to calculate average send time only */
clock_t start, end;
double cpu_time_used_avg = 0;
double cpu_time_used;

typedef struct mesg_buffer {
    long msg_type;
    char payload[PAYLOAD_SIZE];
} ds_message;

int mq_run_client(enum time_type type, const char *out_file, int exe_cnt, int blocking, int debug, int round) {
    struct timeval **send_time = (struct timeval **)malloc(round * sizeof(struct timeval *));
    key_t key;
    ds_message message;
    int mqid;
    int semid;
    int mq_flag = blocking == 1 ? 0 : IPC_NOWAIT;
    int i = 0, ret = -1;
    int curr_round = 0;

    if (type == AVG) {
        for (int i = 0; i < round; ++i)
            send_time[i] = (struct timeval *)malloc(sizeof(struct timeval));
    } else if (type == SEND) {
        for (int i = 0; i < round; ++i)
            send_time[i] = (struct timeval *)malloc(exe_cnt * sizeof(struct timeval));
    }
    if (send_time == NULL) {
        perror("malloc");
        return 1;
    }
    key = retrieve_key(MQ_KEY_FILE);
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

    /* get semid from stored file */
    semid = retrieve_key(SEM_ID_FILE);
    ret = -1;

    if (type == AVG) {
        for ( ; curr_round < round; ++curr_round) {
            gettimeofday(send_time[curr_round], NULL);
            for (int j = 0; j < exe_cnt;) {
                ret = msgsnd(mqid, &message, MESSAGE_LEN, mq_flag);
                if (ret == 0) {
                    ++j;
                } else { // msgsnd returns error
                    if (!blocking && errno == EAGAIN) {
                        continue;
                    }
                    /* all other types of error are considered bugs and are handled here */
                    printf("msgsnd: [%s] %s\n", get_error_str(errno), strerror(errno));
                    return -1;
                }
            } // inner for
        } // outer for
    } else if (type == SEND) {
        for ( ; curr_round < round; ++curr_round) {
            /* check sem to see if okay to start sending next batch */
            while (ret != 0) {
                ret = reserve_sem(semid, 0);
            }
            //printf("sending in round %d\n", curr_round);
            for (int j = 0; j < exe_cnt;) {
                gettimeofday(&send_time[curr_round][j], NULL);
                ret = msgsnd(mqid, &message, MESSAGE_LEN, mq_flag);
                if (ret == 0) {
                    ++j;
                } else { // msgsnd returns error
                    if (!blocking && errno == EAGAIN) {
                        continue;
                    }
                    /* all other types of error are considered bugs and are handled here */
                    printf("msgsnd: [%s] %s\n", get_error_str(errno), strerror(errno));
                    return -1;
                }
            } // for
            //printf("round %d cnt 1: time=%ld.%ld\n", curr_round,
            //        send_time[curr_round][1].tv_sec, send_time[curr_round][1].tv_usec);
        }
    } else {
        printf("[ERROR] client incompatible with type recv, use send/avg instead\n");
        return 1;
    }
    for (int i = 0; i < round; ++i) {
        char *round_out_file = get_indexed_filename(out_file, i);
        write_timestamp_to_file(type, round_out_file, send_time[i], exe_cnt);
    }

    /* cleanup */
    for (int i = 0; i < round; ++i)
        free(send_time[i]);
    free(send_time);
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

int mq_run_server(int exe_cnt, enum time_type type, const char *out_file, int blocking, int debug, int round) {
    struct timeval **recv_time = (struct timeval **)malloc(round * sizeof(struct timeval *));
    ds_message message;
    key_t key;
    int mqid;
    int mq_flag = blocking == 1 ? 0 : IPC_NOWAIT;
    int ret = -1;
    int curr_round = 0;

    if (type == AVG) {
        for (int i = 0; i < round; ++i)
            recv_time[i] = (struct timeval *)malloc(sizeof(struct timeval));
    } else if (type == RECV) {
        for (int i = 0; i < round; ++i)
            recv_time[i] = (struct timeval *)malloc(exe_cnt * sizeof(struct timeval));
    }
    if (recv_time == NULL) {
        perror("malloc");
        return 1;
    }

    /* ftok to generate unique key */
    key = ftok(PATH, PROJECT_ID);
    if (key == -1) {
        printf("%s: key=-1 %s\n", __func__, strerror(errno));
        return -1;
    } else {
        do {
            ret = store_key(MQ_KEY_FILE, key);
        } while (ret != 0);
    }
    /* msgget creates a message queue and returns identifier */
    mqid = msgget(key, QUEUE_PERM | IPC_CREAT | IPC_EXCL);
    if (mqid == -1) {
        printf("%s: mqid=-1 %s\n", __func__, strerror(errno));
        return -1;
    }

    printf("About to create sem\n");
    /* create a semaphore to sync send/receive with client */
    int semid = semget(IPC_PRIVATE, 1, IPC_CREAT | IPC_EXCL | PERMS);

    /* check if semid is valid, if so make it available for client to start sending */
    if (semid != -1) {
        ret = init_sem_available(semid, 0);
        if (ret != 0) {
            printf("[ERR] init_sem_available\n");
        }
    } else {
        printf("[ERR] semget\n");
        return -1;
    }

    /* store semid in file so client can obtain it and use the semaphore */
    ret = 0;
    do {
        ret = store_key(SEM_ID_FILE, semid);
    } while (ret != 0);

    printf("Created sem\n");
    if (type == AVG) {
        for (; curr_round < round; curr_round++) {
            for (int j = 0; j != exe_cnt;) {
                ssize_t bytes_read = -1;
                /* msg_type = 0 simply retrieves the first message in queue */
                ret = msgrcv(mqid, &message, MESSAGE_LEN, 0, mq_flag);
                if (ret == MESSAGE_LEN) {
                    if (debug) {
                        printf("Received: %s \n", message.payload);
                    }
                    ++j;
                } else if (ret == 0) {
                    // nothing in queue just keep looping
                } else if (ret > 0) {
                   printf("[ERROR] %s: partial read detected: %d\n", __func__, ret);
                } else { // msgrcv returns error
                    if (!blocking && errno == ENOMSG) {
                        continue;
                    }
                    /* all other types of error are considered bugs and are handled here */
                    printf("msgrcv: [%s] %s\n", get_error_str(errno), strerror(errno));
                    return -1;
                }
            }
            gettimeofday(recv_time[curr_round], NULL);
        }
    } else if (type == RECV) {
        for (; curr_round < round; curr_round++) {
            //printf("current round = %d\n", curr_round);
            for (int j = 0; j != exe_cnt;) {
                ssize_t bytes_read = -1;
                /* msg_type = 0 simply retrieves the first message in queue */
                ret = msgrcv(mqid, &message, MESSAGE_LEN, 0, mq_flag);
                gettimeofday(&recv_time[curr_round][j], NULL);
                if (ret == MESSAGE_LEN) {
                    if (debug) {
                        printf("Received: %s \n", message.payload);
                    }
                    ++j;
                } else if (ret == 0) {
                    // nothing in queue just keep looping
                } else if (ret > 0) {
                   printf("[ERROR] %s: partial read detected: %d\n", __func__, ret);
                } else { // msgrcv returns error
                    if (!blocking && errno == ENOMSG) {
                        continue;
                    }
                    /* all other types of error are considered bugs and are handled here */
                    printf("msgrcv: [%s] %s\n", get_error_str(errno), strerror(errno));
                    return -1;
                }
            }
            /* up the sem to notify client to send the next batch */
            ret = release_sem(semid, 0);
            if (ret == -1) {
                printf(" --> sem up failed\n");
            } else {
                printf(" -->  sem up one\n");
            }
        }
    }
    for (int i = 0; i < round; ++i) {
        char *round_out_file = get_indexed_filename(out_file, i);
        write_timestamp_to_file(type, round_out_file, recv_time[i], exe_cnt);
    }

    /* cleanup */
    for (int i = 0; i < round; ++i)
        free(recv_time[i]);
    free(recv_time);
    if (debug) {
        display_mq_ds_info();
        printf("Server removing queue now!\n");
    }
    msgctl(mqid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID, 0);
    remove(MQ_KEY_FILE);
    remove(SEM_ID_FILE);
    return 0;
}

void mq_print_attr()
{

}

static void usage(const char *prog) {
    printf("-------------------------------------------------------------------------\n");
    printf("%s --execution-count <execCnt> --mode <mode> --timing-type <type> --output-file <fileName> [--blocking] --round <num_of_rounds> [--debug-flag]\n", prog);
    printf("mode:\n");
    printf("\tserv  - run as server\n");
    printf("\tcli   - run as client\n");
    printf("\tattr  - check attribute of the message queue\n");
    printf("timing-type:\n");
    printf("\tsend  - if run as client, timestamp right before send\n");
    printf("\tavg   - if run as client, compute average send time\n");
    printf("\trecv  - only used for server, timestamp after reception\n");
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
        { "round", required_argument, NULL, 'd'           },
        { "help", no_argument, NULL, 'h'                  },
        {  NULL, 0, NULL, '\0'}};
    int opt_idx = 0, option = 0, exe_cnt = 0, ret = 0;
    char out_file[MAX_FILE_NAME_LEN];
    enum run_mode mode;
    enum time_type type;
    int blocking = 0;
    int debug_flag = 0;
    int round = 1;
    int opt_flag = 0;
    while ((option = getopt_long(argc, argv, "e:m:t:o:d:bgh", longOpts,
                &opt_idx)) != -1) {
        opt_flag = 1;
        switch (option) {
            case 'd':
                round = atoi(optarg);
                if (round > MAX_ROUND) {
                    printf("[ERROR] Round number must not exceed %d\n", MAX_ROUND);
                    return 1;
                }
                break;
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

    if (!is_file_exists(PATH)) {
        FILE *fd = NULL;
        fd = fopen(PATH, "w");
        if (fd != NULL) {
            fclose(fd);
        }
    }

    if (mode == SERV) {
        ret = mq_run_server(exe_cnt, type, out_file, blocking, debug_flag, round);
    } else if (mode == CLI) {
        ret = mq_run_client(type, out_file, exe_cnt, blocking, debug_flag, round);
    } else if (mode == ATTR) {

    } else {
        printf("[ERROR] You should never see this, something is terribly wrong\n");
        return 1;
    }
    if (ret != 0) {
        printf("[ERROR] Something went wrong in this run! ret = %d\n", ret);
    }
    return 0;
}
