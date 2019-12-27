#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <argp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <wait.h>
#include <signal.h>
#include <openssl/md5.h>
#include "utils.h"
#include "args.h"
#include "logging.h"


#define RECORDDELIMITER '\n'

unsigned int worker_id;
unsigned long accu_wbytes = 0, accu_wlast = 0;
unsigned long accu_rbytes = 0, accu_rlast = 0;

void gen_record(char *rec, const unsigned int no, const unsigned int reclen)
{
    unsigned int randlen = reclen - 33; //- index 10 - timestamp 20 - <>\n 3

    time_t ltime = time(NULL);
    struct tm *tm = localtime( &ltime );

    char st[20];
    strftime(st, 20, "%F %H:%M:%S", tm);


    char randbuf[randlen + 1];   // + end\0 1 
    rand_str( randbuf, randlen );

    sprintf( rec, "%010d.%s<%s>%c", no, st, randbuf, RECORDDELIMITER );
} 

int gen_recfile(const char *filename, const unsigned int recnum, 
		const unsigned int reclen )
{
    unsigned char rec[RECORDMAXLEN];
    unsigned char md5bin_eachwrite[MD5_DIGEST_LENGTH], md5bin_wholefile[MD5_DIGEST_LENGTH];
    char fn_write_final[NAME_MAX], fn_write_uw[NAME_MAX], fn_read[NAME_MAX];
    int fd;
    struct timespec tstart={0,0}, tend={0,0};

    sprintf( fn_write_final, "%s/%s", args.writepath, filename );
    sprintf( fn_write_uw, "%s/%s/%s", args.writepath, SUBDIR_UNDERWAY, filename );
    sprintf( fn_read, "%s/%s", args.readpath, filename );
    strcpy( log_f_rec.ttfs_fn, fn_write_final );

    TIMER_START(tstart);
    fd = open( fn_write_uw, O_WRONLY | O_CREAT, 0644);
    if(fd == -1){
        perror("open");
        return(EXIT_FAILURE);;
    }
    TIMER_END( log_f_rec.tm.open_tlen, tend, tstart );

    log_f_rec.ttfs_reclen = reclen;
    log_f_rec.ttfs_recnum = recnum;

    MD5_CTX cw,cr;
    MD5_Init(&cw);

    //begin writing
    for ( int i = 0; i < recnum; i++ ) {
        gen_record( rec, i, reclen );
	
	    TIMER_START(tstart);
	    write( fd, rec, reclen );
        accu_wbytes += reclen;
	    TIMER_END( log_f_rec.tm.write_tlen, tend, tstart );
        MD5_Update( &cw, rec, reclen );
    }
    MD5_Final( md5bin_eachwrite, &cw );
    bin2hex( log_f_rec.md5str_eachwrite, md5bin_eachwrite, MD5_DIGEST_LENGTH );

    TIMER_START(tstart);
    close(fd);
    TIMER_END( log_f_rec.tm.close_tlen, tend, tstart );

    //begin mving
    TIMER_START(tstart);
    rename( fn_write_uw, fn_write_final );
    TIMER_END( log_f_rec.tm.mv_tlen, tend, tstart );

    //begin reading
    fd = open(fn_read, O_RDONLY);
    if (fd == -1) {
        perror("readonly open");
        return(EXIT_FAILURE);
    }

    MD5_Init(&cr);
    int first = 1;
    ssize_t len_in = 0;
    while( first || len_in > 0 ){
        first = 0;
        TIMER_START(tstart);
        len_in = read( fd, rec, reclen );
        TIMER_END( log_f_rec.tm.read_tlen, tend, tstart );
        MD5_Update( &cr, rec, len_in );
        accu_rbytes += len_in;
    }
    MD5_Final(md5bin_wholefile, &cr);

    close(fd);

    bin2hex( log_f_rec.md5str_wholefile, md5bin_wholefile, MD5_DIGEST_LENGTH );
    return(EXIT_SUCCESS);
}

void logrec_file()
{
    char logrec[1024];
    //log_mode,worker_id,filename,reclen(Bytes),recnum,md5str_eachwrite,md5str_wholefile,
    //opentlen(Second),closetlen,mvtlen,writetlen,readtlen
    sprintf( logrec, "%d,%d,%s,%d,%d,%s,%s,%f,%f,%f,%f,%f\n", \
		    TTFS_FILE, worker_id, log_f_rec.ttfs_fn, log_f_rec.ttfs_reclen, log_f_rec.ttfs_recnum, \
            log_f_rec.md5str_eachwrite, log_f_rec.md5str_wholefile, \
            log_f_rec.tm.open_tlen, log_f_rec.tm.close_tlen, log_f_rec.tm.mv_tlen, \
            log_f_rec.tm.write_tlen, log_f_rec.tm.read_tlen );
    logger.write( &logger, logrec );
}

