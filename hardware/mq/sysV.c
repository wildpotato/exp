#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>

#define EXPERIMENT_COUNT 100
#define PAYLOAD_SIZE     1020 // msg = payload + msg_type
#define PATH             "/var/tmp/progfile"
#define PROJECT_ID       65

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
    msgid = msgget(key, 0666 | IPC_CREAT);
    message.msg_type = 1; // must be greater than 0

    // printf("Write Data : ");
    // fgets(message.payload, sizeof(message.payload), stdin);

    strcpy(message.payload, "hello");
    int e,i;

    for (e = 0; e < EXPERIMENT_COUNT; e++)
    {
        start = clock();

        for (i = 0; i != 1000000; i++)
            msgsnd(msgid, &message, sizeof(message), 0);

        end = clock();
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

        printf("cpu_time_used = %f\n", cpu_time_used);

        cpu_time_used_avg += cpu_time_used;
    }

    printf("cpu_time_used_avg = %f\n", cpu_time_used_avg/EXPERIMENT_COUNT);

    // // display the message
    // printf("Data send is : %s \n", message.payload);

    return 0;
}

int run_receiving_app()
{
    key_t key;
    int msgid;

    // ftok to generate unique key
    key = ftok("/var/tmp/progfile", PROJECT_ID);

    // msgget creates a message queue
    // and returns identifier
    msgid = msgget(key, 0666 | IPC_CREAT);

    while(1)
    {
        // msgrcv to receive message
        msgrcv(msgid, &message, sizeof(message), 0, 0);
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
    if (argc > 1)
        if (!strcasecmp(argv[1], "cli"))
            run_sending_app();
        else if (!strcasecmp(argv[1], "serv"))
            run_receiving_app();
        else if (!strcasecmp(argv[1], "attr"))
            mq_print_attr();

    return 0;
}
