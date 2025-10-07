//client

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <arpa/inet.h>

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

void PrintError(const char* Message = " ")
{
   perror(Message);
}


void SendMessageToServer ( int socket, bool& bSendSuccess )
{
	while ( 1 )
	{
		std::cout << "Client: ";
		std::string message;
		std::getline ( std::cin, message );
		char hostname[ 20 ];
		gethostname ( hostname, 20 );
		message = std::string ( hostname ) + ": " + message;
		int Result = send ( socket, message.c_str ( ), 200, 0 );

		if ( Result == SOCKET_ERROR )
		{
			PrintError("SendFailed");
			bSendSuccess = 0;
			break;
		}
		bSendSuccess = 1;
	}
}

void ReceiveMessage ( int socket, bool& bReceiveSuccess )
{
	while ( 1 )
	{
		char recvData[ 200 ];
		int Result = recv ( socket, recvData, 200, 0 );
		if ( Result > 0 )
		{
			std::cout << recvData << std::endl;
			bReceiveSuccess = 1;
		}
		else if ( Result == 0 )
		{
			std::cout << "Connection Closed" << std::endl;
			bReceiveSuccess = 0;
            close(socket);
			exit ( EXIT_FAILURE );
			break;
		}
		else
		{
			PrintError("Connection_Client Failed");
			bReceiveSuccess = 0;
            close(socket);
			exit ( EXIT_FAILURE );
			break;
		}
	}
}




int main ( )
{

	/*wsa startup
	socket
	connect to server
	send
	recv
	close*/

	int Result;
	int ConnectSocket = INVALID_SOCKET;

	ConnectSocket = socket ( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if ( ConnectSocket == INVALID_SOCKET )
	{
		PrintError("Socket_Client Creation Failed");
		return 1;
	}

	sockaddr_in ClientAddr;
	ClientAddr.sin_family = AF_INET;
	ClientAddr.sin_port = htons ( 8080 );
	inet_pton ( AF_INET, "127.0.0.1", &ClientAddr.sin_addr );
	Result = connect ( ConnectSocket, ( sockaddr* ) &ClientAddr, sizeof ( ClientAddr ) );
	if ( Result == SOCKET_ERROR )
	{
		PrintError("Socket_Client connection failed");
		close ( ConnectSocket );
		return 1;
	}
	std::cout << "Client..." << std::endl;
	bool bMessageSent = 1;
	std::thread t1 ( SendMessageToServer, ConnectSocket, std::ref ( bMessageSent ) );

	bool bMessageReceived = 1;
	std::thread t2 ( ReceiveMessage, ConnectSocket, std::ref ( bMessageReceived ) );
	t1.join ( );
	t2.join ( );


	close ( ConnectSocket );
	return 0;
}