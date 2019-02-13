#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <openssl/md5.h>
#include <ctype.h>

#include "RoundBuffer.h"
#include "TimeFunctions.h"

float delay;	    //delay in seconds.
int rFlag = 0;
int sFlag = 0; 
char Addr[80] = "127.0.0.1";
int port = 0;
int Timer;

FILE* Report;

struct pollfd TimerPoll = {0, 0, 0};
struct pollfd ReadSock = {0, 0, 0};

//In main() functions
int PrepareClient();
int ReadArguments( int argc, char* argv[]);
void RunClientRun( int NumberOfPosts, int socket_fd );

//Usage Functions
void RunS( int NumberOfPosts, int sock_fd );
void RunR( int NumberOfPosts, int socket_fd );
int SendRequest( struct timespec* After, int socket_fd, int NumberOfPosts);
void OpenFileToWrite();
float RandomVal( char* argument );
unsigned char* ComputeMD5( unsigned char* MD5Table, char* TempBuffer);
void CheckIfLocalHost();
void PreparePoll( int sock_fd );

//Output functions
void WriteReport( FILE* OutputFile, int ReportType, float Latency1, float Latency2, unsigned char* MD5 );
void PrintUsage();


int main( int argc, char* argv[])
{
    int NumberOfPosts = ReadArguments(argc, argv);
    printf("Args: \n\t-# %d,\n\t -r/s %f rFlag:%d sFlag:%d,\n\t %s:%d\n", 
    	    NumberOfPosts, delay, rFlag, sFlag, Addr, port);
    
    int socket_fd = PrepareClient();
    RunClientRun( NumberOfPosts, socket_fd ); 
  
    close( socket_fd );
    return 0;
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

    if( (r = connect( sock_fd, (struct sockaddr*)&Addres, (socklen_t)sizeof(Addres)) ) == -1)
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

void PreparePoll( int sock_fd )
{
    TimerPoll.fd = Timer;
    TimerPoll.events = POLLIN;
    
    ReadSock.fd = sock_fd;
    ReadSock.events = POLLIN;
}

void RunR( int NumberOfPosts, int socket_fd )
{
    struct timespec AfterSend, AfterRead, AfterBlock;
   
    char* TempBuffer = (char*)calloc(112*1024, sizeof(char));
    
    PreparePoll( socket_fd );
    struct pollfd PollTable[2] = { TimerPoll, ReadSock };

    SetTimer( delay, Timer);

    int Incomes = 0;
    int IncomesNeed = NumberOfPosts;
    unsigned long res = 0;
    while( NumberOfPosts != 0 || Incomes != IncomesNeed  )	//Request every time period.
    {
	poll( PollTable, 2, -1);
	
	if( PollTable[0].revents & POLLIN )
	    if( NumberOfPosts && read( Timer, TempBuffer, 8) > 0 )
		NumberOfPosts = SendRequest( &AfterSend, socket_fd, NumberOfPosts);	

	if( PollTable[1].revents & POLLIN )
	{
	    CheckTime( &AfterRead, CLOCK_REALTIME );
	    if( (res = recv( socket_fd, TempBuffer, 112*1024, MSG_WAITALL)) > 0) //read( socket_fd, TempBuffer, 112*1024) ) > 0 )	 e
	    {
		CheckTime( &AfterBlock, CLOCK_REALTIME );
		
		unsigned char MD5Table[MD5_DIGEST_LENGTH] = {0};
		WriteReport( 
		    Report, 
		    2, 
		    DeltaT( AfterSend, AfterRead), 
		    DeltaT( AfterRead, AfterBlock),
		    ComputeMD5( MD5Table, TempBuffer));
		
		printf("Readed block of data. Size: %lu [char], last char: %c\n", res, TempBuffer[res]);	

		Incomes++;
	    }
	}
    }	
}

void RunS( int NumberOfPosts, int socket_fd )
{
    struct timespec AfterSend, AfterRead, AfterBlock;
   
    char* TempBuffer = (char*)calloc(112*1024, sizeof(char));
    
    PreparePoll( socket_fd );    
    
    int Incomes = 0;

    int IncomesNeed = NumberOfPosts;
    unsigned long res = 0;
    while( NumberOfPosts != 0 || Incomes != IncomesNeed  )	//Request after whole block readed.
    {
	if( NumberOfPosts > 0 )
	   NumberOfPosts = SendRequest( &AfterSend, socket_fd, NumberOfPosts);	
	
	if( poll( &ReadSock, 1, -1) )	//Blocking till read available.
	{
	    CheckTime( &AfterRead, CLOCK_REALTIME );
	    if( (res = read( socket_fd, TempBuffer, 112*1024) ) < 112*1024)
	    {
		WriteReport( Report, 3, 0, 0, 0);
	    }
	    else
	    {
		CheckTime( &AfterBlock, CLOCK_REALTIME );

		unsigned char MD5Table[MD5_DIGEST_LENGTH] = {0};
		WriteReport( 
		    Report, 
		    2, 
		    DeltaT( AfterSend, AfterRead), 
		    DeltaT( AfterRead, AfterBlock), 
		    ComputeMD5( MD5Table, TempBuffer));
		
		printf("Readed block of data. Size: %lu [char]\n", res);	
		
		Incomes++;
	    }		
	    
	    SetTimer( delay, Timer);
	    poll( &TimerPoll, 1, -1);
	    read( Timer, TempBuffer, 8);
	}
    }
}

int SendRequest( struct timespec* After, int socket_fd, int NumberOfPosts)
{
    write( socket_fd, "PwsL", 4);
    CheckTime( After, CLOCK_REALTIME );
    printf("Request for data sent. %lu chars. \n", sizeof(char));
    return --NumberOfPosts;
}

unsigned char* ComputeMD5( unsigned char* MD5Table, char* TempBuffer )
{
    MD5_CTX md5;
    MD5_Init( &md5 );
    MD5_Update( &md5, TempBuffer, strlen(TempBuffer));
    MD5_Final( MD5Table, &md5);
    
    return MD5Table;
}

int ReadArguments( int argc, char* argv[])
{
    if( argc < 3 )
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
    
    port = strtod( argv[optind], &EndPtr);
    if( *EndPtr != '\0' )
    {
	int idx = 0;
	while ( argv[optind][idx] != ':' )
	{
	    Addr[idx] = argv[optind][idx];
	    idx++;
	}

	Addr[idx] = '\0';

	port = strtod( argv[optind]+idx+1, &EndPtr);
	if( *EndPtr != '\0' )
	{
	    PrintUsage();
	    ERROR("Invalid internal argument near port number. ");
	}

	CheckIfLocalHost();
    }

    return NumberOfPosts;
}

void CheckIfLocalHost()
{
    int idx = 0;
    while( Addr[idx] )
    {
	Addr[idx] = tolower( Addr[idx] );
	idx++;
    }
    if( strcmp( Addr, "localhost") == 0 )
	strcpy( Addr, "127.0.0.1\0" );
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
    struct tm tm1 = *localtime(&t);
    if( fprintf( Report, "\n----- Client start running %d.%d.%d at %d:%d:%d. ----- \n", 
		tm1.tm_mday, tm1.tm_mon+1, tm1.tm_mday, tm1.tm_hour, tm1.tm_min, tm1.tm_sec) < 0)
	ERROR("First print error. OpenFileToWrite(). ");

    fflush(Report);
}

void WriteReport( FILE* OutputFile, int ReportType, float Latency1, float Latency2, unsigned char* MD5 )
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
			TimMonotonic.tv_sec+(TimMonotonic.tv_nsec/1000000000), 
			TimWall.tv_sec+(TimWall.tv_nsec/1000000000),
			getpid(), Addr, port);
	    }; break;
	case 2: 
	    {
		fprintf( OutputFile, "==== BLOCK %d ===\nLatency Request-Answer: %f\nLatency First Byte-Whole Block: %f\nMD5 of block: ",
			count, Latency1, Latency2);
		
		for(int i=0; i <MD5_DIGEST_LENGTH; i++) 
		{
	            fprintf(OutputFile, "%02x",MD5[i]);
		}

		fprintf( OutputFile, "\n\n" );
		count++;
	    }; break;
	case 3:
	    {
		fprintf( Report, "Read error!! Block %d corrupted. ", count);
	    } break;
    }

    fflush( Report );
}

void PrintUsage()
{
    printf("Konsument: -# <cnt> -r/-s <dly> [<addr>:]port\nUsage:\n\t-# <cnt>  Number of request sent to server.\n\t-r or -s <dly> Time peroid between: \n\t\t*requests sent (r option), \n\t\t*block read and request sent (s option).\n\t[<addr>:]port Adress of server (producer). Recommended \":8000\".\n"); 
}
