#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define load 1

// Dummy Task (periodic)
void periodicTask() {

	for(int i =0; i < load *1000; i ++) {
		/* do nothing , keep counting */
	}

    time_t t;
    time(&t);
    printf("RT task done at %s\n", ctime(&t));
}

// Signal handler
void signalHandler(int sig) {
    printf("Missed Deadline\n");
    periodicTask();
}

// Creating timer and setting up timer variables
void timerSetup() {
    timer_t timer;
    struct itimerspec timer_time;

    // Create a new timer that will send the default SIGALRM signal
    if (timer_create(CLOCK_REALTIME, NULL, &timer) != 0) {
        perror("timer_create");
        exit(1);
    }

    timer_time.it_value.tv_sec = 1; 
    timer_time.it_value.tv_nsec = 0;
    timer_time.it_interval.tv_sec = 2; 
    timer_time.it_interval.tv_nsec = 500000000;

	// Schedule the timer
    if (timer_settime(timer, 0, &timer_time, NULL) != 0) {
        perror("timer_settime");
        exit(1);
    }
}

// Setup of signal handler
void signalHandlerSetup() {
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    sigemptyset(&action.sa_mask);

    action.sa_handler = signalHandler;

    if (sigaction(SIGALRM, &action, NULL) != 0) {
        perror("sigaction");
        exit(1);
    }
}


int main()
{
    int result = -1;
    sigset_t waitset;
	int signum;
	
    timerSetup();
    signalHandlerSetup();

    sigemptyset(&waitset);
    sigaddset(&waitset, SIGALRM);

    while (1) {
        sigprocmask(SIG_BLOCK, &waitset, NULL);

        if ((result = sigwait(&waitset, &signum)) == 0) {
            printf("sigwait returned for signal %d\n", signum);
            periodicTask();
        } else {
            printf("sigwait returned error number %d\n", result);
            perror("sigwait() function failed\n" );
        }

        sigprocmask(SIG_UNBLOCK, &waitset, NULL);
        sleep(rand() % 5);
    }
}
