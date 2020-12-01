#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include <sys/time.h>
#include <getopt.h>

enum run_mode {SERV = 1, CLI = 2, ATTR = 3};
enum time_type {AVG = 0, SEND = 1, RECV = 2};

/*
 * Note on MQ_FLAG for IPC message queue:
 * use 0 | IPC_NOWAIT | MSG_EXCEPT | MSG_NOERROR
 * IPC_NOWAIT:  performs non-blocking receive
 * MSG_EXCEPT:  effective if msg_type > 0
 * MSG_NOERROR: truncates message if it exceeds size limit
 *
 */

#define MAX_FILE_NAME_LEN  32
#define MSG_TYPE           0
#define ITER_COUNT         1000000
#define MAX_SIZE           1024
#define QUEUE_PERM         0666
#define PAYLOAD_SIZE       MAX_SIZE - sizeof(long)
#define MESSAGE            "hello"
#define PATH               "/var/tmp/progfile"
#define PROJECT_ID         65

/* these vars are used to calculate batch send time only */
clock_t start, end;
double cpu_time_used_avg = 0;
double cpu_time_used;

struct mesg_buffer {
    long msg_type;
    char payload[PAYLOAD_SIZE];
} message;

int mq_run_client(enum time_type type, const char *out_file, int exe_cnt, int debug) {
    struct timeval send_time;
    FILE *out_fp;
    key_t key;
    int msgid;
    int e = 0, i = 0;

    /* ftok to generate unique key */
    key = ftok(PATH, PROJECT_ID);

    /* msgget creates a message queue and returns identifier */
    msgid = msgget(key, QUEUE_PERM | IPC_CREAT);
    message.msg_type = 1; // must be greater than 0

    out_fp = fopen(out_file, "w");
    if (out_fp == NULL) {
        printf("[ERROR] failed opening file %s\n", out_file);
        return 1;
    }

    strcpy(message.payload, MESSAGE);
    if (type == AVG) {
        for (; e < exe_cnt; e++) {
            start = clock();
            for (i = 0; i != ITER_COUNT; i++) {
                msgsnd(msgid, &message, PAYLOAD_SIZE, 0);
                end = clock();
                cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
                fprintf(out_fp, "send time = %f\n", cpu_time_used);
                cpu_time_used_avg += cpu_time_used;
            }
            fprintf(out_fp, "average send time = %f\n", cpu_time_used_avg / exe_cnt);
        }
    } else if (type == SEND) {
        for (; e < exe_cnt; ++e) {
            gettimeofday(&send_time, NULL);
            fprintf(out_fp, "%ld %ld\n", send_time.tv_sec, send_time.tv_usec);
            msgsnd(msgid, &message, PAYLOAD_SIZE, 0);
        }
    } else {
        printf("[ERROR] client incompatible with type recv, use send/avg instead\n");
        return 1;
    }
    return 0;
}

int mq_run_server(int exe_cnt, enum time_type type, const char *out_file, int blocking, int debug) {
    struct timeval recv_time;
    FILE *out_fp;
    key_t key;
    int msgid;
    int iter = 0;
    int mq_flag = blocking == 1 ? 0 : IPC_NOWAIT;

    out_fp = fopen(out_file, "w");
    if (out_fp == NULL) {
        printf("[ERROR] failed opening file %s\n", out_file);
        return 1;
    }
    /* ftok to generate unique key */
    key = ftok(PATH, PROJECT_ID);

    /* msgget creates a message queue and returns identifier */
    msgid = msgget(key, QUEUE_PERM | IPC_CREAT);
    if (blocking) {
        for (; iter < exe_cnt; ++iter) {
            /* msgrcv to receive message */
            msgrcv(msgid, &message, PAYLOAD_SIZE, MSG_TYPE, mq_flag);
            gettimeofday(&recv_time, NULL);
            fprintf(out_fp, "%ld %ld\n", recv_time.tv_sec, recv_time.tv_usec);
            if (debug) {
                printf("Received: %s \n", message.payload);
            }
        }
    } else {
        while (iter != exe_cnt) {
            msgrcv(msgid, &message, PAYLOAD_SIZE, MSG_TYPE, mq_flag);
            if (strcmp(message.payload, MESSAGE) == 0) {
                gettimeofday(&recv_time, NULL);
                fprintf(out_fp, "%ld %ld\n", recv_time.tv_sec, recv_time.tv_usec);
                if (debug) {
                    printf("Received: %s \n", message.payload);
                }
                ++iter;
            }
        }
    }
    /* cleanup */
    fclose(out_fp);
    msgctl(msgid, IPC_RMID, NULL);
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
    printf("Example usage for server: %s -e 100 -m serv -t recv -o posixRecv.out\n", prog);
    printf("Example usage for client: %s -e 100 -m cli -t send -o posixSend.out\n", prog);
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
    while ((option = getopt_long(argc, argv, "e:m:t:o:bgh", longOpts,
                &opt_idx)) != -1) {
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
                printf("[%s] Unrecognized option `\\x%x'\n", __FILE__, option);
                usage(argv[0]);
                return 1;
        } // switch
    } // while
    if (mode == SERV) {
        ret = mq_run_server(exe_cnt, type, out_file, blocking, debug_flag);
    } else if (mode == CLI) {
        ret = mq_run_client(type, out_file, exe_cnt, debug_flag);
    } else if (mode == ATTR) {

    } else {
        printf("[ERROR] You should never see this, something is terribly wrong\n");
        return 1;
    }
    return 0;
}
