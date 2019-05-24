### Client - Server app. (poll usage example)

To compile both client and server app just type 'make'. <br>
Example of linux  [poll(2)](http://man7.org/linux/man-pages/man2/poll.2.html). Server is running only when it have to ( period data generating, new client connection, data sending to client, client disconnection). <br>
Average CPU usage for both clients and server apps is below 1%. 

	Producent -r <path> -t <val> [<addr>:]port
    	
        Usage:<br>
        	<path>  - localization of report file,
        	<val>  -  producing data rate (1-8 second * 60 / 96)[s]
        	[<addr>:]port  -  server localization. Suggested:  "8000"
        
        examples: 	./Serwer.out -r report.txt -t 2 localhost:8000
        			./Serwer.out -r report.txt -t 2 12345:8000
                    ./Serwer.out -r report.txt -t 2 :8000
        
        
    Konsument: -# <cnt> -r/-s <dly> [<addr>:]port
    
    	Usage:
    		-# <cnt>  Number of request sent to server.
    		-r or -s <dly> Time peroid between:
        		*requests sent (r option),
        		*block read and request sent (s option).
    		[<addr>:]port Adress of server (producer). Have to be same as server ex.: ":8000".
            
		examples:	./Klient.out -# 10 -r 2 :8000
        			./Klient.out -# 2 -r 10 localhost:8000
                    ./Klient.out -# 15 -s 5 12345:8000

	Note: Start server first, and then connect any number of clients.

Server is using circle buffer to store generated data. <br>
After every client connection data packages are sent through created socket. <br>
Clients are queued while not enough data is available. <br>

Every time period report is generated with information about: <br>
 * Number of clients connected.
 * Bytes sent since last report information generation.
 * Bytes generated sine last report information generation.
 * Data roll.
 * Storage capacity ( with current % usage ).
 * On new clinet connection - Id of new client.
 * On client disconnection - Id of disconnected client with amount of data sent to him.
 
