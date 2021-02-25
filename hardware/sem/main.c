#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#include "binary_sem.h"

#define PERMS S_IRUSR | S_IWUSR
#define SEM_SET_ID "/tmp/sem_id.tmp"

void usage(const char *);
bool is_file_exists(const char *);
int store_key(const char *, key_t);
key_t retrieve_key(const char *);

enum run_mode { SENDER = 1, RECEIVER = 2 };

void sender(int round)
{
    int semid = retrieve_key(SEM_SET_ID);
    int ret = -1;
    for (int i = 0; i < round; ++i) {
        while (ret != 0) {
            ret = reserve_sem(semid, 0);
        }
        printf("[MSG] Allowed to send %d\n", i);
        ret = -1;
    }
    return;
}

void receiver(int round)
{
    int semid = semget(IPC_PRIVATE, 1, IPC_CREAT | IPC_EXCL | PERMS);
    int ret = 0;

    ret = store_key(SEM_SET_ID, semid);
    sleep(2);
    if (semid != -1) {
        ret = init_sem_in_use(semid, 0);
        if (ret != 0) {
            printf("[ERR] init_sem_available\n");
        }
    } else {
        printf("[ERR] semget\n");
        return;
    }

    for (int i = 0; i < round; ++i) {
        printf("[MSG] Receive %d", i);
        sleep(2);
        ret = release_sem(semid, 0);
        if (ret == -1) {
            printf(" --> sem up failed\n");
        } else {
            printf(" -->  sem up one\n");
        }
    }
}

int main(int argc, char **argv)
{
    struct option longOpts[] = {
        { "mode", required_argument, NULL, 'm'  },
        { "help", no_argument, NULL, 'h'        },
        { "round", required_argument, NULL, 'r' },
        {  NULL, 0, NULL, '\0'}};
    int opt_idx = 0, option = 0, round = 0;
    enum run_mode mode;

    while ((option = getopt_long(argc, argv, "m:r:h", longOpts,
                &opt_idx)) != -1) {
        switch(option) {
            case 'm':
                if (strcmp("sender", optarg) == 0) {
                    mode = SENDER;
                } else if (strcmp("receiver", optarg) == 0) {
                    mode = RECEIVER;
                } else {
                    printf("Unsupported mode\n");
                    return 1;
                }
                break;
            case 'r':
                round = atoi(optarg);
                break;
            case 'h':
                usage(argv[0]);
                return 0;
            default:
                usage(argv[0]);
                return 1;
        }
    }
    if (mode == SENDER) {
        sender(round);
    } else if (mode == RECEIVER) {
        receiver(round);
    } else {
    }
    printf("Hello, World!\n");
    return 0;
}

void usage(const char *prog)
{
    printf("==============================================\n");
    printf("Usage: %s -m <mode> -r <round>\n\n", prog);
    printf("mode  :  sender / receiver\n");
    printf("round :  number of rounds\n");
    printf("==============================================\n");
}

bool is_file_exists(const char *path) {
    struct stat st;
    if (!stat(path, &st)) {
        if (S_ISREG(st.st_mode)) {
            return true;
        }
    }
    return false;
}

int store_key(const char *key_file_path, key_t key) {
    FILE *fp;
    fp = fopen(key_file_path, "w");
    if (fp == NULL) {
        printf("[ERROR] failed writing key to file %s\n", key_file_path);
        return -1;
    }
    fprintf(fp, "%d", key);
    fclose(fp);
    return 0;
}

key_t retrieve_key(const char *key_file_path) {
    FILE *fp;
    key_t key;
    int file_exists = -1;
    do {
        file_exists = access(key_file_path, F_OK);
    } while (file_exists);
    fp = fopen(key_file_path, "r");
    if (fp == NULL) {
        printf("[ERROR] failed retrieving key from file %s\n", key_file_path);
        return -1;
    }
    fscanf(fp, "%d", &key);
    fclose(fp);
    return key;
}

