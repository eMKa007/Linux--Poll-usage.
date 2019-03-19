#include <time.h>
#include <sys/timerfd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define ERROR(x) do{\
    perror(x);\
    exit(-1);\
}while(0)


int CreateTimer( int clockid );
void SetTimer( float intervalInSeconds, int TimerFD );
void CheckTime( struct timespec* TimeStructure, clockid_t ClockType );
float DeltaT( struct timespec First, struct timespec Secodn );


