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
#define HALF_FILE_SIZE 550571
#define NAME_FILE "test.txt"
#define BUFFER_SIZE 8192

void senderToServer(char *msg, int sock);
int yesNoQuestions(int sock);
void sendToUserAnser(char *answer, int sock);

int main()
{
    int sock = 0, switchToWhile = 1, clientConnect, freadOutput;
    // A variable to save the algorithm type
    char CC[256];
    socklen_t CClen;
    // serverAddress- server address and all his details
    struct sockaddr_in serverAddress;
    // A variables designed to save half of the file
    char firstHalf[HALF_FILE_SIZE] = {0};
    char secondHalf[HALF_FILE_SIZE] = {0};

    // Pointer to a file
    FILE *pfile;
    char *filename = NAME_FILE;
    pfile = fopen(filename, "r"); // opening the file

    // Creating socket file descriptor
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    // Enter the IP PORT and IP type into serverAddress
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // Convert IPv4 and IPv6 addresses from text to binary
    if (inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    // Makes a connection to the server
    if ((clientConnect = connect(sock, (struct sockaddr *)&serverAddress, sizeof(serverAddress))) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    // insert into the variables the half file corresponding to them
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

        printf("\n------------------------- SET TO RENO ------------------------- \n");
        strcpy(CC, "reno");
        CClen = strlen(CC);

        // Set sockopt to TCP reno algorithm
        if (setsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, CC, CClen) != 0)
        {
            perror("ERROR! socket setting failed!");
            return -1;
        }

        // Function responsible for sending the file to the server
        senderToServer(firstHalf, sock);

        printf("------------------------- SET TO CUBIC -------------------------\n");
        memset(&CC, 0, sizeof(CC));
        strcpy(CC, "cubic");
        CClen = strlen(CC);

        // Set sockopt to TCP cubic algorithm
        if (setsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, CC, CClen) != 0)
        {
            perror("ERROR! socket setting failed!\n");
            return -1;
        }
        // Function responsible for sending the file to the server
        senderToServer(secondHalf, sock);

        // function that is responsible for asking the user if he wants to send the file again.
        // if 1: repeat the loop
        // if 0: Exit the loop
        switchToWhile = yesNoQuestions(sock);
    }

    // closing the file
    fclose(pfile);
    // closing the connected socket
    close(clientConnect);
    return 1;
}

// Function responsible for sending the file to the server
void senderToServer(char *msg, int sock)
{
    int bytesSent;
    //XOR to an identity number
    char authentication[] = "0011010001110100\0";
    // bufferto enter the answer
    char buffer[BUFFER_SIZE] = {0};
    //send to server half first
    bytesSent = send(sock, msg, strlen(msg), 0);
    if (bytesSent == -1)
    {
        perror("error! send() failed!\n");
        exit(1);
    }
    printf("i send the file\n");
    //Get authentication from the server
    recv(sock, buffer, BUFFER_SIZE, 0);
    if (!strcmp(buffer, authentication))
    {
        printf("Authentication succeeded!! %s\n\n", authentication);
    }
    else
    {
        printf("Authentication failed!!\n");
    }
    
}


// function that is responsible for asking the user if he wants to send the file again.
// return 1: if  yes so repeat the loop
// return 0: if  no so exit the loop
int yesNoQuestions(int sock)
{

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

//A function that sends the server whether to continue receiving another file or to close the connection
void sendToUserAnser(char *answer, int sock)
{
    // bufferto enter the answer
    char buffer[BUFFER_SIZE] = {0};
    //Send to server yes or no
    int bytesSent = send(sock, answer, 5, 0);
    if (bytesSent == -1)
    {
        perror("error! send() in sendToUserAnser NO failed!\n");
        close(sock);
    }

    printf("sending to servet %s \n", answer);
    //Wait for a reply from the server that he received the message
    if (recv(sock, buffer, BUFFER_SIZE, 0) < 0)
    {
        perror("error! recv() in sendToUserAnser NO failed!\n");
        close(sock);
    }
}
