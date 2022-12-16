#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <signal.h>

#define SERVER_PORT 2020
#define BUFFER_SIZE 8192

int resvAndsend(struct sockaddr_in clientAddress, int serverSocket, int clientSocket);

typedef struct timeval time;

// calculate the amount of time it takes to get the packets.
double getAmountOfTime(time starting_time, time ending_time)
{
    double total_time = ((ending_time.tv_sec * 1000000 + ending_time.tv_usec) -
                         (starting_time.tv_sec * 1000000 + starting_time.tv_usec));
    return total_time;
}

int main()
{

    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(SERVER_PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    int serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    int yes = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1)
    {
        perror("setsockopt");
        exit(1);
    }

    bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));

    int listenResult = listen(serverSocket, 1);
    if (listenResult == -1)
    {
        printf("listen failed: %d\n", errno);
        close(serverSocket);
        return -1;
    }

    printf("Receiver listen, ip: 0.0.0.0 port: 2020 \n");

    struct sockaddr_in clientAddress;
    memset(&clientAddress, 0, sizeof(clientAddress));
    socklen_t clientAddressLen = sizeof(clientAddress);

    int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressLen);
    if (clientSocket == -1)
    {
        printf("accept() failed: %d \n", errno);
        close(serverSocket);
        return -1;
    }
    printf("get a new maseage from client\n");

    int resvAndsendOutput = resvAndsend(clientAddress, serverSocket, clientSocket);
    if (resvAndsendOutput == -1)
    {
        printf("resvAndsend() failed \n");
        close(serverSocket);
        return -1;
    }

    printf("\n------------------------- SET TO CC ------------------------- \n\n");

    socklen_t cclen;
    char CC[256];
    strcpy(CC, "reno");
    cclen = strlen(CC);

    int setsockoptOutput = setsockopt(serverSocket, IPPROTO_TCP, TCP_CONGESTION, CC, cclen);
    if (setsockoptOutput != 0)
    {
        perror("ERROR! socket setting failed!\n");
        return -1;
    }
    if (getsockopt(serverSocket, IPPROTO_TCP, TCP_CONGESTION, CC, &cclen) != 0)
    {
        perror("ERROR! socket getting failed!");
        return -1;
    }
    printf("set sockopt to CC is successfully\n");

    resvAndsendOutput = resvAndsend(clientAddress, serverSocket, clientSocket);
    if (resvAndsendOutput == -1)
    {
        printf("error resvAndsend()\n");
        close(serverSocket);
        return -1;
    }

    // close(serverSocket);
    return 0;
}

int resvAndsend(struct sockaddr_in clientAddress, int serverSocket, int clientSocket)
{
    char authentication[] = "0011010001110100";
    time starting_time, ending_time;
    double countTimeTCP = 0;
    double countTimeCC = 0;
    int count = 0;

    gettimeofday(&starting_time, NULL);
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    int bytesReceived;
    for (size_t i = 0; i < 8; i++)
    {
        bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (bytesReceived == -1)
        {
            printf("recv() failed: %d \n", errno);
            close(serverSocket);
            close(clientSocket);
            return -1;
        }
        count++;

    }
    printf("%d packages were received from the customer\n",count);

    int bytesSent = send(clientSocket, authentication, sizeof(authentication), 0);
    if (bytesSent == -1)
    {
        printf("send() failed: %d\n", errno);
        close(serverSocket);
        close(clientSocket);
        return -1;
    }
    else if (bytesSent == 0)
    {
        printf("peer has closed the TCP connection prior to send().\n");
    }
    else if (bytesSent < sizeof(authentication))
    {
        printf("sent authentication\n");
    }
    else
    {
        printf("message was successfully sent.\n");
    }

    gettimeofday(&ending_time, NULL); // stop counting
    double current_time = getAmountOfTime(starting_time, ending_time);
    countTimeCC += current_time;
    printf("total time = %f\n", countTimeCC);

    return 0;
}
