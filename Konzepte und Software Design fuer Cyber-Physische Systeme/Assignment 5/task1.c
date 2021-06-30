#include <stdio.h>
#include <time.h>
#include <signal.h>

/* some timer_create() parameters */
#define CLOCKID CLOCK_REALTIME      // CLOCK_REALTIME: A settable system-wide real-time clock
#define SEVP NULL                   // NULL:  a sigevent structure in which sigev_notify is SIGEV_SIGNAL, sigev_signo is SIGALRM, and sigev_value.sival_int is the timer id

/* some timer_settime() parameters */
#define FLAG 0                      // the timer is relative to 0
#define OLD_VALUE NULL              // no previous value

/* some sigaction(),sigwait() parameters */
#define SIG SIGALRM              // Alarm clock expired
#define OLD_ACT NULL                // No previous action



/*------- Handler for timer signal -------*/
static void handler(){
    timer_t t;
    char buffer[80];
    strftime(buffer,sizeof(buffer),"%Y-%m-%d %H:%M:%S",localtime(&t));
    printf("SIGALRM appear at:");
}

int main(){

    /*------- Establish the handler -------*/
    // The sigaction() system call is used to change the action taken by a process on receipt of a specific signal.
    struct sigaction new_act;
    new_act.sa_flags = SA_SIGINFO;
    new_act.sa_sigaction = handler;
    if(sigaction(SIG, &new_act,OLD_ACT) == -1){
        perror("sigaction failed");
        exit(1);
    }

    /*------- Synchronous Signal -------*/
    // The sigwait() function suspends execution of the calling thread until one of the signals specified in the signal set becomes pending
    // The function accepts the signal (removes it from the pending list of signals), and returns the signal number in sig.
    // set: the set of signals to await 
    // sig: the signal number
    sigset_t set;       
    int sig;
    sigemptyset(&set);          //initialize and empty the set
    sigaddset(&set, SIG);       //add signal SIG to the set
    if(sigwait(&set,&sig) == -1){
        perror("sigwait failed");
        exit(1);
    }
    sigdelset(&set,SIG);        //delete signal from the set


    /*------- Create timer -------*/
    // timer_create() creates a new per-process interval timer.
    // CLOCKID:                     specifies the clock that the new timer uses to measure time
    // SEVP:                        points to a sigevent structure that specifies how the caller should be notified when the timer expires.
    
    timer_t timer1; // timer id
    if(timer_create(CLOCKID, SEVP, &timer1) == -1){ 
        perror("timer_create failed");
        exit(1);
    }

    /*------- Set timer -------*/
    // timer_settime() arms or disarms the timer identified by timerid.
    // FLAG:                        specifies a relative timer
    // new_value:                   specifies the new initial value and the new interval for the timer
    // OLD_VALUE:                   the old timer's datum
    // new_value -> it_value:       set the timer to initially expire at the given time
    // new_value -> it_interval:    specified the period of the timer

    struct itimerspec timer1_value;                  // the new_value
    timer1_value.it_value.tv_sec     = 2;            // 2s
    timer1_value.it_value.tv_nsec    = 500000000;    // 0.5s -> after 2+0.5=2.5s timer first expire
    timer1_value.it_interval.tv_sec  = 0;            // 0s
    timer1_value.it_interval.tv_nsec = 100000000;    // 0.1s -> period of the timer is 0.1s
    if(timer_settime(timer1,FLAG, &timer1_value, OLD_VALUE) == -1){
        perror("timer_settime failed");
        exit(1);
    }

    while(1){
        printf("--------Here is the main loop--------");
        sleep(1);
    }

}