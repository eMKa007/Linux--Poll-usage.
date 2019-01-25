//vim set sts=4 sw=4:
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/timerfd.h>
#include <poll.h>

#include "RoundBuffer.h"


#define ERROR(x) do{\
    perror(x);\
    exit(-1);\
}while(0)


char Path[80] = {0};
char Addr[80] = "localhost";
int port = 0;
int TotalClients = 0;

struct BufferChar ProduceBuffer; 
struct BufferInt ToSendBuffer; 

int ReadArguments( int argc, char* argv[]);
void PrepareServer(int Tempo);
void MainLoop();
void FillBuffer( struct Buffer );


int CreateTimer( float intervalInSeconds );
void SetTimer( float intervalInSeconds );
void CheckTime( struct timespec* TimeStructure, clockid_t ClockType );
void SleepMe( int Tempo );
void PrintUsage();

struct pollfd* DescriptorTable;
int* SendQueue;	    //Need to be changed to round buffer. 

int main( int argc, char* argv[])
{
    int Tempo = ReadArguments(argc, argv);
     
    
    
    
    // --------------   Wyniesc do innych funkcji!!!
    //Utworzenie gniazda..
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if( sock_fd  == -1 )
    {
	ERROR("Socket initialise error. ");
    }
    
    //Zarejetrowanie Lokalizacji/Adresu
    struct sockaddr_in Addres;
    Addres.sin_family = AF_INET;
    Addres.sin_port = htons(port);  
    
    int r;
    if( (r = inet_aton(Addr, &Addres.sin_addr)) == 0 )
      ERROR("Internet routine manipulation fail.. ");	

    if( (r = bind( sock_fd, (struct sockaddr*)&Addres, sizeof(Addres) ) ) == -1 )
	ERROR("Socket bind error. ");

    //Zmiana Trybu na pasywny
    if( (r = listen( sock_fd, 50)) == -1)	    //Up to 50 clients.
	ERROR("Change to passive socket error. ");
     
    //Oczekiwanie na polaczenie i akceptacja.
    struct sockaddr_in Client;
    socklen_t ClientLen;
    int Client_fd;
    if( (Client_fd = accept(sock_fd, (struct sockaddr*)&Client, &ClientLen)) == -1)
       ERROR("New client acceptance error. ");	

    //Realizacja usługi zapis.odczyt informacji. 
    //read(); write();
    //ssize_t recv(int sockfd, void* buf, size_t len, int flags);
    //ssize_t send(int sockfd, void* buf, size_t len, int flags);
    
    //Zamkniecie polaczenia
    //int close( int fd );
    //int shutdown( int sockfd, int how) - inf trafia do drugiej strony.
    
    return 0;
}

