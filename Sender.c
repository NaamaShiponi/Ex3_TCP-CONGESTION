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
#define MSG_SIZE 8192
#define BUFFER_SIZE 1024
#define NAME_FILE "test.txt"

int senderToServer(int ClentSocket, FILE *pfile);

int main()
{
    FILE *pfile;
    char *filename = NAME_FILE;

    int ClentSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in address_server;
    memset(&address_server, 0, sizeof(address_server));

    address_server.sin_family = AF_INET;
    address_server.sin_port = htons(SERVER_PORT);
    address_server.sin_addr.s_addr = INADDR_ANY;

    int connectResult = connect(ClentSocket, (struct sockaddr *)&address_server, sizeof(address_server));
    if (connectResult == -1)
    {
        printf("connect() failed : %d", errno);
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

    printf("\n------------------------- SET TO CC -------------------------\n\n");

    socklen_t cclen;
    char CC[256];
    strcpy(CC, "reno");
    cclen = strlen(CC);

    int setsockoptOutput = setsockopt(ClentSocket, IPPROTO_TCP, TCP_CONGESTION, CC, cclen);

    if (setsockoptOutput != 0)
    {
        perror("error! setsockopt() failed!\n");
        return -1;
    }
    printf("set sockopt to CC is successfully\n");

    cclen = sizeof(CC);

    senderToServerOutpot = senderToServer(ClentSocket, pfile);
    if (senderToServerOutpot == -1)
    {
        printf("error! senderToServer() \n");
        return -1;
    }

    fclose(pfile);
    // close(ClentSocket);

    return 0;
}

int senderToServer(int ClentSocket, FILE *pfile)
{
    char msg[MSG_SIZE];
    memset(&msg, 0, sizeof(msg));
    int count = 0;

    for (size_t i = 0; i < 8; i++)
    {
        int freadOutput = fread(msg, 1, sizeof(msg), pfile);
        if (freadOutput < 0)
        {
            perror("error! fread() failed!\n");
            exit(1);
        }

        int bytesSent = send(ClentSocket, msg, sizeof(msg), 0);
        if (bytesSent == -1)
        {
            perror("error! send() failed!\n");
            exit(1);
        }
        else
        {
            count++;
        }
    }

    printf("The file was sent in %d messages \n", count);

    if (ferror(pfile))
    {
        perror("ereor! file failed!\n");
    }

    // Receive data from server
    char bufferReply[BUFFER_SIZE] = {'\0'};
    char authentication[] = "0011010001110100\0";

    int bytesReceived = recv(ClentSocket, bufferReply, BUFFER_SIZE, 0);
    if (bytesReceived == -1)
    {
        printf("recv() failed : %d \n", errno);
        return -1;
    }
    else if (bytesReceived == 0)
    {
        printf("peer has closed the TCP connection prior to recv().\n");
        return -1;
    }
    if (0 == strcmp(bufferReply, authentication))
    {
        printf("Authentication succeeded!! %s\n", authentication);
    }
    else
    {
        printf("Authentication failed!!\n");
    }

    return 0;
}
