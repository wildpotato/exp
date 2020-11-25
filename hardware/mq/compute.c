#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#define MAX_FILE_PATH_LEN  32

enum queue_type{systemV = 1, POSIX = 2};

static void usage(const char *prog) {
    printf("%s --execution-count <numOfExecution> --send-time-input <sendFileName> --receive-time-input <receiveFileName> --system-V|--posix \n", prog);
    printf("Example usage on system V: %s -e 100 -s sysVSend.out -r sysVRecv.out -v\n", prog);
    printf("Example usage on POSIX: %s -e 100 -s posixSend.out -r posixRecv.out -p\n", prog);
}

int main(int argc, char **argv) {
    struct option longOpts[] = {
        { "execution-count", required_argument, NULL, 'e'    },
        { "send-time-input", required_argument, NULL, 's'    },
        { "receive-time-input", required_argument, NULL, 'r' },
        { "system-V", no_argument, NULL, 'v'                 },
        { "posix", no_argument, NULL, 'p'                    },
        { "help", no_argument, NULL, 'h'                     },
        { NULL, 0, NULL, '\0'} };

    int option_index = 0, option = 0;
    int exe_cnt = 0;
    char send_file_name[MAX_FILE_PATH_LEN];
    char recv_file_name[MAX_FILE_PATH_LEN];
    enum queue_type mode;
    while ((option = getopt_long(argc, argv, "e:s:r:vph", longOpts,
                &option_index)) != -1) {
        switch (option) {
            case 'e':
                exe_cnt = atoi(optarg);
                printf("e: %d\n", exe_cnt);
                break;
            case 's':
                strcpy(send_file_name, optarg);
                printf("s: %s\n", send_file_name);
                break;
            case 'r':
                strcpy(recv_file_name, optarg);
                printf("r: %s\n", recv_file_name);
                break;
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
    printf("mode = %d (systemV=1, POSIX=2)\n", mode);
    return 0;
}
