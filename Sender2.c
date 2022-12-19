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
#include <arpa/inet.h>

#define PORT 2020
#define MSG_SIZE 540672
#define NAME_FILE "test.txt"
#define BUFFER_SIZE 8192

int senderToServer(char *msg, int sock);
int yesNoQuestions(int sock);
void sendToUserAnser(char *answer, int sock);

int main()
{
    int sock = 0, switchToWhile=1,clientConnect;
    char CC[256];
    socklen_t CClen;
    struct sockaddr_in serverAddress;
    char *hello = "Hello from client";
    char *msg = "i send msg 2";
    char buffer[BUFFER_SIZE] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // Convert IPv4 and IPv6 addresses from text to binary
    // form
    if (inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if ((clientConnect = connect(sock, (struct sockaddr *)&serverAddress, sizeof(serverAddress))) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    while (switchToWhile)
    {
        printf("\n\nnew loop \n");
        printf("------------------------- SET TO CUBIC -------------------------\n");

        strcpy(CC, "cubic");
        CClen = strlen(CC);

        if (setsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, CC, CClen) != 0)
        {
            perror("ERROR! socket setting failed!");
            return -1;
        }
        senderToServer(hello, sock);

        printf("\n------------------------- SET TO RENO ------------------------- \n");
        memset(&CC, 0, sizeof(CC));
        strcpy(CC, "reno");
        CClen = strlen(CC);
        if (setsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, CC, CClen) != 0)
        {
            perror("ERROR! socket setting failed!\n");
            return -1;
        }

        senderToServer(msg, sock);

        switchToWhile=yesNoQuestions(sock);
    }
    // closing the connected socket
    close(clientConnect);
    return 0;
}







int senderToServer(char *msg, int sock)
{
    int valrecv,bytesSent;
    char buffer[BUFFER_SIZE] = {0};
    printf("i am the claint\n");
    bytesSent = send(sock, msg, strlen(msg), 0);
    if (bytesSent == -1)
    {
        perror("error! send() failed!\n");
        exit(1);
    }
    printf("i send %s\n", msg);

    valrecv = recv(sock, buffer, BUFFER_SIZE, 0);
    printf("server resv to my: %s\n", buffer);
}


int yesNoQuestions(int sock){
    char answer[5];
    while (1)
    {
        printf("\n\nSend the file again? yes/no\n");
        scanf("%s", answer);
        if (!strcmp(answer, "yes"))
        {
            sendToUserAnser(answer, sock);
            printf("\n");

            return 1;
        }
        else if (!strcmp(answer, "no"))
        {
            sendToUserAnser(answer, sock);
            return 0;
        }
    }
}

void sendToUserAnser(char *answer, int sock)
{
    char buffer[BUFFER_SIZE] = {0};
    int bytesSent = send(sock, answer, 5, 0);
    if (bytesSent == -1)
    {
        perror("error! send() in sendToUserAnser NO failed!\n");
        close(sock);
    }

    printf("sending to servet %s \n",answer);
    if (recv(sock, buffer, BUFFER_SIZE, 0) < 0)
    {
        perror("error! recv() in sendToUserAnser NO failed!\n");
        close(sock);
    }

}