void gen_origfiles(const unsigned int wid)
{
    char orig_fn[NAME_MAX];
    for ( int i = 0; i < args.generatedfiles; i++ ) {
        memset( &log_f_rec, 0, sizeof(ttfs_logrec_file_t) );
        sprintf( orig_fn, "e%04d.w%03d.%07d.orig", args.epoch, worker_id, i  );
        gen_recfile( orig_fn, args.recordnum, args.recordlength );

	logrec_file();
    }
}

void logrec_epoch()
{
    char logrec[1024];
    //log_mode, epoch_id, workers, reclen, recnum, files, starttime, endtime
    sprintf( logrec, "%d,%d,%d,%d,%d,%d,%f,%f\n",
             TTFS_EPOCH, log_e_rec.epoch, 
             log_e_rec.workers, log_e_rec.ttfs_reclen,
             log_e_rec.ttfs_recnum, log_e_rec.files, 
             log_e_rec.start_timestamp, log_e_rec.end_timestamp );
    logger.write( &logger, logrec );
}

void logrec_process()
{
    char logrec[1024];
    struct timespec ts;
    double checkpoint, elapsedtime;
    unsigned long _wbs = accu_wbytes;
    unsigned long _winterval = _wbs - accu_wlast;
    unsigned long _rbs = accu_rbytes;
    unsigned long _rinterval = _rbs - accu_rlast;

    TIMER_NOW( checkpoint, ts );
    elapsedtime = checkpoint - log_e_rec.start_timestamp;
    //log_mode, worker_id, ckeckpoint, wbytes_interval, rbytes_internal, elapsedtime
    if ( _winterval > 0 || _rinterval > 0 ) {
        sprintf( logrec, "%d,%d,%f,%ld,%ld,%f\n", TTFS_PROCESS, 
                worker_id, checkpoint, _winterval, _rinterval, elapsedtime);
        logger.write( &logger, logrec );
    }
    accu_wlast = _wbs;
    accu_rlast = _rbs;
}

void timer_thread(union sigval v) 

{
    logrec_process();
}


#define TIMER_THREAD_FUN(interval, handlefun) \
({\
    timer_t timerid;\
    struct sigevent evp;\
    memset(&evp, 0, sizeof(struct sigevent));\
\
    evp.sigev_value.sival_int = worker_id;\
    evp.sigev_notify = SIGEV_THREAD;\
    evp.sigev_notify_function = handlefun;\
\
    if (timer_create(CLOCK_REALTIME, &evp, &timerid) == -1)\
    {\
        perror("fail to timer_create");\
        exit(-1);\
    }\
\
    struct itimerspec it;\
    it.it_interval.tv_sec = (int)interval;\
    it.it_interval.tv_nsec = (long)( ( interval - (int)interval ) * 1.0e9 );\
    it.it_value.tv_sec = (int)interval;\
    it.it_value.tv_nsec = (long)( ( interval - (int)interval ) * 1.0e9 );\
\
    if (timer_settime(timerid, 0, &it, NULL) == -1)\
    {\
        perror("fail to timer_settime");\
        exit(-1);\
    }\
})


int main(int argc, char* argv[])
{
    pid_t pid;
    char fn_logger[NAME_MAX];
    struct timespec tsnow = {0,0};

    init_arguments(&args);
    argp_parse (&argp, argc, argv, 0, 0, &args);
    valid_arguments(&args);
    prepare_env(&args);

    log_e_rec.epoch = args.epoch;
    log_e_rec.workers = args.workers;
    log_e_rec.ttfs_reclen = args.recordlength;
    log_e_rec.ttfs_recnum = args.recordnum;
    log_e_rec.files = args.generatedfiles;
    TIMER_NOW(log_e_rec.start_timestamp, tsnow);

    sprintf( fn_logger, "/tmp/ttfslog.%04d", args.epoch );
    logger = (ttfs_logfile_t){ fn_logger, -1, lfinit, lfwrite, lfclose };
    logger.init(&logger);

    for ( worker_id = 0; 
        worker_id < args.workers; worker_id++) {
        pid = fork();
        
        if ( pid == 0 ) {
            logrec_process();

            TIMER_THREAD_FUN( 0.1, timer_thread );
            gen_origfiles( worker_id );

            logrec_process();
            return(0);
        }
    }

    for (int i=0; i<args.workers; i++) {
            wait(NULL);
    }

    TIMER_NOW(log_e_rec.end_timestamp, tsnow);
    logrec_epoch();
    logger.close(&logger);
    return(0);        
}
