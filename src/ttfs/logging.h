#ifndef HEADER_LOGGING
#define HEADER_LOGGING

#include <stdlib.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include "utils.h"
#include "timemeter.h"

typedef enum {
    TTFS_FILE = 1,
    TTFS_PROCESS = 2,
    TTFS_EPOCH = 3,
} ttfs_log_mode_t;

typedef struct {
    unsigned int epoch;
    unsigned int workers;
    unsigned int ttfs_reclen;
    unsigned int ttfs_recnum;
    unsigned int files;
    double start_timestamp;
    double end_timestamp;
} ttfs_logrec_epoch_t;


typedef struct {
    char ttfs_fn[NAME_MAX];
    unsigned int ttfs_reclen;
    unsigned int ttfs_recnum;
    unsigned char md5str_eachwrite[MD5_DIGEST_LENGTH*2 + 1];
    unsigned char md5str_wholefile[MD5_DIGEST_LENGTH*2 + 1];
    timemeter_t tm;
} ttfs_logrec_file_t;


typedef struct ttfs_logfile_t ttfs_logfile_t;

typedef struct ttfs_logfile_t {
    char *logfilename;
    int fd;
    void (*init) (ttfs_logfile_t*);
    void (*write) (const ttfs_logfile_t*, const char*);
    void (*close) (const ttfs_logfile_t*);
} ttfs_logfile_t;

void lfinit(ttfs_logfile_t *l)
{
    char bakupname[NAME_MAX], rstr[5];
    struct stat st = {0};
    int r = stat( l->logfilename, &st );

    if ( r == 0 ) {
        rand_str( rstr, 4 );
        sprintf( bakupname, "%s.%s.bak", l->logfilename, rstr );
        rename( l->logfilename, bakupname );
    }

    l->fd = open( l->logfilename, O_WRONLY | O_CREAT, 0644 );
}

void lfwrite(const ttfs_logfile_t *l, const char* logrec)
{
    write( l->fd, logrec, strlen(logrec) );
}

void lfclose(const ttfs_logfile_t *l)
{
    close(l->fd);
}


ttfs_logfile_t logger = {0};
ttfs_logrec_file_t log_f_rec = {0} ;
ttfs_logrec_epoch_t log_e_rec = {0};
#endif
