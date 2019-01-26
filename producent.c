#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <ctype.h>

#include "RoundBuffer.h"
#include "TimeFunctions.h"

#define ACC_SOCK 0
#define TIM_PROD 1
#define TIM_REP 2

struct ClientStr
{
    int ClientFd;
    int PackagesDelivered;
    in_port_t port;
    struct in_addr Adress;
};

// Variables
char Path[80] = {0};
char Addr[80] = "127.0.0.1";

int port = 0;
int TotalClients = 0;
int PollTableSize = 0;
int PacksSent = 0;
int PacksGen = 0;
int ProduceBufferUsage = 0;
int MaxProduceBufferUsage = 0;

struct pollfd* PollTable;
struct ClientStr* ClientsInfo; 
struct BufferChar ProduceBuffer; 
struct BufferInt ToSendBuffer; 

FILE* Report;

// In main() functions
int ReadArguments( int argc, char* argv[]);
void PrepareServer();
void MainLoop( int Tempo );

// Usage Functions
int FillProduceBuffer( struct BufferChar, int LastIdx );
int readToTempBuffer(struct BufferChar ProduceBuffer, char* TempBuffer, int LastIdx );
int CreateAcceptSocket();
void PlaceIntoPollTable( int ClientFd );
void FillInSendTable( int i, char* fd_buffer );
void AcceptAndPlaceInPollTab( int socketFd );
void PlaceClientInTab( struct sockaddr_in newClient, int ClientFd );
void OpenFileToWrite();
void TimeReportAction();

// Usage Functions
void WriteReport( FILE* OutputFile, int ClientIdx, int TotalClients, int ReportType );
void FinalReport();
void PrintUsage();


int main( int argc, char* argv[])
{
    int Tempo = ReadArguments(argc, argv);
    PrepareServer();
    MainLoop( Tempo ); 

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

    printf("Input Arguments: -r %s -t %d %s:%d\n", Path, tempo, Addr, port ); 
    return tempo;
}

void PrepareServer()
{
    ProduceBuffer = CreateRoundBufferChar(1250000/sizeof(char)); 
    MaxProduceBufferUsage = 1250000; 

    //Utworzenie kolejki cyklicznej ToSend
    ToSendBuffer = CreateRoundBufferInt(1000);
    
    //Utworzenie Tablicy dla informacji o klientach.
    ClientsInfo = (struct ClientStr*)calloc(10, sizeof(struct ClientStr));

    //Utworzenie Tablicy dla Poll.
    PollTableSize = 10;
    PollTable = (struct pollfd*)calloc(PollTableSize, sizeof(struct pollfd));
    
    //Utworzenie socketu do połączeń.
    int AccSock = CreateAcceptSocket();
	PollTable[ACC_SOCK].fd = AccSock;
	PollTable[ACC_SOCK].events = POLLIN;
	
    //Utworzenie Zegara Produkcyjnego
    int TimerProd = CreateTimer( CLOCK_REALTIME );
	PollTable[TIM_PROD].fd = TimerProd;
	PollTable[TIM_PROD].events = POLLIN;

    //Utworzenie Zegara Raportowego
    int TimerReport = CreateTimer( CLOCK_REALTIME );
	PollTable[TIM_REP].fd = TimerReport;
	PollTable[TIM_REP].events = POLLIN;

    OpenFileToWrite();
}

