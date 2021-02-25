#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <errno.h>
#include <getopt.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define ONE_MILLION        1000000
#define MAX_FILE_NAME_LEN  32
#define MESSAGE_LEN        512
#define MAX_ROUND          9999
#define MAX_NUM_INDEX      4
#define PERMS S_IRUSR | S_IWUSR

enum run_mode {SERV = 1, CLI = 2, ATTR = 3};
enum compute_mode {ALLINONE = 0, ONE2ONE = 1 };
enum time_type {AVG = 0, SEND = 1, RECV = 2};

char *get_indexed_filename(const char *out_file, int index) {
    char *ret = malloc(sizeof(char) * MAX_FILE_NAME_LEN);
    char idx_str[MAX_NUM_INDEX];
    strcpy(ret, out_file);
    strcat(ret, "_");
    sprintf(idx_str, "%d", index);
    strcat(ret, idx_str);
    return ret;
}

void write_timestamp_to_file(enum time_type type, const char *out_file, const struct timeval *timestamp, int exe_cnt) {
    int iter = 0;
    FILE *out_fp = fopen(out_file, "w");
    if (out_fp == NULL) {
        printf("[ERROR] failed opening file %s\n", out_file);
    }
    if (type == AVG) {
        //printf("writing time %ld %ld to file %s\n", timestamp->tv_sec, timestamp->tv_usec, out_file);
        fprintf(out_fp, "%ld %ld\n", timestamp->tv_sec, timestamp->tv_usec);
    } else if (type == SEND || type == RECV) {
        for (; iter != exe_cnt; ++iter) {
            fprintf(out_fp, "%ld %ld\n", timestamp[iter].tv_sec, timestamp[iter].tv_usec);
        }
    }
    fclose(out_fp);
    return;
}

const char *get_error_str(int error_no) {
    switch (error_no) {
        case EACCES:
            return "EACCES";
        case EAGAIN:
            return "EAGAIN";
        case EBADF:
           return "EBADF";
        case EEXIST:
            return "EEXIST";
        case EFAULT:
            return "EFAULT";
        case EIDRM:
            return "EIDRM";
        case EINTR:
            return "EINTR";
        case EINVAL:
            return "EINVAL";
        case ENOENT:
           return "ENOENT";
        case ENOMEM:
            return "ENOMEM";
        case EMFILE:
           return "EMFILE";
        case EMSGSIZE:
           return "EMSGSIZE";
        case ENOSYS: // msgrcv only
            return "ENOSYS";
        case ENOSPC:
           return "ENOSPC";
        case ETIMEDOUT:
           return "ETIMEDOUT";
        case E2BIG:  // msgrcv only
            return "E2BIG";
        default:
            return "ERROR TYPE NOT FOUND";
    } // switch
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

#endif
