//server
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <thread>
#include <unistd.h>

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

void PrintError(const char* Message = " ")
{
    perror(Message) ;
}

void SendMessageToClient ( int socket, bool& bSuccess )
{
	while ( 1 )
	{
		std::cout << "Server: ";
		std::string messagetoSend;
		std::getline ( std::cin, messagetoSend );
		messagetoSend = "Server: " + messagetoSend + '\0';
		int sResult;
		//this function will return the number of bites received, 0 if connection is closed, and <0 if receive failed
		sResult = send ( socket, messagetoSend.c_str ( ), messagetoSend.size ( ), 0 );
		if ( sResult == -1 )
		{
			PrintError("Send Failed");
			bSuccess = 0;
			break;
		}
		bSuccess = 1;
	}
}

void ReceiveMessage ( int socket, bool& bSuccess )
{
	while ( 1 )
	{
		int rResult;
		char buff[ 200 ];
		rResult = recv ( socket, buff, 200, 0 );
		if ( rResult > 0 )
		{
			std::cout << buff << std::endl;
			bSuccess = 1;
		}
		else if ( rResult == 0 )
		{
			std::cout << "Connection CLosed by the client\n";
			break;
		}
		else
		{
			std::cerr << "Connection Failed\n";
			break;
		}
		bSuccess = 0;
	}
}

int main ( )
{
	/*create wsadata
	wsa startup
	create socket
	bind socket
	listen to client
	accept req
	recv data
	clean up*/

	int Result;

	//socket for listen server
	int ListenSocket = INVALID_SOCKET;

	ListenSocket = socket ( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if ( ListenSocket == INVALID_SOCKET )
	{
		PrintError("Socket Creation Falied");
		close(ListenSocket);
		
		return 1;
	}
	std::cout << "Socket Creation successful" << std::endl;

	sockaddr_in serversockaddr;
	serversockaddr.sin_family = AF_INET;//address family for ipv4
	serversockaddr.sin_addr.s_addr = INADDR_ANY;//can accept any ip
	serversockaddr.sin_port = htons ( 8080 );
	Result = bind ( ListenSocket, (sockaddr*) &serversockaddr, sizeof ( sockaddr ) );
	if ( Result == SOCKET_ERROR )
	{
        PrintError("Unable to bind to socket");
		close ( ListenSocket );
		std::cin.get ( );
		return 1;
	}
    std::cout<<"Bind to socket successful"<<std::endl;

	Result = listen ( ListenSocket, SOMAXCONN );// SOMAXCONN sets max connections allowed in queue
	if ( Result == SOCKET_ERROR )
	{
		PrintError("Listen Failed ");
		close ( ListenSocket );
		return 1;
	}
    std::cout<<"Listening to client requests"<<std::endl;

	int ClientSocket = INVALID_SOCKET;
	ClientSocket = accept ( ListenSocket, NULL, NULL );
	if ( ClientSocket == INVALID_SOCKET )
	{
		PrintError("Accept failed");
		close ( ClientSocket );
		close ( ListenSocket );
		return 1;
	}
    std::cout<<"Accepted a client request: "<<ClientSocket<<std::endl;
	bool bMessageReceived;
	std::thread t1 ( ReceiveMessage, ClientSocket, std::ref ( bMessageReceived ) );

	bool bMessageSend;
	std::thread t2 ( SendMessageToClient, ClientSocket, std::ref ( bMessageSend ) );
	t1.join ( );
	t2.join ( );


	close ( ClientSocket );
	close ( ListenSocket );
}
