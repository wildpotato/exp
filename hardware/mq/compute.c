#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/time.h>

#define MAX_FILE_NAME_LEN  32
#define ONE_MILLION   1000000

enum time_type{ONE2ONE = 1, AVG = 2};

static void usage(const char *prog) {
    printf("%s --execution-count <numOfExecution> --send-time-input <sendFileName> --receive-time-input <receiveFileName> --elapsed-time-output <outFileName> --mode <timeMode> \n", prog);
    printf("Example usage on one-to-one: %s -e 100 -s sysVSend.out -r sysVRecv.out -o result.out -m one2one\n", prog);
    printf("Example usage on average: %s -e 100 -s posixSend.out -r posixRecv.out -o result.out -m avg\n", prog);
}

static float getAvgUsecElapsed(long long int totalUsecElapsed, int count) {
    float avg_usec_elapsed = (float)totalUsecElapsed / count;
    return avg_usec_elapsed;
}

void processFiles(char old_file[], char new_file[], int count, enum time_type mode, char out_file[]) {
    FILE *old_fp, *new_fp, *out_fp;
    struct timeval old_time, new_time;
    long long int total_elapsed_usec = 0;
    float avg_elapsed_usec = 0.0;
    long int sec = 0, usec = 0;
    long int elapsed_usec = 0;
    int i = mode == AVG ? count - 1 : 0;
    old_fp = fopen(old_file, "r");
    new_fp = fopen(new_file, "r");
    out_fp = fopen(out_file, "w");
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
    avg_elapsed_usec = getAvgUsecElapsed(total_elapsed_usec, count);
    fprintf(out_fp, "Total usecs elapsed: %lld (exe_cnt=%d)\n", total_elapsed_usec, count);
    fprintf(out_fp, "Average elapsed usecs per operation: %f\n", avg_elapsed_usec);
    fclose(out_fp);
    fclose(old_fp);
    fclose(new_fp);
}


int main(int argc, char **argv) {
    struct option longOpts[] = {
        { "execution-count", required_argument, NULL, 'e'     },
        { "send-time-input", required_argument, NULL, 's'     },
        { "receive-time-input", required_argument, NULL, 'r'  },
        { "elapsed-time-output", required_argument, NULL, 'o' },
        { "mode", no_argument, NULL, 'm'                      },
        { "help", no_argument, NULL, 'h'                      },
        { NULL, 0, NULL, '\0'} };

    int option_index = 0, option = 0;
    int exe_cnt = 0;
    char send_file_name[MAX_FILE_NAME_LEN];
    char recv_file_name[MAX_FILE_NAME_LEN];
    char output_file_name[MAX_FILE_NAME_LEN];
    enum time_type mode;
    int opt_flag = 0;
    while ((option = getopt_long(argc, argv, "e:s:r:o:m:h", longOpts,
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
            case 'm':
                if (strcmp("one2one", optarg) == 0) {
                    mode = ONE2ONE;
                } else if (strcmp("avg", optarg) == 0) {
                    mode = AVG;
                } else {
                    printf("[ERROR] Unsupported mode, please use serv/cli/attr\n");
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
    processFiles(send_file_name, recv_file_name, exe_cnt, mode, output_file_name);
    return 0;
}
