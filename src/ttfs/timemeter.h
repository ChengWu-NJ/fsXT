/** ttfs ver0.1 **/
#ifndef HEADER_TIMEMETER
#define HEADER_TIMEMETER

#include <time.h>

#define TIMER_START(tstart) \
({\
     clock_gettime(CLOCK_MONOTONIC, &tstart); \
})

#define TIMER_END(tlen, tend, tstart) \
({\
     clock_gettime(CLOCK_MONOTONIC, &tend); \
     tlen += (((double)tend.tv_sec + (1.0e-9)*(double)tend.tv_nsec)  \
             - ((double)tstart.tv_sec + (1.0e-9)*(double)tstart.tv_nsec)); \
})

#define TM_APPEND_TLEN(tlen, val) \
({\
     tlen += val;\
})

#define TIMER_NOW(doublenow, tsnow) \
({\
    clock_gettime(CLOCK_MONOTONIC, &tsnow); \
    doublenow = (double)tsnow.tv_sec + (1.0e-9)*(double)tsnow.tv_nsec; \
})

typedef struct {
    double open_tlen;
    double close_tlen;
    double mv_tlen;
    double write_tlen;
    double read_tlen;
} timemeter_t;


time_t tm_get_all(timemeter_t *a)
{
    return a->open_tlen + a->close_tlen + a->mv_tlen 
	    + a->write_tlen + a->read_tlen;
}

void tm_append_all(timemeter_t *a, timemeter_t *b)
{
    TM_APPEND_TLEN(a->open_tlen, b->open_tlen);
    TM_APPEND_TLEN(a->close_tlen, b->close_tlen);
    TM_APPEND_TLEN(a->mv_tlen, b->mv_tlen);
    TM_APPEND_TLEN(a->write_tlen, b->write_tlen);
    TM_APPEND_TLEN(a->read_tlen, b->read_tlen);
}

#endif
