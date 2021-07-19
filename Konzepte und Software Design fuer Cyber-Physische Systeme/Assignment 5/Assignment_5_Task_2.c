#include <time.h>
#include <signal.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static unsigned int PERIOD = 999;
static unsigned int load = 100000;
static int SCHEDULING_POLICY = SCHED_FIFO;
static int PRIORITY = 1;


// Dummy Task (periodic)
void periodicTask() {
    time_t t;
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_REALTIME, &start_time);

    for(int i =0; i < load *1000; i ++) {
		/* do nothing , keep counting */
	}

    clock_gettime(CLOCK_REALTIME, &end_time);
    double delta = (end_time.tv_sec - start_time.tv_sec) * 1000 + (end_time.tv_nsec - start_time.tv_nsec) / 1000000;

    time(&t);
    printf("RT task done at %.19s, duration: %.2f ms\n\n", ctime(&t), delta);
}

// Signal handler
void signalHandler(int sig) {
    printf("Missed Deadline\n");
    periodicTask();
}

// Creating timer and setting of timer variables
void timerSetup() {
	
    timer_t timer;
    struct itimerspec timer_spec;

    if (timer_create(CLOCK_REALTIME, NULL, &timer) != 0) {
        perror("timer_create");
        exit(1);
    }

    timer_spec.it_value.tv_sec = 1;
    timer_spec.it_value.tv_nsec = 0;
    timer_spec.it_interval.tv_sec = 0;
    timer_spec.it_interval.tv_nsec = PERIOD * 1000000;

    if (timer_settime(timer, 0, &timer_spec, NULL) != 0) {
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
    action.sa_flags = 0;

    if (sigaction(SIGALRM, &action, NULL) != 0) {
        perror("sigaction");
        exit(1);
    }
}

// Setting Scheduling Policy
void setSchedulingPolicy() {
    struct sched_param param;
    param.sched_priority = PRIORITY;

    if (sched_setscheduler(0, SCHEDULING_POLICY, &param) != 0) {
        perror("sched_setscheduler");
        exit(1);
    }
}

int main(int argc, char **argv) {

    int result = -1;
    sigset_t waitset;
	int opt, signum;

    while ((opt = getopt(argc, argv, "t:l:s:p:")) != -1) {
        switch (opt) {
            case 't': {
                unsigned int PERIOD = strtol(optarg, NULL, 10);
                if (PERIOD < 1 || PERIOD > 999) {
                    printf("period exceeds limits (1-999 ms), ignoring argument\n");
                }
            } break;

            case 'l': {
                load = strtol(optarg, NULL, 10);
            } break;

            case 's': {
                if (strlen(optarg) == 10 && strcmp(optarg, "SCHED_FIFO") == 0)
                    SCHEDULING_POLICY = SCHED_FIFO;
                else if (strlen(optarg) == 8 && strcmp(optarg, "SCHED_RR") == 0)
                    SCHEDULING_POLICY = SCHED_RR;
                else
                    printf("unknown scheduling policy supplied, falling back to default (SCHED_FIFO)\n");
            } break;

            case 'p': {
                const int min_prio = sched_get_priority_min(SCHEDULING_POLICY);
                const int max_prio = sched_get_priority_max(SCHEDULING_POLICY);
                int prio = strtol(optarg, NULL, 10);

                if (prio < min_prio || prio > max_prio) {
                    printf("priority exceeds limits (%d-%d), ignoring argument\n", min_prio, max_prio);
                    PRIORITY = min_prio;
                } else {
                    PRIORITY = prio;
                }
            } break;

            case '?': {
                printf("unknown option or missing argument: %c\n", optopt);
            } break;
        }
    }

    setSchedulingPolicy();
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
        //sleep(rand() % 3);
    }
}