void MainLoop( int Tempo )
{
    //Utworzenie Buforu Pomocniczego 
    char* TempBuffer = (char*)calloc(112000/sizeof(char), sizeof(char));
    char* fd_buffer = (char*)calloc(120, sizeof(char));
    if( !TempBuffer || !fd_buffer)
	ERROR("Memory allocation error. MainLoop(). ");

    //Wystartowanie zegara produkcyjnego
    SetTimer( Tempo*60/96.f, PollTable[TIM_PROD].fd );
    //Wystartowanie zegara raportowego
    SetTimer( 5, PollTable[TIM_REP].fd );
    
    int LastIdx = 0;
    int TempBufferLastIdx = 0;

    printf("Server started!, press 'Enter' button to quit.\n");
    struct pollfd Temp;
    Temp.fd = STDIN_FILENO;
    Temp.events = POLLIN;
    
    while( poll( &Temp, 1, 0 ) == 0 )
    {
	// Poll na wszystkich deskrypotrach
	poll( PollTable, TotalClients+3, 0);	
    	
	// Sprawdzenie zegara produkcja
	if( read( PollTable[TIM_PROD].fd, fd_buffer, 8) > 0 )
	{   
	    LastIdx = FillProduceBuffer( ProduceBuffer, LastIdx );
	    PacksGen++;
	} 
	
	//Sprawdzenie Nadejscia nowego polczenia
	if( PollTable[ACC_SOCK].revents == POLLIN )
	    AcceptAndPlaceInPollTab( PollTable[ACC_SOCK].fd );
	
	//Sprawdzenie zegara raport
	if( read( PollTable[TIM_REP].fd, fd_buffer, 8) > 0 )
	    TimeReportAction();    
	
	//Sprawdzenie deskryptorów Klientów- wypelnianie tablicy zamówień.
	long i = 3;
	while( i < PollTableSize && TotalClients > 0 )
	{
	    FillInSendTable( i, fd_buffer); 
	    i++;	    
	}

	//Sprawdzenie, czy w TempBuffer są wszystkie dane do wysylki.
	if( ( TempBufferLastIdx = readToTempBuffer( ProduceBuffer, TempBuffer, TempBufferLastIdx)) == 0 )
	{
	    int Client = 0;
	    if( (Client = popInt( ToSendBuffer )) != 0 )
	    {
		//Wysyła dane do klienta. Jednego klienta.
		int res = 0;
		if( (res = send( Client, TempBuffer, sizeof(TempBuffer), 0)) == -1)
		    perror("Error sending message to client. ");
		
		PacksSent++;	
		//czyszczenie Bufora.
		memset( TempBuffer, 0, sizeof(TempBuffer)/sizeof(*TempBuffer) );
	    }
	}
    }

    FinalReport();
}

void TimeReportAction()
{
    WriteReport( Report, 0, TotalClients, 3);
    printf("Clock report written.\n");
    PacksGen = 0;
    PacksSent = 0;
}

void FillInSendTable( int i, char* fd_buffer )
{
     if( PollTable[i].revents == POLLNVAL )
	    {
		PollTable[i].revents = 0;
		PollTable[i].fd = -1;
		TotalClients--;
		WriteReport( Report, i-3, TotalClients, 2); 
	    }
	    else if( PollTable[i].fd > 0 && read( PollTable[i].fd, fd_buffer, 4) > 0 )
	    {
		PollTable[i].revents = 0;
 		pushInt(ToSendBuffer, PollTable[i].fd);
		printf("\t\tNew order from Client: %d\n", PollTable[i].fd);
	    }
}

