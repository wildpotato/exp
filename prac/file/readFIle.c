#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <unistd.h>
#include <stdint.h>
#include <libgen.h>
#include <sys/inotify.h>

#define     OPTSTR                  "i:o:h"
#define     USAGE_FMT               "Usage: %s [-v] [-i InputFile] [-h]\n"
#define     ERR_FOPEN_INPUT         "fopen(input, r)"
#define     ERR_FOPEN_OUTPUT        "fopen(output, w)"
#define     DEFAULT_PROGNAME        "a.out"

#define     READ_LEN                10

extern int errno;
extern char *optarg;
extern int opterr, optind;

typedef struct {
    int verbose;
    uint32_t flags;
    FILE *input;
    FILE *output;
} options_t;

void usage(char *progname, int opt)
{
    fprintf(stderr, USAGE_FMT, progname ? progname : DEFAULT_PROGNAME);
    exit(EXIT_FAILURE);
}

int transferFileData(optoins_t *options)
{
    int numsRead = 0;
    char buf[READ_LEN];

    if (!options)
    {
        errno = EINVAL;
        goto failure;
    }

    if (!options->input || !options->output)
    {
        errno = ENOENT;
        goto failure;
    }

    while ((numsRead = read(options->input, buf, READ_LEN)) > 0)


    fclose(options->output);
    fclose(options->input);
    return EXIT_SUCCESS;

failure:
    return EXIT_FAILURE;
}

int main(int argc, char *argv[])
{
    int opt;
    options_t options = { 0, 0x0, stdin, stdout };

    opterr = 0;

    while ((opt = getopt(argc, argv, OPTSTR)) != EOF)
    {
        switch (opt) {
            case 'i':
                if (!(options.input = fopen(optarg, "r"))) {
                    perror(ERR_FOPEN_INPUT);
                    exit(EXIT_FAILURE);
                }
                break;
            case 'o':
                if (!(options.output = fopen(optarg, "w"))) {
                    perror(ERR_FOPEN_OUTPUT);
                    exit(EXIT_FAILURE);
                }
                break;
            case 'h':
            default:
                usage(basename(argv[0]), opt);
                break;
        }
    }
    return EXIT_SUCCESS;
}
