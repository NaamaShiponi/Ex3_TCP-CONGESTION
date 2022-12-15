#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

#define SERVER_PORT 2020
#define BUFFER_SIZE 1024
#define SIZE 1

int senderToServer(int ClentSocket, FILE *pfile);

int main()
{
    FILE *pfile;
    char *filename = "test.txt";

    int ClentSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in address_server;
    memset(&address_server, 0, sizeof(address_server));

    address_server.sin_family = AF_INET;
    address_server.sin_port = htons(SERVER_PORT);
    address_server.sin_addr.s_addr = INADDR_ANY;

    int connectResult = connect(ClentSocket, (struct sockaddr *)&address_server, sizeof(address_server));
    if (connectResult == -1)
    {
        printf("connect() failed with error code : %d", errno);
        close(ClentSocket);
        return -1;
    }

    printf("connected to server\n");

    pfile = fopen(filename, "r");
    if (!pfile)
    {
        printf("ERROR! file opening has failed!\n");
    }

    int senderToServerOutpot = senderToServer(ClentSocket, pfile);
    if (senderToServerOutpot == -1)
    {

        printf("error! senderToServer \n");
    }

    close(ClentSocket);

    printf("------------------------- SET TO CC -------------------------\n");
    socklen_t cclen;
    char *CC = "reno";
    cclen = strlen(CC);
    ClentSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    int setsockoptOutput = setsockopt(ClentSocket, IPPROTO_TCP, TCP_CONGESTION, CC, cclen);
    if (setsockoptOutput != 0)
    {
        perror("ERROR! socket setting failed!\n");
        return -1;
    }

    int connectOutput = connect(ClentSocket, (struct sockaddr *)&address_server, sizeof(address_server));
    if (connectOutput == -1)
    {
        perror("ERROR! connection has failed!\n");
    }
    else
    {
        printf("You have successfully connected to the server\n");
    }

    senderToServerOutpot = senderToServer(ClentSocket, pfile);
    if (senderToServerOutpot == -1)
    {

        printf("error! senderToServer \n");
    }

    return 0;
}

int senderToServer(int ClentSocket, FILE *pfile)
{

    char msg[BUFFER_SIZE];
    int count = 0;
    for (size_t i = 0; i < 10; i++)
    {
        fread(msg, 1, sizeof(msg), pfile);
        int bytesSent = send(ClentSocket, msg, sizeof(msg), 0);
        if (bytesSent == -1)
        {
            perror("ERROR! Sending has failed!\n");
            exit(1);
        }
        else if (bytesSent == 0)
        {
            printf("peer has closed the TCP connection prior to send().\n");
            return -1;
        }
        else
            count++;
    }

    printf("count send: %d\n", count);
    if (ferror(pfile))
    {
        perror("ERROR! file opening has failed!\n");
    }
    fclose(pfile);

    // Receive data from server
    char bufferReply[BUFFER_SIZE] = {'\0'};
    int bytesReceived = recv(ClentSocket, bufferReply, BUFFER_SIZE, 0);
    if (bytesReceived == -1)
    {
        printf("recv() failed with error code : %d \n", errno);
        return -1;
    }
    else if (bytesReceived == 0)
    {
        printf("peer has closed the TCP connection prior to recv().\n");
        return -1;
    }
    else
    {
        printf("received %d bytes from server: %s\n", bytesReceived, bufferReply);
    }
    return 0;
}
