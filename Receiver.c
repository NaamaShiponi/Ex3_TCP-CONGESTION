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

int resvAndsend(struct sockaddr_in clientAddress, int serverSocket, int clientSocket, double arrTime[], int *arrTimelen);
int connectionStatus(int clientSocket, int serverSocket);
void printTime(double renoTime[], int renoTimelen, double cubicTime[], int cubicTimelen);

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
    double *arrTimeCubic = (double *)malloc(sizeof(double) * 100);
    double *arrTimeReno = (double *)malloc(sizeof(double) * 100);
    int arrTimeCubiclen = 1;
    int arrTimeRenolen = 1;

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
    int exit = 1;
    char CC[256];
    socklen_t cclen;
    do
    {
        printf("\n------------------------- SET TO CUBIC ------------------------- \n");

        strcpy(CC, "cubic");
        cclen = strlen(CC);

        if (setsockopt(serverSocket, IPPROTO_TCP, TCP_CONGESTION, CC, cclen) != 0)
        {
            perror("error! set sockopt to cubic failed!");
            return -1;
        }
        int resvAndsendOutput = resvAndsend(clientAddress, serverSocket, clientSocket, arrTimeCubic, &arrTimeCubiclen);
        if (resvAndsendOutput == -1)
        {
            printf("resvAndsend() failed \n");
            close(serverSocket);
            return -1;
        }

        printf("\n------------------------- SET TO RENO ------------------------- \n");
        memset(&CC, 0, sizeof(CC));
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
        printf("set sockopt to reno is successfully\n");

        resvAndsendOutput = resvAndsend(clientAddress, serverSocket, clientSocket, arrTimeReno, &arrTimeRenolen);
        if (resvAndsendOutput == -1)
        {
            printf("error resvAndsend()\n");
            close(serverSocket);
            return -1;
        }

        exit = connectionStatus(clientSocket, serverSocket);

    } while (exit > 0);
    printTime(arrTimeReno, arrTimeRenolen, arrTimeCubic, arrTimeCubiclen);
    free(arrTimeCubic);
    free(arrTimeReno);
    close(clientSocket);
    // close(serverSocket);
    return 0;
}

int resvAndsend(struct sockaddr_in clientAddress, int serverSocket, int clientSocket, double arrTime[], int *arrTimelen)
{
    char authentication[] = "0011010001110100";
    time starting_time, ending_time;
    // int count = 0;
    int isFirst=1;
    int countHalfSizeFile = 0;
    int halfSizeFile;
    char *ptr;

    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    int bytesReceived;
    memset(&starting_time, 0, sizeof(starting_time));
    memset(&ending_time, 0, sizeof(ending_time));
    do
    {
        bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);

        if (bytesReceived == -1)
        {
            printf("recv() failed: %d \n", errno);
            close(serverSocket);
            close(clientSocket);
            return -1;
        }
        if(isFirst){
            isFirst=0;
            bytesReceived=0;
            halfSizeFile=strtol(buffer, &ptr,10);
            gettimeofday(&starting_time, NULL);

        }else{
            countHalfSizeFile+=bytesReceived;
            // count++;
        }
        
        
    }while (countHalfSizeFile < halfSizeFile);
    
    gettimeofday(&ending_time, NULL); // stop counting

    // printf("%d packages were received from the customer\n", count);

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

    double current_time = getAmountOfTime(starting_time, ending_time);
    int newSize=((sizeof(double)*(*arrTimelen))*2);
    // arrTime = (double *)realloc(arrTime, newSize);
    // if(arrTime==NULL) printf("error\n"); 
    arrTime[*arrTimelen] = current_time;
    *arrTimelen = *arrTimelen + 1;
    return 0;
}

int connectionStatus(int clientSocket, int serverSocket)
{

    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
    if (bytesReceived == -1)
    {
        printf("recv() failed: %d \n", errno);
        close(clientSocket);
        close(serverSocket);
        return -1;
    }
    if (0 == strcmp(buffer, "no"))
    {
        printf("\nclosing the connection\n");
        return 0;
    }
    return 1;
}

void printTime(double renoTime[], int renoTimelen, double cubicTime[], int cubicTimelen)
{

    printf("\n---------------- Times ----------------\n");
    double average = 0;
    printf("Times of the first half file (cubic TCP)\n");
    for (int i = 1; i < cubicTimelen; i++)
    {

        printf("message  %d : total time = %f\n", i, cubicTime[i]);
        average += cubicTime[i];
    }
    if (cubicTimelen == 1)
    {
        average;
    }
    else{
        average = average / (cubicTimelen - 1);
    }
    printf("average times (cubic TCP): %f\n", average);

    average = 0;
    printf("\nTimes of the second half file (reno TCP)\n");
    for (int i = 1; i < renoTimelen; i++)
    {
        printf("message  %d: total time= %f\n", i, renoTime[i]);
        average += renoTime[i];
    }
     if (renoTimelen == 1)
    {
        average;
    }
    else{
    average = average / (renoTimelen - 1);
    }

    printf("average times(reno TCP): %f\n", average);
}
