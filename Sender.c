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
#define MSG_SIZE 550571
#define NAME_FILE "test.txt"
#define BUFFER_SIZE 8192

int senderToServer(char *msg, int sock);
int yesNoQuestions(int sock);
void sendToUserAnser(char *answer, int sock);

int main()
{
    int sock = 0, switchToWhile=1,clientConnect,freadOutput;
    char CC[256];
    socklen_t CClen;
    struct sockaddr_in serverAddress;
    char *hello[MSG_SIZE];
    //= "Hello from client";
    char *msg[MSG_SIZE]; 
    //= "i send msg 2";
    char firstHalf[MSG_SIZE]={0};
    char secondHalf[MSG_SIZE]={0};
    char buffer[BUFFER_SIZE] = {0};
    FILE *pfile;
    char *filename = NAME_FILE;
    pfile = fopen(filename, "r");

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


    freadOutput = fread(firstHalf, 1, sizeof(firstHalf), pfile);
    if (freadOutput < 0)
    {
        perror("error! fread() failed!\n");
        exit(1);
    }
    freadOutput = fread(secondHalf, 1, sizeof(secondHalf), pfile);
    if (freadOutput < 0)
    {
        perror("error! fread() failed!\n");
        exit(1);
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
        senderToServer(firstHalf, sock);

        printf("\n------------------------- SET TO RENO ------------------------- \n");
        memset(&CC, 0, sizeof(CC));
        strcpy(CC, "reno");
        CClen = strlen(CC);
        if (setsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, CC, CClen) != 0)
        {
            perror("ERROR! socket setting failed!\n");
            return -1;
        }

        senderToServer(secondHalf, sock);

        switchToWhile=yesNoQuestions(sock);
    }

    // closing the file
    fclose(pfile);
    // closing the connected socket
    close(clientConnect);
    return 0;
}







int senderToServer(char *msg, int sock)
{
    int valrecv,bytesSent;
    char buffer[BUFFER_SIZE] = {0};
    bytesSent = send(sock, msg, strlen(msg), 0);
    if (bytesSent == -1)
    {
        perror("error! send() failed!\n");
        exit(1);
    }
    printf("i send the file\n");

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
