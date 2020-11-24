#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <mqueue.h>
#include <time.h>

#define EXPERIMENT_COUNT 100
#define ITER_COUNT       1000000
#define QUEUE_NAME       "/test_queue"
#define MAX_SIZE         1024
#define MAX_MSG          10
#define MSG_STOP         "exit"

#define CHECK(x) \
    do { \
        if (!(x)) { \
            fprintf(stderr, "%s:%d: ", __func__, __LINE__); \
            perror(#x); \
            exit(-1); \
        } \
    } while (0) \

clock_t start, end;
double cpu_time_used_avg = 0;
double cpu_time_used;

int mq_run_server()
{
    mqd_t mq;
    struct mq_attr attr;
    char buffer[MAX_SIZE + 1];
    int must_stop = 0;

    /* initialize the queue attributes */
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MSG;
    attr.mq_msgsize = MAX_SIZE;
    attr.mq_curmsgs = 0;

    /* create the message queue */
    // mq_unlink(QUEUE_NAME);
    // exit(0);

    mq = mq_open(QUEUE_NAME, O_CREAT | O_RDONLY, 0666, &attr);
    CHECK((mqd_t)-1 != mq);

    do {
        ssize_t bytes_read;

        /* receive the message */
        bytes_read = mq_receive(mq, buffer, MAX_SIZE, NULL);
        CHECK(bytes_read >= 0);

        buffer[bytes_read] = '\0';
        if (! strncmp(buffer, MSG_STOP, strlen(MSG_STOP)))
        {
            must_stop = 1;
        }
        else
        {
            // printf("Received: %s\n", buffer);
        }
    } while (!must_stop);

    /* cleanup */
    CHECK((mqd_t)-1 != mq_close(mq));
    CHECK((mqd_t)-1 != mq_unlink(QUEUE_NAME));

    return 0;
}

int mq_run_client()
{
    mqd_t mq;
    char buffer[MAX_SIZE];

    /* open the mail queue */
    mq = mq_open(QUEUE_NAME, O_WRONLY);
    CHECK((mqd_t)-1 != mq);


    strcpy(buffer, "hello");
    int e,i;

    for( e = 0; e < EXPERIMENT_COUNT; e++ )
    {
        start = clock();

        for(i = 0; i != ITER_COUNT; i++)
            mq_send(mq, buffer, MAX_SIZE, 0);

        end = clock();
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

        printf("cpu_time_used = %f\n", cpu_time_used);

        cpu_time_used_avg += cpu_time_used;
    }

    printf("cpu_time_used_avg = %f\n", cpu_time_used_avg/EXPERIMENT_COUNT);


    // printf("Send to server (enter \"exit\" to stop it):\n");
    //
    // do {
    //     printf("> ");
    //     fflush(stdout);

    //     memset(buffer, 0, MAX_SIZE);
    //     fgets(buffer, MAX_SIZE, stdin);

    //     /* send the message */
    //     CHECK(0 <= mq_send(mq, buffer, MAX_SIZE, 0));

    // } while (strncmp(buffer, MSG_STOP, strlen(MSG_STOP)));

    /* cleanup */
    CHECK((mqd_t)-1 != mq_close(mq));

    return 0;
}

void mq_print_attr()
{
    struct mq_attr attr;
    /*
    struct rlimit limit;
    if(getrlimit(RLIMIT_MSGQUEUE,&limit) == -1) {
        perror("getrlimit");
        return 1;
    }
    printf("RLIMIT_MSGQUEUE cur = %ld,max = %ld\n",(long)limit.rlim_cur,(long)limit.rlim_max);
    */

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

int main(int argc, char const *argv[])
{
    if (argc > 1) {
        if (!strcasecmp(argv[1], "serv")) {
            mq_run_server();
        } else if (!strcasecmp(argv[1], "cli")) {
            mq_run_client();
        } else if (!strcasecmp(argv[1], "attr")) {
            mq_print_attr();
        } else {
			printf("arg: cli/serv/attr\n");
        }
	} else {
		printf("Use 1 required argument\n");
	}
    return 0;
}
