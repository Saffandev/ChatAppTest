//client

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>

#pragma comment(lib,"ws2_32.lib")


void SendMessageToServer ( SOCKET socket, bool& bSendSuccess )
{
	while(1)
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
			std::cerr << "Send Failed: " << WSAGetLastError ( ) << std::endl;
			bSendSuccess = 0;
			break;
		}
		bSendSuccess = 1;
	}
}

void ReceiveMessage ( SOCKET socket, bool& bReceiveSuccess )
{
	while(1)
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
			break;
		}
		else
		{
			std::cerr << "Connection_Client Failed: " << WSAGetLastError ( ) << std::endl;
			bReceiveSuccess = 0;
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
	
	WSADATA wsadata;
	int Result;
	
	Result = WSAStartup ( MAKEWORD ( 2, 2 ), &wsadata );
	if ( Result != 0 )
	{
		std::cerr << "WSAStartup Failed: " << WSAGetLastError ( ) << std::endl;
		return 1;
	}
	SOCKET ConnectSocket = INVALID_SOCKET;

	ConnectSocket = socket ( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if ( ConnectSocket == INVALID_SOCKET )
	{
		std::cerr << "Socket_Client Creation Failed: " << WSAGetLastError ( ) << std::endl;
		WSACleanup ( );
		return 1;
	}

	sockaddr_in ClientAddr;
	ClientAddr.sin_family = AF_INET;
	ClientAddr.sin_port = htons ( 8080 );
	inet_pton ( AF_INET, "127.0.0.1", &ClientAddr.sin_addr );
	Result = connect ( ConnectSocket, ( SOCKADDR* ) &ClientAddr, sizeof ( ClientAddr ) );
	if ( Result == SOCKET_ERROR )
	{
		std::cerr << "Socket_Client connection failed: " << WSAGetLastError ( ) << std::endl;
		closesocket ( ConnectSocket );
		WSACleanup ( );
		return 1;
	}
	std::cout << "Client..." << std::endl;
	bool bMessageSent = 1;
	std::thread t1 ( SendMessageToServer, ConnectSocket, std::ref(bMessageSent) );
		
	bool bMessageReceived = 1;
	std::thread t2 ( ReceiveMessage, ConnectSocket, std::ref ( bMessageReceived ) );
	t1.join ( );
	t2.join ( );
	
	//while ( true )
	//{
	//	bool bMessageSent = 1;
	//	std::thread t1 ( SendMessageToServer, ConnectSocket, std::ref(bMessageSent) );
	//	t1.detach ( );
	//	if ( !bMessageSent )
	//	{
	//		break;
	//	}

	//	bool bMessageReceived = 1;
	//	std::thread t2 ( SendMessageToServer, ConnectSocket, std::ref ( bMessageReceived ) );
	//	t2.detach ( );
	//	if ( !bMessageReceived )
	//	{
	//		break;
	//	}

	//	/*if ( !SendMessage ( ConnectSocket, ) )
	//	{
	//		break;
	//	}
	//	if ( !ReceiveMessage ( ConnectSocket ) )
	//	{
	//		break;
	//	}*/
	//	
	//}

	closesocket ( ConnectSocket );
	WSACleanup ( );
	return 0;
}