int ReadArguments( int argc, char* argv[])
{
    if( argc < 6)
    {
	PrintUsage();
    	exit(-1);
    }

    char* EndPtr;
    int option;
    int tempo = 0;   

    while( ( option = getopt(argc, argv, "r:t:")) != -1)
    	{
	    switch(option)
	    {
		case 'r':
		{
		   strcpy(Path, optarg);
		}; break;

		case 't':
		{
		    tempo = strtod(optarg, &EndPtr);
		    if( *EndPtr != '\0' || tempo < 1 || tempo > 8 )
		    {
			PrintUsage();
			ERROR("Invalid internal argument. ");
		    }
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
    return tempo;
}

void PrepareServer( int Tempo )
{
    ProduceBuffer = CreateRoundBufferChar(1250000/sizeof(char)); 

    //Utworzenie kolejki cyklicznej ToSend
    ToSendBuffer = CreateRoundBufferInt(1000);
    
    //Utworzenie Tablicy wsystkoch deskryptorów
    int* AllDescriptors = (int*)calloc(10, sizeof(int));

    //Utworzenie Tablicy dla Poll.
    struct pollfd* PollTable = (struct pollfd*)calloc(4, sizeof(struct pollfd));
    
    //Utworzenie Buforu Pomocniczego dla wysyłki.
    char* TempBuffer = (char*)calloc(128000/sizeof(char), sizeof(char));
    
    //Utworzenie socketu do połączeń.
	//Wpisanie fd do tablic AllFd oraz Poll
	
    //Utworzenie Zegara Produkcyjnego
    int TimerProd = CreateTimer( Tempo*60/96.f );
	//Wpisanie fd do tablic AllFd oraz Poll

    //Utworzenie Zegara Raportowego
    int TimerReport = CreateTimer( 5 );
	//Wpisanie fd do tablic AllFd oraz Poll

}


void MainLoop()
{
    //Wystartowanie zegara produkcyjnego
    //Wystartowanie zegara raportowego
    
    while( 1 )
    {
	// Poll na wszystkich deskrypotrach
	
	// Sprawdzenie zegara produkcja
	    //Ewentualna produkcja
	
	//Sprawdzenie Nadejscia nowego polczenia
	    //Ewentualna obsluga nowego polaczenia
	
	//Sprawdzenie zegara raport
	    //Ewentualny raport
	
	//Sprawdzenie deskryptorów Klientów
	//Jesli zwroci wartosc fd to wpisanie do listy wysylkowej.
	
	//Wyslanie danych do fd pierwszego klienta z listy. 

    }
}


void FillBuffer( struct Buffer FillBuffer )
{
    char* TempTable = (char*)calloc(160, sizeof(char));
    
    while ( 1 )
    {	
	int i = 0;
        memset( TempTable, 69, 160); 

	while( i < 160 )
	{
	    while( push( FillBuffer, TempTable[i] ) == 0 )
	    {
		//Wait...  Change it!.
	    }
	    
	    i++;
	}
	printf("%10s\n", TempTable);
    }
} 

void WriteReport( FILE* OutputFile, char* ClientAddress, int ReportType )
{
    struct timespec TimMonotonic, TimWall;
    CheckTime( &TimMonotonic, CLOCK_MONOTONIC );
    CheckTime( &TimWall, CLOCK_REALTIME );
    
    fprintf( OutputFile, "%ld [Monotonic]    %ld [RealTime aka WallTime]\n", TimMonotonic.tv_sec+(TimMonotonic.tv_nsec/1000000000), TimWall.tv_sec+(TimWall.tv_nsec/1000000000));
   
    switch (ReportType)
    {
	case 1: 
	    {
		fprintf(OutputFile, "New client adress: %s\n", ClientAddress);
	    }; break;
	case 2: 
	    {
		fprintf(OutputFile, "Client disconnect: %s.\nTotal data send: %d\n", ClientAddress, 100);
		//TODO: Send valid value of data send.
	    }; break;
	case 3: 
	    {
		fprintf( OutputFile, "Number of clients connected: %d,\nStorage usage: USAGE HERE.\nData roll: DATA ROLL here\n", TotalClients);
	    }; break;
    }

    fprintf(OutputFile, "\n====================\n");
}

int* CreateSendQueue()
{

    return NULL;
}

struct pollfd* CreatePollTable()
{
    
    return NULL;
}

int CreateTimer()
{
    int fd = timerfd_create( CLOCK_REALTIME, TFD_NONBLOCK);
    if( fd == -1 )
	ERROR("Produce Timer create error. ");

    return fd;
}

void SetTimer( float intervalInSeconds )
{
    struct itimerspec ITimerSpec;
    struct itimerspec ITimerSpecold;

    ITimerSpec.it_interval.tv_sec = (int)intervalInSeconds;
    ITimerSpec.it_interval.tv_nsec = (intervalInSeconds - (int)intervalInSeconds) * 1000000000;
    
    int res;
    if( (res = timerfd_settime( fd, 0, &ITimerSpec, &ITimerSpecold)) == -1)
	ERROR("Setting Produce Timer error. ");
}

void CheckTime( struct timespec* TimeStructure, clockid_t ClockType )
{
    int res = 0;
    if( ( res = clock_gettime( ClockType, TimeStructure) ) == -1 )
	ERROR("clock_gettime() error. ");
}

void SleepMe( int Tempo )
{
    float Tempo_Sec = Tempo * 60/96.f;
    struct timespec tim, tim2;
    tim.tv_sec = (int)Tempo_Sec;
    tim.tv_nsec = (Tempo_Sec - tim.tv_sec) * 1000000000;
    
    nanosleep(&tim, &tim2);
}

void PrintUsage()
{
	printf("Usage information here... \n");
}
