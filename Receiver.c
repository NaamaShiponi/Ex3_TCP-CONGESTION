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
#define BUFFER_SIZE 1024

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
    int sendcount = 0;

    time starting_time, ending_time;
    double countTimeTCP = 0;
    double countTimeCC = 0;
    double count = 0;

    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(SERVER_PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    int serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));

    int listenResult = listen(serverSocket, 1);
    if (listenResult == -1)
    {
        printf("listen failed, error code : %d\n", errno);
        close(serverSocket);
        return -1;
    }

    printf("Receiver listen ip: 0.0.0.0 port: 2020 \n");

    struct sockaddr_in clientAddress;
    socklen_t clientAddressLen = sizeof(clientAddress);

    gettimeofday(&starting_time, NULL); // start counting

    for (size_t i = 0; i < 1; i++)
    {
        memset(&clientAddress, 0, sizeof(clientAddress));
        clientAddressLen = sizeof(clientAddress); // TODO CHECKE
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressLen);
        if (clientSocket == -1)
        {
            printf("listen failed with error code : %d", errno);
            // close the sockets
            close(serverSocket);
            return -1;
        }

        printf("get A new maseage from client\n");

        char buffer[BUFFER_SIZE];
        memset(buffer, 0, BUFFER_SIZE);
        int bytesReceived;
        for (size_t i = 0; i < 10; i++)
        {
            bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesReceived < 0)
                break;
            sendcount++;
        }

        printf("sendcount: %d\n", sendcount);
        if (bytesReceived == -1)
        {
            printf("recv failed with error code : %d\n", errno);
            // close the sockets
            close(serverSocket);
            close(clientSocket);
            return -1;
        }

        // Reply to client
        char *message = "Welcome Naama shiponi and Ben dabush\n";
        int messageLen = strlen(message) + 1;

        int bytesSent = send(clientSocket, message, messageLen, 0);
        if (bytesSent == -1)
        {
            printf("send() failed with error code : %d\n", errno);
            close(serverSocket);
            close(clientSocket);
            return -1;
        }
        else if (bytesSent == 0)
        {
            printf("peer has closed the TCP connection prior to send().\n");
        }
        else if (bytesSent < messageLen)
        {
            printf("sent only %d bytes from the required %d.\n", messageLen, bytesSent);
        }
        else
        {
            printf("message was successfully sent.\n");
        }
        close(clientSocket);
    }

    gettimeofday(&ending_time, NULL); // stop counting
    double current_time = getAmountOfTime(starting_time, ending_time);
    countTimeTCP += current_time;
    printf("total time = %f\n", countTimeTCP);

    printf("------------------------- SET TO CC ------------------------- \n");
    char *CC = "reno";
    socklen_t socklen = strlen(CC);

    int setsockoptOutput = setsockopt(serverSocket, IPPROTO_TCP, TCP_CONGESTION, CC, socklen);
    if (setsockoptOutput != 0)
    {
        perror("ERROR! socket setting failed!\n");
        return -1;
    }

    listenResult = listen(serverSocket, 1);

    gettimeofday(&starting_time, NULL); // start counting

    for (size_t i = 0; i < 1; i++)
    {
        memset(&clientAddress, 0, sizeof(clientAddress));
        clientAddressLen = sizeof(clientAddress); // TODO CHECKE
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressLen);
        if (clientSocket == -1)
        {
            printf("listen failed with error code : %d \n", errno);
            // close the sockets
            close(serverSocket);
            return -1;
        }

        printf("get A new maseage from client\n");

        char buffer[BUFFER_SIZE];
        memset(buffer, 0, BUFFER_SIZE);
        int bytesReceived;
        sendcount = 0;
        i = 0;
        for (size_t i = 0; i < 10; i++)
        {
            recv(clientSocket, buffer, BUFFER_SIZE, 0);
            sendcount++;
        }

        printf("sendcount: %d\n", sendcount);

        if (bytesReceived == -1)
        {
            printf("recv failed with error code : %d \n", errno);
            // close the sockets
            close(serverSocket);
            close(clientSocket);
            return -1;
        }

        // Reply to client
        char *message = "I didn't believe you would make it\n";
        int messageLen = strlen(message) + 1;

        int bytesSent = send(clientSocket, message, messageLen, 0);
        if (bytesSent == -1)
        {
            printf("send() failed with error code : %d\n", errno);
            close(serverSocket);
            close(clientSocket);
            return -1;
        }
        else if (bytesSent == 0)
        {
            printf("peer has closed the TCP connection prior to send().\n");
        }
        else if (bytesSent < messageLen)
        {
            printf("sent only %d bytes from the required %d.\n", messageLen, bytesSent);
        }
        else
        {
            printf("message was successfully sent.\n");
        }
        close(clientSocket);
    }
    gettimeofday(&ending_time, NULL); // stop counting
    current_time = getAmountOfTime(starting_time, ending_time);
    countTimeCC += current_time;
    printf("total time = %f\n", countTimeCC);

    count++;

    close(serverSocket);
    return 0;
}
