#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/time.h>

#define MAX_FILE_NAME_LEN  32

enum queue_type{systemV = 1, POSIX = 2};

static void usage(const char *prog) {
    printf("%s --execution-count <numOfExecution> --send-time-input <sendFileName> --receive-time-input <receiveFileName> --elapsed-time-output <outFileName> --system-V|--posix \n", prog);
    printf("Example usage on system V: %s -e 100 -s sysVSend.out -r sysVRecv.out -o result.out -v\n", prog);
    printf("Example usage on POSIX: %s -e 100 -s posixSend.out -r posixRecv.out -o result.out -p\n", prog);
}

void processFiles(char old_file[], char new_file[], int count, enum queue_type mode, char out_file[]) {
    FILE *old_fp, *new_fp, *out_fp;
    struct timeval old_time, new_time;
    long int sec = 0, usec = 0;
    long int elapsed_usec = 0;
    int i = 0;
    old_fp = fopen(old_file, "r");
    new_fp = fopen(new_file, "r");
    out_fp = fopen(out_file, "w");
    for (; i < count; ++i) {
        fscanf(old_fp, "%ld", &old_time.tv_sec);
        fscanf(old_fp, "%ld", &old_time.tv_usec);
        fscanf(new_fp, "%ld", &new_time.tv_sec);
        fscanf(new_fp, "%ld", &new_time.tv_usec);
        elapsed_usec = (new_time.tv_sec - old_time.tv_sec) * 1000000 +
            (new_time.tv_usec - old_time.tv_usec);
        fprintf(out_fp, "%ld\n", elapsed_usec);
    } // for
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
        { "system-V", no_argument, NULL, 'v'                  },
        { "posix", no_argument, NULL, 'p'                     },
        { "help", no_argument, NULL, 'h'                      },
        { NULL, 0, NULL, '\0'} };

    int option_index = 0, option = 0;
    int exe_cnt = 0;
    char send_file_name[MAX_FILE_NAME_LEN];
    char recv_file_name[MAX_FILE_NAME_LEN];
    char output_file_name[MAX_FILE_NAME_LEN];
    enum queue_type mode;
    while ((option = getopt_long(argc, argv, "e:s:r:o:vph", longOpts,
                &option_index)) != -1) {
        switch (option) {
            case 'e':
                exe_cnt = atoi(optarg);
                //printf("e: %d\n", exe_cnt);
                break;
            case 's':
                strcpy(send_file_name, optarg);
                //printf("s: %s\n", send_file_name);
                break;
            case 'r':
                strcpy(recv_file_name, optarg);
                //printf("r: %s\n", recv_file_name);
                break;
            case 'o':
                strcpy(output_file_name, optarg);
            case 'v':
                mode = systemV;
                break;
            case 'p':
                mode = POSIX;
                break;
            case 'h':
                usage(argv[0]);
                exit(0);
            default:
                printf("Unrecognized option %c\n", option);
                usage(argv[0]);
                exit(0);
        } // switch
    } // while
    //printf("mode = %d (systemV=1, POSIX=2)\n", mode);
    processFiles(send_file_name, recv_file_name, exe_cnt, mode, output_file_name);
    return 0;
}
