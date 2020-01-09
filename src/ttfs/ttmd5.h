/** ttfs ver0.1 **/
#ifndef HEADER_TTMD5
#define HEADER_TTMD5

#include <openssl/md5.h>
#include "timemeter.h"
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>

#define RDBUFSIZE 4096

int md5sumf( const unsigned char* infilename, 
            unsigned char* mdout, double *read_tlen )
{
    int infd;
    ssize_t len_in;
    char buf[RDBUFSIZE];

    struct timespec tstart = {0, 0}, tend = {0, 0};

    MD5_CTX c;
    MD5_Init(&c);

    if ( strcmp("stdin", infilename) == 0 )
        infd = 0;
    else
        infd = open(infilename, O_RDONLY);

    if (infd == -1) {
        perror("open");
    return(EXIT_FAILURE);
    }

    int first = 1;
    while( first || len_in > 0 ){
        first = 0;
        TIMER_START(tstart);
        len_in = read(infd, &buf, RDBUFSIZE);
        TIMER_END(*read_tlen, tend, tstart);
        MD5_Update( &c, buf, len_in );
    }
    MD5_Final(mdout, &c);

    close(infd);
    return(EXIT_SUCCESS);
}



#endif
