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

#define ACC_SOCK 0
#define TIM_PROD 1
#define TIM_REP 2

// Global Variables
char Path[80] = {0};
char Addr[80] = "localhost";
int port = 0;
int TotalClients = 0;

struct pollfd* PollTable;
struct BufferChar ProduceBuffer; 
struct BufferInt ToSendBuffer; 

FILE* Report;

// In main() functions
int ReadArguments( int argc, char* argv[]);
void PrepareServer();
void MainLoop( int Tempo );

// Usage Functions
int FillProduceBuffer( struct BufferChar, int LastIdx );
int readToTempBuffer(struct BufferChar ProduceBuffer, char* TempBuffer); 
void PlaceIntoPollTable( int ClientFd );
int CreateAcceptSocket();
struct sockaddr_in AcceptAndPlaceInPollTab( int socketFd );
void OpenFileToWrite();

// Time Functions
int CreateTimer( int clockid );
void SetTimer( float intervalInSeconds, int TimerFD );
void CheckTime( struct timespec* TimeStructure, clockid_t ClockType );
void SleepMe( int Tempo );

// Usage Functions
void WriteReport( FILE* OutputFile, char* ClientAddress, int TotalClients, int ReportType );
void PrintUsage();



int main( int argc, char* argv[])
{
    int Tempo = ReadArguments(argc, argv);
     
    
    
    
        //Realizacja usługi zapis.odczyt informacji. 
    //read(); write();
    //ssize_t recv(int sockfd, void* buf, size_t len, int flags);
    //ssize_t send(int sockfd, void* buf, size_t len, int flags);
    
    //Zamkniecie polaczenia
    //int close( int fd );
    //int shutdown( int sockfd, int how) - inf trafia do drugiej strony.
    

    fclose( Report );    
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

void PrepareServer()
{
    ProduceBuffer = CreateRoundBufferChar(1250000/sizeof(char)); 

    //Utworzenie kolejki cyklicznej ToSend
    ToSendBuffer = CreateRoundBufferInt(1000);
    
    //Utworzenie Tablicy wsystkoch deskryptorów
    //int* AllDescriptors = (int*)calloc(10, sizeof(int));

    //Utworzenie Tablicy dla Poll.
    PollTable = (struct pollfd*)calloc(4, sizeof(struct pollfd));
    
        //Utworzenie socketu do połączeń.
    int AccSock = CreateAcceptSocket();
	//Wpisanie fd do tablic AllFd oraz Poll
	//AllDescriptors[ACC_SOCK] = AccSock;
	PollTable[ACC_SOCK].fd = AccSock;
	PollTable[ACC_SOCK].events = POLLIN;
	
    //Utworzenie Zegara Produkcyjnego
    int TimerProd = CreateTimer( CLOCK_REALTIME );
	//Wpisanie fd do tablic AllFd oraz Poll
	//AllDescriptors[TIM_PROD] = TimerProd;
	PollTable[TIM_PROD].fd = TimerProd;
	PollTable[TIM_PROD].events = POLLIN;

    //Utworzenie Zegara Raportowego
    int TimerReport = CreateTimer( CLOCK_REALTIME );
	//Wpisanie fd do tablic AllFd oraz Poll
        //AllDescriptors[TIM_REP] = TimerReport;
	PollTable[TIM_REP].fd = TimerReport;
	PollTable[TIM_REP].events = POLLIN;

    OpenFileToWrite();
}

int CreateAcceptSocket()
{
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
     
    return sock_fd;
}

struct sockaddr_in AcceptAndPlaceInPollTab( int socketFd )
{
    //Kod odpowiedzialny za powstanie nowego socketu do klienta.
    struct sockaddr_in Client;
    socklen_t ClientLen;
    int Client_fd;
    if( (Client_fd = accept( socketFd, (struct sockaddr*)&Client, &ClientLen)) == -1)
       ERROR("New client acceptance error. ");	
    
    PlaceIntoPollTable( Client_fd );
    return Client;
}

void PlaceIntoPollTable( int ClientFd )
{
    static int idx = 0;
    struct pollfd Client;
    Client.fd = ClientFd;
    Client.events = POLLIN;

    if( idx == sizeof(PollTable)/sizeof(*PollTable) )
	//realloc
    
    PollTable[idx] = Client;
    TotalClients++;
    idx++;
}

void OpenFileToWrite()
{
    FILE* Output = fopen( Path, "a");
    if( !Output )
	ERROR("Filed to open/create report file. ");

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    fprintf( Output, "\n----- Server start running %d.%d.%d at %d:%d:%d. ----- \n", tm.tm_mday, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
 
    Report = Output;   
}


void MainLoop( int Tempo )
{
    //Utworzenie Buforu Pomocniczego 
    char* TempBuffer = (char*)calloc(112000/sizeof(char), sizeof(char));
    char* fd_buffer = (char*)calloc(8, sizeof(char));

    //Wystartowanie zegara produkcyjnego
    SetTimer( Tempo*60/96.f, PollTable[TIM_PROD].fd );
    //Wystartowanie zegara raportowego
    SetTimer( 5, PollTable[TIM_REP].fd );
    
    int jobs = 0;
    int LastIdx = 0;
    while( 1 )
    {
	// Poll na wszystkich deskrypotrach
	if( !jobs )
	jobs = poll( PollTable, sizeof(PollTable)/sizeof(*PollTable), -1);	
    	
	// Sprawdzenie zegara produkcja
	if( read( PollTable[TIM_PROD].fd, fd_buffer, 8) > 0 )
	{
	    if( !jobs ) return;

	    LastIdx = FillProduceBuffer( ProduceBuffer, LastIdx );
	    jobs--; 
	} 
	
	//Sprawdzenie Nadejscia nowego polczenia
	if( read( PollTable[ACC_SOCK].fd, fd_buffer, 4) > 0 )
	{
	    if( !jobs ) return;

	    struct sockaddr_in newClient = AcceptAndPlaceInPollTab( PollTable[ACC_SOCK].fd );
	    jobs--;
	}
	
	//Sprawdzenie zegara raport
	 if( read( PollTable[TIM_REP].fd, fd_buffer, 4) > 0 )
	{
	    if( !jobs ) return;
	    
	    WriteReport( Report, NULL, TotalClients, 3);
	    jobs--;
	}
	
	//Sprawdzenie deskryptorów Klientów
	unsigned long i = 3;
	while( jobs && (i < sizeof(PollTable)/sizeof(*PollTable)) )
	{
	    if( read( PollTable[i].fd, fd_buffer, 4) > 0 )
	       pushInt(ToSendBuffer, PollTable[i].fd);
	    
	    i++;	    
	}

	//Sprawdzenie, czy w TempBuffer są wszystkie dane do wysylki.
	if( readToTempBuffer( ProduceBuffer, TempBuffer) == 0 )
	{
	    int Client = 0;
	    if( (Client = popInt( ToSendBuffer )) != 0 )
	    {
		//Send to Client
	    }
	}
	//Wyslanie danych do fd pierwszego klienta z listy.
	
	
    }
}

int FillProduceBuffer( struct BufferChar FillBuffer, int LastIdx )
{
    int i = 0;
    if( LastIdx )
	i = LastIdx;

    while( i < 160 )	//640 bytes
    {
	if( pushChar( FillBuffer, 'A'  ) == 0 )
	{
	    return i;
	}
	
	i++;
    }

    return 0; 
} 

int readToTempBuffer(struct BufferChar ProduceBuffer, char* TempBuffer, int LastIdx )
{
    unsigned long i = 0;
    if( !LastIdx )
	i = LastIdx;
    
    while( i < sizeof(TempBuffer)/sizeof(*TempBuffer) )
    {
	if( (TempBuffer[i] = popChar( ProduceBuffer ) != '\0' ) )
		i++;
	else
	    return i;
    }

    return 0;
}


int* CreateSendQueue()
{

    return NULL;
}

struct pollfd* CreatePollTable()
{
    
    return NULL;
}

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

void WriteReport( FILE* OutputFile, char* ClientAddress, int TotalClients, int ReportType )
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
	printf("Producent -r <path> -t <val> [<addr>:]port\n\nUsage:\n\t\t<path>  - localization of report file,\n\t\t<val>  -  producing data rate (1-8 second * 60 / 96)[s]\n\t\t[<addr>:]port  -  server localization. Suggested:  \":8000\"\n\n");
}
