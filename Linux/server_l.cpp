//server
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <thread>
#include <unistd.h>
#include <vector>
#include <signal.h>

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define PORT 8080

std::vector<int> ConnectdClients;

void ListenToClients(const int& ServerSocket)
{

    fd_set readset;
    int MaxSocketNum;
    std::cout<<"listening to clients " <<std::endl;
    if(listen(ServerSocket,SOMAXCONN) < 0)
    {
        perror("Listen Failed: ");
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
        timeval serverTimeOut;
        serverTimeOut.tv_sec = 5;
        serverTimeOut.tv_usec = 0;
        int Result = select(MaxSocketNum + 1 , &readset, NULL, NULL, &serverTimeOut);
        if(Result == SOCKET_ERROR)
        {
            perror("Select gave some error: ");
            break;
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
                perror("Accept Failed");
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
            if(client > 0 && FD_ISSET(client,&readset) )
            {
                char message[100];
                int Result = read(client,message,100);
                if(Result <= 0)
                {
                    perror("No data to read from socket");
                    close(client);
                    client = 0;
                }
               else
               { 
                    for(const int& c : ConnectdClients)
                    {
                        if(c != client)
                        {
                            if(send(c,message,100,0) < 0)
                            {
                                perror("Send Failed");
                            }
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
    }

}

void HandleTermination(int sign)
{
    std::cout<<"Termination called from the terminal\n";
    exit(EXIT_FAILURE);
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

    signal(SIGINT,HandleTermination);
	int Result;

	//socket for listen server
	int ListenSocket = INVALID_SOCKET;

	ListenSocket = socket ( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if ( ListenSocket == INVALID_SOCKET )
	{
		perror("Socket Creation Falied");
		close(ListenSocket);
		
		return 1;
	}
	std::cout << "Socket Creation successful" << std::endl;

    int sockopt = 1;
    if(setsockopt(ListenSocket,SOL_SOCKET,SO_REUSEADDR,&sockopt,sizeof(sockopt)) < 0)
    {
        perror("SocketOption: ");
        close(ListenSocket);
        return 1;
    }

	sockaddr_in serversockaddr;
	serversockaddr.sin_family = AF_INET;//address family for ipv4
	serversockaddr.sin_addr.s_addr = INADDR_ANY;//can accept any ip
	serversockaddr.sin_port = htons ( PORT );
	Result = bind ( ListenSocket, (sockaddr*) &serversockaddr, sizeof ( sockaddr ) );
	if ( Result == SOCKET_ERROR )
	{
        perror("Unable to bind to socket");
		close ( ListenSocket );
        std::string er = "sudo lsof -i :" + std::to_string(PORT);
        system(er.c_str());
		return 1;
	}
    std::cout<<"Bind to socket successful"<<std::endl;

       
    std::thread ListenThread(ListenToClients,std::ref(ListenSocket));
    ListenThread.join();

	close ( ListenSocket );
    for(int clientSocket : ConnectdClients)
    {
        close(clientSocket);
    }
}
