#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <arpa/inet.h>

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

class Client
{

private:
    int ServerSocket;
    int Result;
    sockaddr_in ClientAddr;



public:
    Client();
    bool SetupClient();

private:
    void SendMessageToServer();
    void ReceiveMessageFomServer();

};