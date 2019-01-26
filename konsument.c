//vim set sts=4 sw=4:
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/timerfd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>

#include "RoundBuffer.h"

#define ERROR(x) do{\
    perror(x);\
    exit(-1);\
}while(0)


float delay;	    //delay in seconds.
int rFlag = 0;
int sFlag = 0; 
char Addr[80] = "127.0.0.1";
int port = 0;
int Timer;

FILE* Report;

int ReadArguments( int argc, char* argv[]);

float RandomVal( char* argument );

int PrepareClient();
void RunClientRun( int NumberOfPosts, int socket_fd );
void RunS( int NumberOfPosts, int sock_fd );
void RunR( int NumberOfPosts, int socket_fd );
void OpenFileToWrite();
void WriteReport( FILE* OutputFile, int ReportType, int Latency1, int Latency2, char* MD5 );
void SetTimer( float intervalInSeconds, int fd );
int CreateTimer( int clockid );
void SleepMe( int Tempo );
void PrintUsage();

int main( int argc, char* argv[])
{
    int NumberOfPosts = ReadArguments(argc, argv);
    printf("Args: \n\t-# %d,\n\t -r/s %f rFlag:%d sFlag:%d,\n\t %s:%d\n", 
    	    NumberOfPosts, delay, rFlag, sFlag, Addr, port);
    
    int socket_fd = PrepareClient();
    RunClientRun( NumberOfPosts, socket_fd ); 

    return 0;
}

void RunClientRun( int NumberOfPosts, int socket_fd )
{
    switch( rFlag )
    {
	case 0:
	    {
		RunS( NumberOfPosts, socket_fd );
	    } break;
	case 1:
	    {
		RunR( NumberOfPosts, socket_fd );
	    } break;
	default: break;
    }
}

void RunR( int NumberOfPosts, int socket_fd )
{
    
    SetTimer( delay, Timer);
    //send first request here?
   
    char* TempBuffer = (char*)calloc(28000, sizeof(char));
    struct pollfd TimerPoll;
    TimerPoll.fd = Timer;
    TimerPoll.events = POLLIN;

    struct pollfd ReadSock;
    ReadSock.fd = sock_fd;
    ReadSock.events = POLLIN;
    
    int Incomes = 0;
    int IncomesNeed = NumberOfPosts;
    int res = 0;
    while( NumberOfPosts != 0 || Incomes == IncomesNeed  )	//Request every time period.
    {
	//If read from TimerDescriptor
	if( poll( &TimerPoll, 1, 0) )
	{
	    //Send request;
	    write( socket_fd, "a", sizeof(char));
	    NumberOfPosts--;

	    //Znaczniki Czasu.
	}

	//Read from socket descriptor, nieblokujaco.
	if( poll( &ReadSock, 1, 0) )
	{
	    //Znacznik Czasu 1. Latency 1. 
	    if( (res = read( socket_fd, TempBuffer, sizeof(TempBuffer)) ) < sizeof(TempBuffer))
	    {
	    }	
	    
	    //Znacznik Czasu 2.
	}
		//Jesli sie uda przeczytac
	    //oblicz sume, dodaj do raportu ze znacznikami czasu.
	    //Incomes++
    }	
}

void RunS( int NumberOfPosts, int socket_fd )
{
    int Incomes = 0;
    int IncomesNeed = NumberOfPosts;
    while( NumberOfPosts != 0 && Incomes == IncomesNeed  )	//Request after whole block read.    
    {
	//if read from timer descriptor
	    //Send Request
		//NumberOfPost--;
		//Kliknij znaczniki czasu
	
	//Read frim socket descriptor, blokujaco
	    //Oblicz sume, dodaj do raportu ze znacznikami czasi
	    //Incomes++;
    }	
}

int PrepareClient()
{
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if( sock_fd == -1 )
	ERROR("Socket create error. ");
    
    printf("Socket created! \n");

    struct sockaddr_in Addres;
    Addres.sin_family = AF_INET;
    Addres.sin_port = htons(port);

    int r;
    if( (r = inet_aton(Addr, &Addres.sin_addr)) == 0 )
	ERROR("Internet routine manipulation fail.. ");

    if( (r = connect( sock_fd, (struct sockaddr*)&Addres, sizeof(Addres)) ) == -1)
    {
	ERROR("Socket connect error.. ");
    }
    else if ( r == 0 )
    {
	printf("Client connected!\n");
    }
    OpenFileToWrite();
    
    Timer = CreateTimer( CLOCK_REALTIME );



    return sock_fd;
}

