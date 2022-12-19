#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <signal.h>

#define PORT 2020
#define BUFFER_SIZE 8192

void resvAndsend(char *msg, int new_socket);
int questionsContinued(int new_socket);

int main()
{
    int switchToWhile = 1, server_socket, new_socket;
    struct sockaddr_in serverAddress;
    char CC[256];
    socklen_t CClen;
    memset(&serverAddress, 0, sizeof(serverAddress));
    int serverAddressLen = sizeof(serverAddress);
    char buffer[BUFFER_SIZE] = {0};
    char *authentication = "0011010001110100";

    // Creating socket file descriptor
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    int yes = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &yes, sizeof(yes)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (bind(server_socket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_socket, 1) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    printf("Receiver listen, ip: 0.0.0.0 port: 2020 \n");

    if ((new_socket = accept(server_socket, (struct sockaddr *)&serverAddress, (socklen_t *)&serverAddressLen)) < 0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    while (switchToWhile)
    {

        printf("\n\nnew loop \n" );
        printf("------------------------- SET TO CUBIC -------------------------\n");

        strcpy(CC, "cubic");
        CClen = strlen(CC);

        if (setsockopt(server_socket, IPPROTO_TCP, TCP_CONGESTION, CC, CClen) != 0)
        {
            perror("error! set sockopt to cubic failed!");
            return -1;
        }
        resvAndsend(authentication, new_socket);




        printf("\n------------------------- SET TO RENO ------------------------- \n");
        memset(&CC, 0, sizeof(CC));
        strcpy(CC, "reno");
        CClen = strlen(CC);
        if (setsockopt(server_socket, IPPROTO_TCP, TCP_CONGESTION, CC, CClen) != 0)
        {
            perror("ERROR! socket setting failed!\n");
            return -1;
        }

        resvAndsend(authentication, new_socket);

 
        switchToWhile= questionsContinued(new_socket);
        

    } 

    // closing the connected socket
    close(new_socket);
    // closing the listening socket
    shutdown(server_socket, SHUT_RDWR);
    return 0;
}








void resvAndsend(char *msg, int new_socket)
{
    int valrecv;
    char buffer[BUFFER_SIZE] = {0};
    printf("i am the server\n");
    if (valrecv = recv(new_socket, buffer, BUFFER_SIZE, 0) == -1)
    {
        printf("recv() in resvAndsend failed: %d \n", errno);
        close(new_socket);
        exit(1);
    }
    printf("client send to my: %s\n", buffer);

    if (send(new_socket, msg, strlen(msg), 0) == -1)
    {
        printf("send() in resvAndsend failed: %d\n", errno);
        close(new_socket);
        exit(1);
    }
    printf("i recv: %s \n", msg);
}




int questionsContinued(int new_socket){
    printf("\nWaiting for you or no..\n");
    int valrecv;
    char buffer[BUFFER_SIZE] = {0};
    if (valrecv = recv(new_socket, buffer, BUFFER_SIZE, 0) == -1)
    {
        printf("recv() in questionsContinued failed: %d \n", errno);
        close(new_socket);
        exit(1);
    }
    printf("buffer %s\n",buffer);

    if (!strcmp(buffer, "no"))
    {
        printf("\nclosing the connection\n");
        if (send(new_socket, "OK", strlen("OK"), 0) == -1)
        {
            printf("send() failed: %d\n", errno);
            close(new_socket);
            exit(1);
        }
        return 0;
    }
    if (!strcmp(buffer, "yes"))
    {
        printf("\nconnection\n");
        if (send(new_socket, "OK", strlen("OK"), 0) == -1)
        {
            printf("send() failed: %d\n", errno);
            close(new_socket);
            exit(1);
        }
        return 1;
    }
    else
    {
        printf("\nyes????no???  %s\n ", buffer);
        close(new_socket);
        exit(1);
    }

}
