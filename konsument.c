//vim set sts=4 sw=4:
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "RoundBuffer.h"

#define ERROR(x) do{\
    perror(x);\
    exit(-1);\
}while(0)


float delay;	    //delay in seconds.
int rFlag = 0;
int sFlag = 0; 
char Addr[80] = "localhost";
int port = 0;

int ReadArguments( int argc, char* argv[]);

float RandomVal( char* argument );
void SleepMe( int Tempo );
void PrintUsage();

int main( int argc, char* argv[])
{
    int NumberOfPosts = ReadArguments(argc, argv);
    printf("Args: \n\t-# %d,\n\t -r/s %f rFlag:%d sFlag:%d,\n\t %s:%d\n", 
    	    NumberOfPosts, delay, rFlag, sFlag, Addr, port);
    
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if( sock_fd == -1 )
	ERROR("Socket create error. ");


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


    return 0;
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

void PrintUsage()
{
	printf("Usage information here... \n");
}