int CreateAcceptSocket()
{
    //Utworzenie gniazda..
    int sock_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
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

void AcceptAndPlaceInPollTab( int socketFd )
{
    //Kod odpowiedzialny za powstanie nowego socketu do klienta.
    struct sockaddr_in Client;
    socklen_t ClientLen;
    int Client_fd;
    if( (Client_fd = accept4( socketFd, (struct sockaddr*)&Client, &ClientLen, SOCK_NONBLOCK)) == -1)
       ERROR("New client acceptance error. ");	 

    printf("New Client has come! \n");
    PlaceIntoPollTable( Client_fd );
    PlaceClientInTab( Client, Client_fd );
}

void PlaceIntoPollTable( int ClientFd )
{
    static int idx = 3;
    struct pollfd Client;
    Client.fd = ClientFd;
    Client.events = POLLIN | POLLNVAL;

    if( idx == PollTableSize )
	PollTable = (struct pollfd*)realloc( PollTable, (++PollTableSize)*sizeof(struct pollfd));

    PollTable[idx] = Client;
    TotalClients++;
    idx++;
}

void PlaceClientInTab( struct sockaddr_in newClient, int ClientFd )
{
    static int idx = 0;
    ClientsInfo[idx].ClientFd = ClientFd;
    ClientsInfo[idx].Adress = newClient.sin_addr;
    ClientsInfo[idx].port = newClient.sin_port;
    ClientsInfo[idx].PackagesDelivered = 0;

    WriteReport( Report, idx, TotalClients, 1);
    idx++;
} 

void OpenFileToWrite()
{
    Report = fopen( Path, "a");
    if( !Report )
	ERROR("Filed to open/create report file. ");

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    if( fprintf( Report, "\n----- Server start running %d.%d.%d at %d:%d:%d. ----- \n", 
		tm.tm_mday, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec) < 0)
	ERROR("First print error. OpenFileToWrite(). ");
}

int FillProduceBuffer( struct BufferChar FillBuffer, int LastIdx )
{
    static int Case = 0;
    static int ToSend = 0;
    int Temp;
    int i = 0;
    if( LastIdx )
	i = LastIdx;
    
    if( Case % 2 )
	Temp = tolower( (ToSend%25)+65 );
    else
	Temp = (ToSend%26)+65;

    while( i < 640 )	//640 bytes
    {
	if( pushChar( FillBuffer, (char)Temp  ) == 0 )
	    return i;
	i++;
	ProduceBufferUsage++;
    }
    
    printf("Buffer filled %dx'%c'. \n", i, (char)Temp);
    if( Case % 2) ToSend++;
    Case++;
    
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

void WriteReport( FILE* OutputFile, int ClientIdx, int TotalClients, int ReportType )
{
    struct timespec TimMonotonic, TimWall;
    CheckTime( &TimMonotonic, CLOCK_MONOTONIC );
    CheckTime( &TimWall, CLOCK_REALTIME );
    
    fprintf( OutputFile, "\n%ld [Monotonic]    %ld [RealTime aka WallTime]\n", 
	    TimMonotonic.tv_sec+(TimMonotonic.tv_nsec/1000000000), TimWall.tv_sec+(TimWall.tv_nsec/1000000000));
   
    switch (ReportType)
    {
	case 1: 
	    {
		fprintf(OutputFile, "New client adress: %u\n", 
			ClientsInfo[ClientIdx].Adress.s_addr);
	    }; break;
	case 2: 
	    {
		fprintf(OutputFile, "Client disconnect: %u.\nTotal packages sent: %d\n", 
			ClientsInfo[ClientIdx].Adress.s_addr, ClientsInfo[ClientIdx].PackagesDelivered);
	    }; break;
	case 3: 
	    {
		int bytesGen = PacksGen * 640;
		int bytesSent = PacksSent * 112000;
		int PercentUsage = ProduceBufferUsage * 100 / MaxProduceBufferUsage;
		fprintf( OutputFile, "Number of clients connected: %d,\nStorage usage: %d[B] (%d%% of capacity).\nData roll: %d\n", 
			TotalClients, ProduceBufferUsage, PercentUsage, bytesGen-bytesSent);
	    }; break;
    }

    fprintf(OutputFile, "\n====================\n");
    fflush(Report);
}

void FinalReport()
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    if( fprintf( Report, "\n----- Server ends work: %d.%d.%d at %d:%d:%d. ----- \n", 
		tm.tm_mday, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec) < 0)
	ERROR("End Work Message Error. ");
    fflush(Report);
}

void PrintUsage()
{
	printf("Producent -r <path> -t <val> [<addr>:]port\n\nUsage:\n\t\t<path>  - localization of report file,\n\t\t<val>  -  producing data rate (1-8 second * 60 / 96)[s]\n\t\t[<addr>:]port  -  server localization. Suggested:  \":8000\"\n\n");
}
