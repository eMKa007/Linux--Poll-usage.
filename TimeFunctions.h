#include <time.h>

int CreateTimer( int clockid );
void SetTimer( float intervalInSeconds, int TimerFD );
void CheckTime( struct timespec* TimeStructure, clockid_t ClockType );
float DeltaT( struct timespec First, struct timespec Secodn );