int ReadArguments( int argc, char* argv[])
{
    if( argc < 6 )
    {
	PrintUsage();
    	exit(-1);
    }

    char* EndPtr;
    int option;
    int NumberOfPosts = 0;
    while( ( option = getopt(argc, argv, "#:r:s:")) != -1)
    	{
	    switch(option)
	    {	
		case '#':
		{
		    NumberOfPosts = strtod(optarg, &EndPtr);
		    if( *EndPtr != '\0' )
		    {
			NumberOfPosts = (int)RandomVal( optarg );
		    }
		}; break;


		case 'r':
		{
		    if( rFlag == 1 || sFlag == 1 )
			continue;

		    delay = strtof(optarg, &EndPtr);
		    if( *EndPtr != '\0' )
		    {
			delay = RandomVal( optarg );
		    }
		    rFlag++;		
		}; break;

		case 's':
		{
		    if( rFlag == 1 || sFlag == 1 )
			continue;

		    delay = strtof(optarg, &EndPtr);
		    if( *EndPtr != '\0' )
		    {
			delay = RandomVal( optarg );
		    }
		    sFlag++;
		}
	    }
	}
    
    //Load Addr and port. 
    if( *argv[optind] == ':')
    {
	port = 	strtod( (argv[optind]+1), &EndPtr);
	if( *EndPtr != '\0')
	{ 
	    PrintUsage();
	    ERROR("Invalid internal argument. ");
	}
    }
    else
    {
	int j = 0;
	while( argv[optind][j] != ':' )
	{
	    Addr[j] = argv[optind][j];
	    j++;
	}
	Addr[j] = '\0';

	port = 	strtod( argv[optind]+j+1, &EndPtr);
	if( *EndPtr != '\0')
	{ 
	    PrintUsage();
	    ERROR("Invalid internal argument. ");
	}
    }

    return NumberOfPosts;
}

float RandomVal( char* argument )
{
    int j = 0;
    float FirstVal = 0;
    float SecondVal = 0;
    char Temp[80] = {0};
    
    char* EndPtr; 
    
    while( argument[j] != ':' )
    {
	Temp[j] = argument[j];
	j++;
    }
    
    FirstVal = strtof(Temp, &EndPtr);
    if( *EndPtr != '\0')
    { 
        PrintUsage();
        ERROR("Invalid internal argument. ");
    }
    
    SecondVal = strtof( &argument[j+1], &EndPtr );
    if( *EndPtr != '\0')
    { 
        PrintUsage();
        ERROR("Invalid internal argument. ");
    }
    
    if( FirstVal > SecondVal )
    {
	printf("Invalid input argument! First > Second\n");
	PrintUsage();
    }
    
    srand( time(NULL) );    
    float scale = rand() / (float) RAND_MAX;
    return FirstVal + scale * (SecondVal - FirstVal);
}

void SleepMe( int Delay )
{
    struct timespec tim, tim2;
    tim.tv_sec = Delay;
    tim.tv_nsec = 0;
    
    nanosleep(&tim, &tim2);
}

void OpenFileToWrite()
{
    char Path[] = "Report_Client";
    char pid[80] = {0};
    sprintf(pid, "%d", getpid());
    strcat(Path, pid);

    Report = fopen( Path, "a");
    if( !Report )
	ERROR("Filed to open/create report file. ");

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    if( fprintf( Report, "\n----- Client start running %d.%d.%d at %d:%d:%d. ----- \n", 
		tm.tm_mday, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec) < 0)
	ERROR("First print error. OpenFileToWrite(). ");

    fflush(Report);
}

// ---------------------------------------------------------------------------------------------- Time
int CreateTimer( int clockid )
{
    int fd = timerfd_create( clockid, 0);
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
//---------------------------------------------------------------------------------------------
void WriteReport( FILE* OutputFile, int ReportType, int Latency1, int Latency2, char* MD5 )
{
    static int count = 0;
        switch (ReportType)
    {
	case 1: 
	    {
		struct timespec TimMonotonic, TimWall;
		CheckTime( &TimMonotonic, CLOCK_MONOTONIC );
		CheckTime( &TimWall, CLOCK_REALTIME );

		fprintf( OutputFile, "\n%ld [Monotonic]    %ld [RealTime aka WallTime]\nClient Info: PID: %d\tAddres: %s:%d\n", 
			TimMonotonic.tv_sec+(TimMonotonic.tv_nsec/1000000000), TimWall.tv_sec+(TimWall.tv_nsec/1000000000),
			getpid(), Addr, port);
	    }; break;
	case 2: 
	    {
		fprintf( OutputFile, "==== BLOCK %d ===\nLatency Request-Answer: %d\nLatency First Byte-Whole Block: %d\nMD5 of block %s\n\n",
			count, Latency1, Latency2, MD5);
		count++;
	    }; break;
	case 3:
	    {
		fprintf( Report, "Read error!! Block %d corrupted. ", count);
	    } break;
    }
}

void PrintUsage()
{
    printf("Konsument: -# <cnt> -r/-s <dly> [<addr>:]port\nUsage:\n\t-# <cnt>  Number of request sent to server.\n\t-r or -s <dly> Time peroid between: \n\t\t*requests sent (r option), \n\t\t*block read and request sent (s option).\n\t[<addr>:]port Adress of server (producer). Recommended \":8000\".\n"); 
}
