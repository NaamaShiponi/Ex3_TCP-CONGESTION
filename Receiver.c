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
#define HALF_FILE_SIZE 550571

void resvAndsend(char *msg, int new_socket,double arrTime[], int *arrTimelen);
int questionsContinued(int new_socket);
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
    int arrTimeCubiclen = 1,arrTimeRenolen = 1;
    
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
        resvAndsend(authentication, new_socket,arrTimeCubic, &arrTimeCubiclen);




        printf("\n------------------------- SET TO RENO ------------------------- \n");
        memset(&CC, 0, sizeof(CC));
        strcpy(CC, "reno");
        CClen = strlen(CC);
        if (setsockopt(server_socket, IPPROTO_TCP, TCP_CONGESTION, CC, CClen) != 0)
        {
            perror("ERROR! socket setting failed!\n");
            return -1;
        }

        resvAndsend(authentication, new_socket,arrTimeReno, &arrTimeRenolen);

 
        switchToWhile= questionsContinued(new_socket);
        

    } 
    printTime(arrTimeReno, arrTimeRenolen, arrTimeCubic, arrTimeCubiclen);
    free(arrTimeCubic);
    free(arrTimeReno);
    // closing the connected socket
    close(new_socket);
    // closing the listening socket
    shutdown(server_socket, SHUT_RDWR);
    return 0;
}




void resvAndsend(char *msg, int new_socket,double arrTime[], int *arrTimelen)
{
    time starting_time, ending_time;
    memset(&starting_time, 0, sizeof(starting_time));
    memset(&ending_time, 0, sizeof(ending_time));

    int countHalfSizeFile=0;
    int valrecv=0;
    char buffer[BUFFER_SIZE] = {0};
    printf("I'm waiting for the file\n");
    gettimeofday(&starting_time, NULL);
    // for (size_t i = 0; i < 67; i++)
    // {
    //     valrecv=recv(new_socket, buffer, BUFFER_SIZE, 0);
    //     printf("valrecv %d",valrecv);
    //     if(valrecv==-1){
    //     printf("recv() in resvAndsend failed: %d \n", errno);
    //     close(new_socket);
    //     exit(1);
    //     }
    // }
    double current_time = getAmountOfTime(starting_time, ending_time);
    arrTime[*arrTimelen] = current_time;
    *arrTimelen = *arrTimelen + 1;


    int count=0;
    while((valrecv=recv(new_socket, buffer, BUFFER_SIZE, 0)) > 0 )
    {
        count++;
        countHalfSizeFile+=valrecv;
        if(countHalfSizeFile>=HALF_FILE_SIZE){
            break;
        }
    }
    gettimeofday(&ending_time, NULL); // stop counting


    if(valrecv==-1){
        printf("recv() in resvAndsend failed: %d \n", errno);
        close(new_socket);
        exit(1);
    }
   
    
    printf("client send file\n");

    if (send(new_socket, msg, strlen(msg), 0) == -1)
    {
        printf("send() in resvAndsend failed: %d\n", errno);
        close(new_socket);
        exit(1);
    }
    printf("i send: %s \n", msg);
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
