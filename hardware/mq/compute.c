#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/time.h>

#include "utils.h"

static void usage(const char *prog) {
    printf("%s --execution-count <numOfExecution> --send-time-input <sendFileName> --receive-time-input <receiveFileName> --elapsed-time-output <outFileName> --mode <computeMode> \n", prog);
    printf("Example usage on one-to-one: %s -e 100 -s sysVSend.out -r sysVRecv.out -o result.out -m one2one\n", prog);
    printf("Example usage on average: %s -e 100 -s posixSend.out -r posixRecv.out -o result.out -m avg\n", prog);
}

static float getAvgUsecElapsed(long long int totalUsecElapsed, int count) {
    float avg_usec_elapsed = (float)totalUsecElapsed / count;
    return avg_usec_elapsed;
}

float processFiles(char old_file[], char new_file[], int count, int curr_round, enum compute_mode mode, char out_file[]) {
    FILE *old_fp, *new_fp, *out_fp;
    struct timeval old_time, new_time;
    long long int total_elapsed_usec = 0;
    float avg_elapsed_usec = 0.0; // average per operation (vary by rounds)
    long int sec = 0, usec = 0;
    long int elapsed_usec = 0;
    int i = mode == ALLINONE ? count - 1 : 0;

    old_fp = fopen(old_file, "r");
    new_fp = fopen(new_file, "r");
    out_fp = fopen(out_file, "w");
    if (old_fp == NULL || new_fp == NULL || out_fp == NULL) {
        printf("[ERR] Open file failed\n");
        return -1;
    }
    for (; i < count; ++i) {
        fscanf(old_fp, "%ld", &old_time.tv_sec);
        fscanf(old_fp, "%ld", &old_time.tv_usec);
        fscanf(new_fp, "%ld", &new_time.tv_sec);
        fscanf(new_fp, "%ld", &new_time.tv_usec);
        elapsed_usec = (new_time.tv_sec - old_time.tv_sec) * ONE_MILLION +
            (new_time.tv_usec - old_time.tv_usec);
        fprintf(out_fp, "%ld\n", elapsed_usec);
        total_elapsed_usec += elapsed_usec;
    } // for
    printf("round total elapsed usec: %lld  ", total_elapsed_usec);
    avg_elapsed_usec = getAvgUsecElapsed(total_elapsed_usec, count);
    fprintf(out_fp, "Total usecs elapsed: %lld (exe_cnt=%d)\n", total_elapsed_usec, count);
    fprintf(out_fp, "Average elapsed usecs per operation: %f (round = %d)\n", avg_elapsed_usec, curr_round);
    fclose(out_fp);
    fclose(old_fp);
    fclose(new_fp);
    printf("round average elapsed usec: %f\n", avg_elapsed_usec);
    return avg_elapsed_usec;
}


int main(int argc, char **argv) {
    struct option longOpts[] = {
        { "execution-count", required_argument, NULL, 'e'     },
        { "send-time-input", required_argument, NULL, 's'     },
        { "receive-time-input", required_argument, NULL, 'r'  },
        { "elapsed-time-output", required_argument, NULL, 'o' },
        { "round", required_argument, NULL, 'd'               },
        { "mode", no_argument, NULL, 'm'                      },
        { "help", no_argument, NULL, 'h'                      },
        { NULL, 0, NULL, '\0'} };
    float avg_elapsed_usec = 0.0; // This is the final result we want
    int option_index = 0, option = 0;
    int exe_cnt = 0;
    char send_file_name[MAX_FILE_NAME_LEN];
    char recv_file_name[MAX_FILE_NAME_LEN];
    char output_file_name[MAX_FILE_NAME_LEN];
    enum compute_mode mode;
    int opt_flag = 0;
    int round = 0;

    while ((option = getopt_long(argc, argv, "e:s:r:o:d:m:h", longOpts,
                &option_index)) != -1) {
        opt_flag = 1;
        switch (option) {
            case 'e':
                exe_cnt = atoi(optarg);
                break;
            case 's':
                strcpy(send_file_name, optarg);
                break;
            case 'r':
                strcpy(recv_file_name, optarg);
                break;
            case 'o':
                strcpy(output_file_name, optarg);
                break;
            case 'd':
                round = atoi(optarg);
                break;
            case 'm':
                if (strcmp("one2one", optarg) == 0) {
                    mode = ONE2ONE;
                } else if (strcmp("allinone", optarg) == 0) {
                    mode = ALLINONE;
                } else {
                    printf("[ERROR] Unsupported mode, please use one2one/allinone\n");
                    return 1;
                }
                break;
            case 'h':
                usage(argv[0]);
                return 0;
            default:
                printf("[ERROR] Unrecognized option `\\x%x'\n", option);
                usage(argv[0]);
                return 1;
        } // switch
    } // while
    if (!opt_flag) {
        usage(argv[0]);
        return 1;
    }
    for (int i = 0; i < round; ++i) {
        char *indexed_send_file = get_indexed_filename(send_file_name, i);
        char *indexed_recv_file = get_indexed_filename(recv_file_name, i);
        char *indexed_out_file = get_indexed_filename(output_file_name, i);
        float round_avg = processFiles(indexed_send_file, indexed_recv_file, exe_cnt, i, mode, indexed_out_file);
        if (i > 0)
            avg_elapsed_usec += round_avg;
    }
    avg_elapsed_usec /= (round - 1);
    printf("Average elapsed time over m-1 round over n execution counts = %f\n", avg_elapsed_usec);
    return 0;
}
