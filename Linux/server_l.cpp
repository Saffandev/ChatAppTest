//server

#include "server_l.h"


Server::Server()
{
    ServerSocket = INVALID_SOCKET;
    signal(SIGINT,Server::HandleTermination);
    ConfigFileName = "serverconfig.txt";

}

std::map<std::string,std::string> Server::ExtractDataFromFile(std::string ConfigFile)
{
    std::map<std::string, std::string> ConfigData;
    std::fstream filestream;
    std::string filedata;
    filestream.open(ConfigFile);
    if(!filestream.is_open())
    {
       // std::cout<<"Cannot open the file " << ConfigFile << ", terminating the programming is there is not data to proceed;\n";
        syslog(LOG_ERR,"Cannot open the file %s terminating the program as there is no data to proceed, check for file name",ConfigFile.c_str());
        exit(EXIT_FAILURE);
    }
  
    while(std::getline(filestream,filedata))
    {
        if(filedata.empty() || filedata[0] == '#')
        {
            continue;
        }
        size_t pos = filedata.find('=');
        if(pos != std::string::npos)
        {
            std::string key = filedata.substr(0,pos);
            std::string value = filedata.substr(pos+1);
            ConfigData[key] = value;
        }
    }

    return ConfigData;

}

void Server::ListenToClients()
{

    fd_set readset;
    int MaxSocketNum;
    std::cout<<"listening to clients " <<std::endl;
    if(listen(ServerSocket,SOMAXCONN) < 0)
    {
      //  perror("Listen Failed: ");
        syslog(LOG_ERR,"Listen Failed: %s", strerror(errno));
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
        serverTimeOut.tv_sec = 20;
        serverTimeOut.tv_usec = 0;
        int Result = select(MaxSocketNum + 1 , &readset, NULL, NULL, &serverTimeOut);
        if(Result == SOCKET_ERROR)
        {
            syslog(LOG_ERR,"Select gave some error: %s", strerror(errno));
            break;
        }
        std::cout<<"it was stuck at the select call\n";
        if(ConnectdClients.size() < maxClientCount &&  FD_ISSET(ServerSocket,&readset))
        {
            sockaddr_in ClientAddr;
            int ClientSocket;
            socklen_t sl = sizeof(ClientAddr);
            std::cout<<"trying to accept something...\n";
            ClientSocket = accept(ServerSocket,NULL,NULL);
            if(ClientSocket <=0)
            {
                syslog(LOG_ERR,"Accept Falied: %s",strerror(errno));
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

        for(auto client_it = ConnectdClients.begin(); client_it != ConnectdClients.end(); client_it++)
        {
            std::cout<<*client_it <<std::endl;
            if(FD_ISSET(*client_it,&readset) )
            {
                char message[100];
                int Result = read(*client_it,message,100);
                if(Result <= 0)
                {
                    perror("No data to read from socket");

                    close(*client_it);
                    ConnectdClients.erase(client_it);
                }
               else
               { 
                    for(const int& c : ConnectdClients)
                    {
                        if(c != *client_it)
                        {
                            if(send(c,message,100,0) < 0)
                            {
                                syslog(LOG_ERR,"Send Failed: %s",strerror(errno));
                            }
                        }
                    }
                 }
            }
            else
            {
                std::cout<<"No fd set for client: " << *client_it << std::endl;
            }
        }
        std::cout<<"-----------------\n";
    }

}

void Server::HandleTermination(int sign)
{
    std::cout<<"Termination called from the terminal\n";
    exit(EXIT_FAILURE);
}

bool Server::SetupServer ( )
{
	/*create wsadata
	wsa startup
	create socket
	bind socket
	listen to client
	accept req
	recv data
	clean up*/

    std::map<std::string,std::string> ConfigData = ExtractDataFromFile(ConfigFileName);
    maxClientCount = std::stoi(ConfigData["maxclient"]);
    port = std::stoi(ConfigData["port"]);
    std::cout<<port << std::endl;
    
    ServerSocket = socket ( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if ( ServerSocket == INVALID_SOCKET )
	{
        syslog(LOG_ERR,"Socket Creation Failed: %s",strerror(errno));
		close(ServerSocket);
		
		return 1;
	}
	std::cout << "Socket Creation successful" << std::endl;

    int sockopt = 1;
    if(setsockopt(ServerSocket,SOL_SOCKET,SO_REUSEADDR,&sockopt,sizeof(sockopt)) < 0)
    {
        perror("SocketOption: ");
        close(ServerSocket);
        return 1;
    }

	
	ServerSockAddr.sin_family = AF_INET;//address family for ipv4
	ServerSockAddr.sin_addr.s_addr = INADDR_ANY;//can accept any ip
	ServerSockAddr.sin_port = htons ( port );
	Result = bind ( ServerSocket, (sockaddr*) &ServerSockAddr, sizeof ( sockaddr ) );
	if ( Result == SOCKET_ERROR )
	{
        syslog(LOG_ERR,"Unable to bind : %s",strerror(errno));
		close ( ServerSocket );
        std::string er = "sudo lsof -i :" + std::to_string(port);
        system(er.c_str());
		return 1;
	}
    std::cout<<"Bind to socket successful"<<std::endl;

       
    std::thread ListenThread(&Server::ListenToClients,this);
    ListenThread.join();

	close ( ServerSocket );
    for(int clientSocket : ConnectdClients)
    {
        close(clientSocket);
    }
    return 0;
}


int main()
{

    openlog("ServerLog",LOG_PID | LOG_CONS | LOG_PERROR, LOG_USER);
    Server s1;
    s1.SetupServer();
    closelog();
    return 0;
}