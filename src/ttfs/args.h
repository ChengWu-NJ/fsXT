#ifndef HEADER_ARGS
#define HEADER_ARGS

#include <stdio.h>
#include <argp.h>
#include <errno.h>

#define RECORDMAXLEN 1048576
#define SUBDIR_UNDERWAY ".writing"

const char *argp_program_version = "torturefs v0.1";

static char doc[] =
  "Torturefs is for evaluting performance and reliability of a filesystem.";

static char args_doc[] = "";

static struct argp_option options[] = {
    {"writepath", 'w', "WRITEPATH", 0,  "The path is to write on test filesystem" },
    {"readpath", 'r', "READPATH", 0,  "The path is alternative one in a distributed filesystem or same as writhpath by default to md5sum write-finished files in writepath" },
    {"workers", 'p', "1", 0, "The number of processes to generate original files" },
    {"generatedfiles", 'f', "10", 0, "The total number of files generate"},
    {"recordnum", 'n', "10000", 0, "The number of records in per generated file" },
    {"recordlength", 'l', "256", 0, "The length record including index and timestamp"},
    {"epoch", 'e', "0", 0, "For logging to identify different combination of workers, recordlength, recordnum, generatedfiles"},
    { 0 }
};

typedef struct {
    
    char writepath[PATH_MAX];
    char readpath[PATH_MAX];
    int workers;
    int generatedfiles;
    int recordnum;
    int recordlength;
    int epoch;
} arguments;

arguments args;

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
    arguments *arguments = state->input;

    switch(key) {
        case 'w':
            strcpy(arguments->writepath, arg);
            break;
        case 'r':
            strcpy(arguments->readpath, arg);
            break;
        case 'p':
            arguments->workers = atoi(arg);
            break;
        case 'f':
            arguments->generatedfiles = atoi(arg);
            break;
        case 'n':
            arguments->recordnum = atoi(arg);
            break;
        case 'l':
            arguments->recordlength = atoi(arg);
            break;
        case 'e':
            arguments->epoch = atoi(arg);
            break;

        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}


static struct argp argp = { options, parse_opt, args_doc, doc };

void init_arguments(arguments *args)
{
    args->workers = 1;
    args->generatedfiles = 10;
    args->writepath[0] = '\0';
    args->readpath[0] = '\0';
    args->recordnum = 10000;
    args->recordlength = 256;
    args->epoch = 0;
}

void valid_arguments(arguments *args)
{
    if ( strlen(args->writepath) == 0 ) {
        printf("The writepath cannot be empty.\n");
        exit(EXIT_FAILURE);
    } else {
        if ( strlen(args->readpath) == 0 )
            strcpy( args->readpath, args->writepath );
    }


    if ( args->recordlength > RECORDMAXLEN ) {
        printf("The record length cannot be greater than %d.\n", RECORDMAXLEN);
        exit(EXIT_FAILURE);
    }

    if ( args->recordlength < 40 ) {
        printf("The record length cannot be less than 40.\n");
        exit(EXIT_FAILURE);
    }

}


//ensure the directory and subdir .orig writable and empty them.
void ensure_path(char *path, char *label, int forreadpath)
{
    struct stat st = {0};
    int r = stat(path, &st);

    if (forreadpath) {
        if (r == -1) {
            printf( "The READPATH %s didn't exist. Please check mount"
                    " path if in a distributed filesystem.\n", path );
            exit(EXIT_FAILURE);
        }
    } else {
        if (r == -1) {
            int r = mkdir(path, 0644);
            if ( r == -1 ) {
                printf( "The %s didn't exist, and cannot create it.\n", label );
                exit(EXIT_FAILURE);
            };

        } else {
            if ( (st.st_mode & S_IFMT) != S_IFDIR ){
                printf( "The %s is not a directory.\n", label );
                exit(EXIT_FAILURE);
            }

            if ( !( ( (st.st_mode&S_IRUSR) > 0 ) & ( (st.st_mode&S_IWUSR) > 0 ) ) ) {
                printf( "Current user has not enogh rights to access the %s.\n", label );
                exit(EXIT_FAILURE);
            }
        }

        char cmdstr[1024];
        sprintf( cmdstr, "rm %s/* %s/%s/* -rf;mkdir -p %s/%s", path, path, SUBDIR_UNDERWAY, path, SUBDIR_UNDERWAY  );
        system( cmdstr );
    }
}

void prepare_env(arguments *args)
{
     ensure_path(args->writepath, "WRITEPATH", 0);
     ensure_path(args->readpath, "READPATH", 1);
     
}



#endif
