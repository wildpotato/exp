#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include <sys/time.h>

/* when running this program, use one and only one of the options */
//#define BENCHMARK_SEND
#define BENCHMARK_SEND_AND_RECEIVE

/* use 0 | IPC_NOWAIT | MSG_EXCEPT | MSG_NOERROR
 * IPC_NOWAIT:  performs non-blocking receive
 * MSG_EXCEPT:  effective if msg_type > 0
 * MSG_NOERROR: truncates message if it exceeds size limit
 */
#define MQ_FLAG          IPC_NOWAIT

#define MSG_TYPE         0
#define EXPERIMENT_COUNT 10
#define ITER_COUNT       1000000
#define MAX_SIZE         1024
#define QUEUE_PERM       0666
#define PAYLOAD_SIZE     MAX_SIZE - sizeof(long)
#define PATH             "/var/tmp/progfile"
#define PROJECT_ID       65

/* this time struct is used to stamp the exact time after epoch
 * right before the send operation occurs
 */
struct timeval send_time;

/* these vars are used to calculate batch send time only */
clock_t start, end;
double cpu_time_used_avg = 0;
double cpu_time_used;

struct mesg_buffer {
    long msg_type;
    char payload[PAYLOAD_SIZE];
} message;

int run_sending_app()
{
    key_t key;
    int msgid;

    // ftok to generate unique key
    key = ftok(PATH, PROJECT_ID);

    // msgget creates a message queue
    // and returns identifier
    msgid = msgget(key, QUEUE_PERM | IPC_CREAT);
    message.msg_type = 1; // must be greater than 0

    // printf("Write Data : ");
    // fgets(message.payload, sizeof(message.payload), stdin);

    strcpy(message.payload, "hello");
    int e,i;

    for (e = 0; e < EXPERIMENT_COUNT; e++)
    {
#ifdef BENCHMARK_SEND
        start = clock();
#endif
#ifdef BENCHMARK_SEND_AND_RECEIVE
        gettimeofday(&send_time, NULL);
        printf("%ld %ld\n", send_time.tv_sec, send_time.tv_usec);
#endif
        for (i = 0; i != ITER_COUNT; i++)
            msgsnd(msgid, &message, sizeof(message), 0);
#ifdef BENCHMARK_SEND
        end = clock();
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
        printf("cpu_time_used = %f\n", cpu_time_used);
        cpu_time_used_avg += cpu_time_used;
#endif
    }
#ifdef BENCHMARK_SEND
    printf("cpu_time_used_avg = %f\n", cpu_time_used_avg/EXPERIMENT_COUNT);
#endif
    // // display the message
    // printf("Data send is : %s \n", message.payload);

    return 0;
}

int run_receiving_app()
{
    key_t key;
    int msgid;

    // ftok to generate unique key
    key = ftok(PATH, PROJECT_ID);

    // msgget creates a message queue
    // and returns identifier
    msgid = msgget(key, QUEUE_PERM | IPC_CREAT);

    while(1)
    {
        // msgrcv to receive message
        msgrcv(msgid, &message, sizeof(message), MSG_TYPE, MQ_FLAG);
        // // display the message
        // printf("Data Received is : %s \n",
        //                 message.payload);
    }


    // to destroy the message queue
    msgctl(msgid, IPC_RMID, NULL);

    return 0;
}

void mq_print_attr()
{

}

int main(int argc, char const *argv[])
{
    if (argc > 1) {
        if (!strcasecmp(argv[1], "cli")) {
            run_sending_app();
		}
        else if (!strcasecmp(argv[1], "serv")) {
            run_receiving_app();
		}
        else if (!strcasecmp(argv[1], "attr")) {
            mq_print_attr();
		}
		else {
			printf("arg: cli/serv/attr\n");
		}
	} else {
		printf("Use 1 required argument\n");
	}
    return 0;
}
