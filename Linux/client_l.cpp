//client
#include "client_l.h"


Client::Client()
{
	ServerSocket = INVALID_SOCKET;
}

void Client::SendMessageToServer ( )
{
	while ( 1 )
	{
		std::cout << "Client: ";
		std::string message;
		std::getline ( std::cin, message );
		char hostname[ 20 ];
		gethostname ( hostname, 20 );
		message = std::string ( hostname ) + ": " + message;
		int Result = send ( ServerSocket, message.c_str ( ), 200, 0 );

		if ( Result == SOCKET_ERROR )
		{
			perror("SendFailed");
			break;
		}
	}
}

void Client::ReceiveMessageFomServer ( )
{
	while ( 1 )
	{
		char recvData[ 200 ];
		int Result = recv ( ServerSocket, recvData, 200, 0 );
		if ( Result > 0 )
		{
			std::cout << recvData << std::endl;
		}
		else if ( Result == 0 )
		{
			std::cout << "Connection Closed" << std::endl;
            close(ServerSocket);
			exit ( EXIT_FAILURE );
			break;
		}
		else
		{
			perror("Connection_Client Failed");
            close(ServerSocket);
			exit ( EXIT_FAILURE );
			break;
		}
	}
}




bool Client::SetupClient ( )
{

	/*wsa startup
	ServerSocket
	connect to server
	send
	recv
	close*/

	ServerSocket = socket ( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if ( ServerSocket == INVALID_SOCKET )
	{
		perror("ServerSocket_Client Creation Failed");
		return 1;
	}

	sockaddr_in ClientAddr;
	ClientAddr.sin_family = AF_INET;
	ClientAddr.sin_port = htons ( 8080 );
	inet_pton ( AF_INET, "127.0.0.1", &ClientAddr.sin_addr );
	Result = connect ( ServerSocket, ( sockaddr* ) &ClientAddr, sizeof ( ClientAddr ) );
	if ( Result == SOCKET_ERROR )
	{
		perror("ServerSocket_Client connection failed");
		close ( ServerSocket );
		return 1;
	}
	std::cout << "Client..." << std::endl;
	std::thread t1 ( &Client::SendMessageToServer,this );
	std::thread t2 ( &Client::ReceiveMessageFomServer,this);
	t1.join ( );
	t2.join ( );


	close ( ServerSocket );
	return 0;
}

int main()
{
	Client c;
	bool bSuccess = c.SetupClient();
	return bSuccess;
}