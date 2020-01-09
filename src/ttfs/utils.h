/** ttfs ver0.1 **/
#ifndef HEADER_UTILS
#define HEADER_UTILS

#include <stdlib.h>
#include <time.h>


void rand_str(char *dest, size_t length)
{
    char charset[] = "0123456789"
                     "abcdefghijklmnopqrstuvwxyz"
                     "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    struct timespec ts = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    srand(ts.tv_nsec);

    while (length-- > 0) {
        size_t index = (double) rand() / RAND_MAX * (sizeof charset - 1);
        *dest++ = charset[index];
    }
    *dest = '\0';
}

void bin2hex(unsigned char * out, const unsigned char *bin, size_t len)
{
    size_t  i;

    if (bin == NULL || len == 0)
        return;

    for (i=0; i<len; i++) {
        out[i*2]   = "0123456789abcdef"[bin[i] >> 4];
        out[i*2+1] = "0123456789abcdef"[bin[i] & 0x0F];
    }
    out[len*2] = '\0';

}

#endif

