/** ttfs ver0.1 **/
#include "../ttmd5.h"
#include "../utils.h"
#include <stdio.h>
#include <string.h>

int main(unsigned int argc, char* argv[]) {

    unsigned char mdbin[MD5_DIGEST_LENGTH], infilename[NAME_MAX] = "stdin";
    unsigned char mdstr[MD5_DIGEST_LENGTH*2+1];
    double read_tlen;

    if (argc > 1)
        strcpy(infilename,argv[1]);

    if ( md5sumf( infilename, mdbin, &read_tlen ) == EXIT_SUCCESS ) {
        bin2hex( mdstr, mdbin, MD5_DIGEST_LENGTH );
        printf( "%s, reading spent %f seconds.\n", mdstr, read_tlen );
    }

    return(0);
}
