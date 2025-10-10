#include <map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <thread>
#include <vector>
#include <string>
#include <signal.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <cstring>
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

class Server
{

private:
    std::vector<int> ConnectdClients;
    int maxClientCount ;
    int port;
public:
    int Result;
    int ServerSocket;
    sockaddr_in ServerSockAddr;
    std::string ConfigFileName;

public:
    Server();
    bool SetupServer();

public:
    void ListenToClients();
    static void HandleTermination(int sign);
    std::map<std::string,std::string> ExtractDataFromFile(std::string ConfigFile);

};