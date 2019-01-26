#include "TimeFunctions.h"

int CreateTimer( int clockid )
{
    int fd = timerfd_create( clockid, TFD_NONBLOCK);
    if( fd == -1 )
	ERROR("Produce Timer create error. ");

    return fd;
}

void SetTimer( float intervalInSeconds, int fd )
{
    struct itimerspec ITimerSpec;

    ITimerSpec.it_interval.tv_sec = (int)intervalInSeconds;
    ITimerSpec.it_interval.tv_nsec = (intervalInSeconds - (int)intervalInSeconds) * 1000000000;
    
    ITimerSpec.it_value.tv_sec = (int)intervalInSeconds;
    ITimerSpec.it_value.tv_nsec = (intervalInSeconds - (int)intervalInSeconds) * 1000000000;

    int res;
    if( (res = timerfd_settime( fd, 0, &ITimerSpec, NULL)) == -1)
	ERROR("Setting Produce Timer error. ");
}

void CheckTime( struct timespec* TimeStructure, clockid_t ClockType )
{
    int res = 0;
    if( ( res = clock_gettime( ClockType, TimeStructure) ) == -1 )
	ERROR("clock_gettime() error. ");
}

float DeltaT( struct timespec First, struct timespec Second )
{   
    float sec = Second.tv_nsec - First.tv_nsec; 
    if( sec < 0 )
    {
	sec = ( Second.tv_nsec + 1000000000) - First.tv_nsec;
	First.tv_sec--;
    }
    sec = sec/1000000000.f;

    return (Second.tv_sec-First.tv_sec)+sec;
}
