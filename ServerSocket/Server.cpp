//server
#include <WinSock2.h>
#include <WS2tcpip.h>//this is the extension if winsock2 for tcp/ip
#include <iostream>
#include <thread>

#pragma comment(lib,"ws2_32.lib")


void SendMessageToClient (SOCKET socket, bool &bSuccess )
{
	while(1)
	{
		std::cout << "Server: ";
		std::string messagetoSend;
		std::getline ( std::cin, messagetoSend );
		messagetoSend = "Server: " + messagetoSend + '\0';
		int sResult;
		//this function will return the number of bites received, 0 if connection is closed, and <0 if receive failed
		sResult = send ( socket, messagetoSend.c_str ( ), messagetoSend.size ( ), 0 );
		if ( sResult == SOCKET_ERROR )
		{
			std::cerr << "Send Failed\n";
			bSuccess = 0;
			break;
		}
		bSuccess = 1;
	}
}

void ReceiveMessage ( SOCKET socket, bool &bSuccess )
{
	while(1){
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

	WSADATA wsadata;
	int Result;
	//this will fill wsadata with network data required and return 0 if its successful
	Result = WSAStartup ( MAKEWORD ( 2, 2 ), &wsadata );
	
	if ( Result != 0 )
	{
		std::cerr << "WSA unable to start: " << WSAGetLastError ( ) << std::endl;
		WSACleanup ( );
		return 1;
	}

	//socket for listen server
	SOCKET ListenSocket = INVALID_SOCKET;

	ListenSocket = socket ( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if ( ListenSocket == INVALID_SOCKET )
	{
		std::cerr << "Socket_Server Creation Failed: " << WSAGetLastError ( ) << std::endl;
		closesocket ( ListenSocket );
		WSACleanup ( );
		return 1;
	}
	std::cout << "Socket Creation successful" << std::endl;

	sockaddr_in sockaddr;
	sockaddr.sin_family = AF_INET;//address family for ipv4
	sockaddr.sin_addr.s_addr = INADDR_ANY;//can accept any ip
	sockaddr.sin_port = htons ( 8080 );
	Result = bind ( ListenSocket, (SOCKADDR* ) & sockaddr, sizeof ( sockaddr ) );
	if ( Result == SOCKET_ERROR )
	{
		std::cerr << "Socket_Server error: " << WSAGetLastError ( ) << std::endl;
		closesocket ( ListenSocket );
		std::cin.get ( );
		WSACleanup ( );
		return 1;
	}

	Result = listen ( ListenSocket, SOMAXCONN );// SOMAXCONN sets max connections allowed in queue
	if ( Result == SOCKET_ERROR )
	{
		std::cerr << "Server Listen Failed: " << WSAGetLastError ( ) << std::endl;
		closesocket ( ListenSocket );
		WSACleanup ( );
		return 1;
	}

	/*fd_set fdread;
	FD_ZERO ( &fdread );
	FD_SET ( ListenSocket, &fdread );
	timeval time;
	time.tv_sec = 10;
	time.tv_usec = 0;
	int s = (int)ListenSocket + 1;

	Result = select ( s, &fdread, NULL, NULL, &time ); 
	std::cout << "Select Result: " << Result;
	std::cout << "Server..." << std::endl;*/

	SOCKET ClientSocket = INVALID_SOCKET;
	ClientSocket = accept ( ListenSocket, NULL, NULL );
	if ( ClientSocket == INVALID_SOCKET )
	{
		std::cerr << "Server Accept Failed: " << WSAGetLastError ( ) << std::endl;
		closesocket ( ClientSocket ); 
		closesocket ( ListenSocket );
		WSACleanup ( );
		return 1;
	}

	bool bMessageReceived;
	std::thread t1 ( ReceiveMessage, ClientSocket, std::ref(bMessageReceived) );

	bool bMessageSend;
	std::thread t2 ( SendMessageToClient, ClientSocket, std::ref(bMessageSend) );
	t1.join ( );
	t2.join();

	//while(1)
	//{
	//	bool bMessageReceived;
	//	std::thread t1 ( ReceiveMessage, ClientSocket, std::ref(bMessageReceived) );
	//	t1.detach ( );
	//	if ( !bMessageReceived )
	//	{
	//		break;
	//	}

	//	bool bMessageSend;
	//	std::thread t2 ( SendMessageToClient, ClientSocket, std::ref(bMessageSend) );
	//	t2.detach ( );
	//	if ( !bMessageSend )
	//	{
	//		break;
	//	}
	//	
	//	////data received
	//	//if( !ReceiveMessage ( ClientSocket ) )
	//	//{
	//	//	break;
	//	//}
	//	////data send
	//	//if ( !SendMessage ( ClientSocket ) )
	//	//{
	//	//	break;
	//	//}
	//}

	closesocket ( ClientSocket );
	closesocket ( ListenSocket );
	WSACleanup ( );
}
