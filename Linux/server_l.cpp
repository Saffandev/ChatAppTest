//server
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <thread>
#include <unistd.h>
#include <vector>

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

std::vector<int> ConnectdClients;

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

void ReceiveMessage ()
{
	while ( 1 )
	{
        if(ConnectdClients.size() < 2)
        {
            continue;
        }
        for(int SenderClient : ConnectdClients)
        {
            char MessageReceived[100];
            if(recv(SenderClient,MessageReceived,100,0) > 0)
            {
                for(int client : ConnectdClients)
                {
                    if(client != SenderClient)
                    {
                        if(send(client,MessageReceived,100,0) <= 0)
                        {
                            PrintError("Failed To Send Message");
                            return ;
                        }
                    }
                }
            }
            else
            {
                PrintError("Receive Message Failed");
            }
        }
	}
}

void ListenToClients(const int& ServerSocket)
{

    fd_set readset;
    int MaxSocketNum;
    std::cout<<"listening to clients " <<std::endl;
    if(listen(ServerSocket,SOMAXCONN) < 0)
    {
        PrintError("Listen Failed: ");
        exit(EXIT_FAILURE);
    }
    while(1)
    {     
        std::cout<<"this is the start of the while loop\n";
        //inititalising the fdset for everycall
        FD_ZERO(&readset);
        FD_SET(ServerSocket,&readset);
        MaxSocketNum = ServerSocket;

        //initialising fdset with new clients and if there is any new socket larget then MaxSocketNum, set MaxSocketNum = ClientSocket
        for(int ClientSocket : ConnectdClients)
        {
            if(ClientSocket > 0)
            {
                std::cout<<"Get a new client: " << ClientSocket << std::endl;
                FD_SET(ClientSocket,&readset);
                if(ClientSocket > MaxSocketNum)
                {
                    MaxSocketNum = ClientSocket;
                }
            }
        }
        std::cout<<"Running the select call\n";
        int Result = select(MaxSocketNum + 1 , &readset, NULL, NULL, NULL);
        if(Result == SOCKET_ERROR)
        {
            PrintError("Select gave some error: ");
        }
        std::cout<<"it was stuck at the select call\n";
        if(FD_ISSET(ServerSocket,&readset))
        {
            sockaddr_in ClientAddr;
            int ClientSocket;
            socklen_t sl = sizeof(ClientAddr);
            ClientSocket = accept(ServerSocket,NULL,NULL);
            if(ClientSocket <=0)
            {
                PrintError("Accept Failed");
                exit(EXIT_FAILURE);
            }
            std::cout<<"Get a client accept request: " << ClientSocket <<std::endl;
            //now we have a new socket, lets add it to the ConnectedSockets
            ConnectdClients.push_back(ClientSocket);
        }
        else
        {
            std::cout<<"Server is not ready for new request" << std::endl;
        }

        for(int &client : ConnectdClients)
        {
            if(FD_ISSET(client,&readset))
            {
                char message[100];
                int Result = read(client,message,100);
                if(Result <= 0)
                {
                    PrintError("No data to read from socket");
                    close(client);
                    client = 0;
                }
                for(const int& c : ConnectdClients)
                {
                    if(c != client)
                    {
                        if(send(c,message,100,0) < 0)
                        {
                            PrintError("Send Failed");
                        }
                    }
                }

            }
            else
            {
                std::cout<<"No fd set for client: " << client << std::endl;
            }
        }
        std::cout<<"-----------------\n";
        /*if(listen(ServerSocket,SOMAXCONN) == 0)
        {
            std::cout<<"listened to something" << std::endl;
            sockaddr_in ClientAddr;
            int ClientSocket;
            socklen_t sl = sizeof(ClientAddr);
            ClientSocket = accept(ServerSocket,(sockaddr*)&ClientAddr,&sl);
            if(ClientSocket != SOCKET_ERROR)
            {
                std::cout<< "Accepted a client with port: "<<ntohs(ClientAddr.sin_port);
                ConnectdClients.push_back(ClientSocket);
            }
            else
            {
                PrintError("Accept Failed");
            }

        }*/
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

	/*Result = listen ( ListenSocket, SOMAXCONN );// SOMAXCONN sets max connections allowed in queue
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
    }*/
       

    std::thread ListenThread(ListenToClients,std::ref(ListenSocket));
    ListenThread.join();
	//std::thread ReceiveMessageThread ( ReceiveMessage);
	//ReceiveMessageThread.join ( );

	// bool bMessageSend;
	// std::thread t2 ( SendMessageToClient, ClientSocket, std::ref ( bMessageSend ) );
	// t2.join ( );

	close ( ListenSocket );
    for(int clientSocket : ConnectdClients)
    {
        close(clientSocket);
    }
}
