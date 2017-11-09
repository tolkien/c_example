#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>

static int ref_count = 2;
static timer_t firstTimerID;
static timer_t secondTimerID;

static void firstCB ( int sig, siginfo_t *si, void *uc ) {
    struct timeval tv;
    if (gettimeofday(&tv, NULL) < 0) {
        printf("[%s] err %s\n", __FUNCTION__, strerror(errno));
    }

    printf("[%s] called at %ld:%ld\n", __FUNCTION__, tv.tv_sec, tv.tv_usec);
}

static void secondCB ( int sig, siginfo_t *si, void *uc ) {
    struct timeval tv;
    if (gettimeofday(&tv, NULL) < 0) {
        printf("[%s] err %s\n", __FUNCTION__, strerror(errno));
    }

    printf("[%s] called at %ld:%ld\n", __FUNCTION__, tv.tv_sec, tv.tv_usec);
}

static void timerHandler( int sig, siginfo_t *si, void *uc ) {
    timer_t *tidp;
    /* it's same as si->_sifields._timer.si_sigval.sival_ptr */
    tidp = si->si_value.sival_ptr;
               
    printf("[%s] signo %d, err %d, code %d\n", __FUNCTION__,
           si->si_signo, si->si_errno, si->si_code);
    printf("\tinfo: timerID %d, overcnt %d, tidp %x, %p\n",
           si->si_timerid, si->si_overrun, *((int *)tidp), uc);

    if ( *tidp == firstTimerID )
        firstCB(sig, si, uc);
    else if ( *tidp == secondTimerID )
        secondCB(sig, si, uc);

    ref_count -= 1;
}

static int signal_registered = 0;
static int makeTimer( char *name,
                      timer_t *timerID, int expireMS, int intervalMS ) {
    struct sigevent   te;
    struct itimerspec its;
    struct sigaction  sa;
    int               sigNo = SIGRTMIN;
    struct timeval tv;

    if (signal_registered == 0) {
    /* Set up signal handler. */
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = timerHandler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(sigNo, &sa, NULL) == -1) {
        fprintf(stderr, "Failed to setup signal handling for %s.\n", name);
        return(-1);
    }
    signal_registered = 1;
    }

    /* Set and enable alarm */
    te.sigev_notify = SIGEV_SIGNAL;
    te.sigev_signo = sigNo;
    te.sigev_value.sival_ptr = timerID;
    timer_create(CLOCK_REALTIME, &te, timerID);

    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = intervalMS * 1000000;
    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = expireMS * 1000000;
    timer_settime(*timerID, 0, &its, NULL);

    ref_count += 1;
    if (gettimeofday(&tv, NULL) < 0) {
        printf("[%s] err %s\n", __FUNCTION__, strerror(errno));
    }
    printf("[%s:%x] create at %ld:%ld\n", name, *((int *)timerID),
           tv.tv_sec, tv.tv_usec);
    return(0);
}

int main( void ) {
    int ret;

    ret = makeTimer("First Timer", &firstTimerID, 40, 0);
    if (ret < 0)
        return ret;

    ret = makeTimer("Second Timer", &secondTimerID, 120, 60);
    if (ret < 0)
        return ret;

    while (ref_count > 0) {
        sleep(1);
    }
    return 0;
}
