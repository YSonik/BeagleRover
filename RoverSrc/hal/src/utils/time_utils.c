#define _POSIX_C_SOURCE 200809L
#include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <errno.h> // Errors
#include <string.h>

#include <sys/epoll.h> // for epoll()
#include <fcntl.h>     // for open()
#include <unistd.h>    // for close()

#include "utils/time_utils.h"

long long getTimeInMs(void)
{
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);
    long long seconds = spec.tv_sec;
    long long nanoSeconds = spec.tv_nsec;
    long long milliSeconds = seconds * 1000 + nanoSeconds / 1000000;
    return milliSeconds;
}

long long getTimeInUs(void)
{
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);
    long long seconds = spec.tv_sec * 1000000LL; // Note: LL specifies long long literal
    long long nanoSeconds = spec.tv_nsec / 1000; // Divide by 1000 to convert nanoseconds to microseconds
    long long microSeconds = seconds + nanoSeconds;
    return microSeconds;
}

void sleepForMs(long long delayInMs)
{
    const long long NS_PER_MS = 1000 * 1000;
    const long long NS_PER_SECOND = 1000000000;
    long long delayNs = delayInMs * NS_PER_MS;
    int seconds = delayNs / NS_PER_SECOND;
    int nanoseconds = delayNs % NS_PER_SECOND;
    struct timespec reqDelay = {seconds, nanoseconds};
    nanosleep(&reqDelay, (struct timespec *)NULL);
}

void sleepForNs(long long delayInNs)
{
    const long long NS_PER_SECOND = 1000000000;
    int seconds = delayInNs / NS_PER_SECOND;
    int nanoseconds = delayInNs % NS_PER_SECOND;
    struct timespec reqDelay = {seconds, nanoseconds};
    nanosleep(&reqDelay, (struct timespec *)NULL);
}
