#include <stdio.h>
#include <time.h>


//struct itimerspec{
//    struct timespec it_inteval;     /* timer period */
//    struct timespec it_value;       /* timer expiration */
//};

struct timespec{
    timer_t tv_sec;                  /* seconds */
    long   tv_nsec;                 /* nanoseconds */
};   


int main(){
    timer_t timer1;

    /* Create timer */

}