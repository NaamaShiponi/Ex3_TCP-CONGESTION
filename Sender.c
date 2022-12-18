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

int AskTheUser(int ClentSocket);
int senderToServer(int ClentSocket, FILE *pfile, int halfSizeFile);
int sendToUserAnser(char answer[], int ClentSocket);

int main()
{

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

    int exit = 1;
    char CC[256];
    socklen_t cclen;
    do
    {
        FILE *pfile;
        char *filename = NAME_FILE;
        pfile = fopen(filename, "r");
        if (!pfile)
        {
            printf("ERROR! file opening has failed!\n");
        }
        fseek(pfile, 0L, SEEK_END);
        int firstHalfFile = MSG_SIZE * ((ftell(pfile) / MSG_SIZE / 2));
        int secondHalfFile = ftell(pfile) - firstHalfFile;
        fseek(pfile, 0L, SEEK_SET);

        printf("\n------------------------- SET TO CUBIC -------------------------\n");

        strcpy(CC, "cubic");
        cclen = strlen(CC);

        if (setsockopt(ClentSocket, IPPROTO_TCP, TCP_CONGESTION, CC, cclen) != 0)
        {
            perror("ERROR! socket setting failed!");
            return -1;
        }
        int senderToServerOutpot = senderToServer(ClentSocket, pfile, firstHalfFile);

        if (senderToServerOutpot == -1)
        {
            printf("error! senderToServer \n");
        }

        printf("\n------------------------- SET TO RENO -------------------------\n");

        memset(&CC, 0, sizeof(CC));
        strcpy(CC, "reno");
        cclen = strlen(CC);

        int setsockoptOutput = setsockopt(ClentSocket, IPPROTO_TCP, TCP_CONGESTION, CC, cclen);

        if (setsockoptOutput != 0)
        {
            perror("error! setsockopt() failed!\n");
            return -1;
        }
        printf("set sockopt to reno is successfully\n");

        senderToServerOutpot = senderToServer(ClentSocket, pfile, secondHalfFile);

        if (senderToServerOutpot == -1)
        {
            printf("error! senderToServer() \n");
            return -1;
        }
        fclose(pfile);
        exit = AskTheUser(ClentSocket);

    } while (exit > 0);

    close(ClentSocket);
}

int senderToServer(int ClentSocket, FILE *pfile, int halfSizeFile)
{
    int countHalfSizeFile = 0;

    char msg[MSG_SIZE];
    memset(&msg, 0, sizeof(msg));
    int count = 0;

    sprintf(msg, "%d", halfSizeFile);
    int bytesSent = send(ClentSocket, msg, sizeof(msg), 0);
    if (bytesSent == -1)
    {
        perror("error! send() failed!\n");
        exit(1);
    }

    while (countHalfSizeFile < halfSizeFile)
    {
        int freadOutput = fread(msg, 1, sizeof(msg), pfile);
        if (freadOutput < 0)
        {
            perror("error! fread() failed!\n");
            exit(1);
        }

        bytesSent = send(ClentSocket, msg, sizeof(msg), 0);
        if (bytesSent == -1)
        {
            perror("error! send() failed!\n");
            exit(1);
        }
        countHalfSizeFile += freadOutput;
        count++;
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
        printf("Authentication succeeded!! %s\n\n", authentication);
    }
    else
    {
        printf("Authentication failed!!\n");
    }

    return 0;
}

int AskTheUser(int ClentSocket)
{

    char answer[5];
    while (1)
    {
        printf("Send the file again? yes/no\n");
        scanf("%s", answer);
        if (0 == strcmp(answer, "yes"))
        {
            sendToUserAnser(answer, ClentSocket);
            printf("\n");

            return 1;
        }
        else if (0 == strcmp(answer, "no"))
        {
            sendToUserAnser(answer, ClentSocket);
            return 0;
        }
    }
}

int sendToUserAnser(char answer[], int ClentSocket)
{
    int bytesSent = send(ClentSocket, answer, 5, 0);
    if (bytesSent == -1)
    {
        perror("error! send() NO failed!\n");
        close(ClentSocket);
        return -1;
    }
    return 1;
}
