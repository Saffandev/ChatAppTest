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

class Server
{
    private:
std::vector<int> ConnectdClients;
int maxClientCount = 10;

public:
int Result;
int ServerSocket;
sockaddr_in ServerSockAddr;

public:
Server();
bool SetupServer();
void ListenToClients();
static void HandleTermination(int sign);


};