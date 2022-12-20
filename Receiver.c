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

void resvAndsend(char *msg, int new_socket, double arrTime[], int *arrTimelen);
int questionsContinued(int new_socket);
void printTime(double renoTime[], int renoTimelen, double cubicTime[], int cubicTimelen);
double getTime(struct timeval starting_time, struct timeval ending_time);



int main()
{
    //Arrays to save the times for each of the algorithms
    double *arrTimeCubic = (double *)malloc(sizeof(double) * 100);
    double *arrTimeReno = (double *)malloc(sizeof(double) * 100);
    int arrTimeCubiclen = 1, arrTimeRenolen = 1;
    //Switch to continue receiving the file again
    int switchToWhile = 1;
    //XOR to an identity number
    char *authentication = "0011010001110100";
    //server_socket-socket connection, new_socket- connecting a socket with a client
    int server_socket, new_socket;
    //serverAddress- server address and all his details
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    int serverAddressLen = sizeof(serverAddress);
    //A variable to save the algorithm type
    char CC[256];
    socklen_t CClen;

    
    // Creating socket file descriptor
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 2020
    int yes = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &yes, sizeof(yes)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    //Enter the IP PORT and IP type into serverAddress
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 2020
    if (bind(server_socket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    //Open listening every time for 1 connection
    if (listen(server_socket, 1) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    printf("Receiver listen, ip: 0.0.0.0 port: 2020 \n");

    //Opening the socket connection and waiting for a client connection
    if ((new_socket = accept(server_socket, (struct sockaddr *)&serverAddress, (socklen_t *)&serverAddressLen)) < 0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }


    while (switchToWhile)
    {
        printf("\n\nnew loop \n");
        printf("------------------------- SET TO RENO ------------------------- \n");
        strcpy(CC, "reno");
        CClen = strlen(CC);

        //Set sockopt to TCP reno algorithm
        if (setsockopt(server_socket, IPPROTO_TCP, TCP_CONGESTION, CC, CClen) != 0)
        {
            perror("error! set sockopt to RENO failed!");
            return -1;
        }
        //Function to receive file and send verification
        resvAndsend(authentication, new_socket, arrTimeReno, &arrTimeRenolen);

        printf("------------------------- SET TO CUBIC -------------------------\n");
        memset(&CC, 0, sizeof(CC));
        strcpy(CC, "cubic");
        CClen = strlen(CC);

        //Set sockopt to TCP cubic algorithm
        if (setsockopt(server_socket, IPPROTO_TCP, TCP_CONGESTION, CC, CClen) != 0)
        {
            perror("ERROR! socket setting failed!\n");
            return -1;
        }
        //Function to receive file and send verification
        resvAndsend(authentication, new_socket, arrTimeCubic, &arrTimeCubiclen);

        //A function waits to receive a message from the client whether to accept the file again or to close the socket.
        //If 1- repeat the loop
        //If 0 - exit the loop
        switchToWhile = questionsContinued(new_socket);
    }
    //Function to print the times we saved
    printTime(arrTimeReno, arrTimeRenolen, arrTimeCubic, arrTimeCubiclen);

    //free the arrays of the times
    free(arrTimeCubic);
    free(arrTimeReno);

    // closing the connected socket
    close(new_socket);
    // closing the listening socket
    shutdown(server_socket, SHUT_RDWR);
    return 1;
}


//Function to receive file and send verification
void resvAndsend(char *msg, int new_socket, double arrTime[], int *arrTimelen)
{
    //Variables to save the start time and end time of the file
    struct timeval starting_time, ending_time;
    memset(&starting_time, 0, sizeof(starting_time));
    memset(&ending_time, 0, sizeof(ending_time));

    //Sum up the size of the received files
    int countHalfSizeFile = 0;

    int valrecv;

    //bufferto enter the answer
    char buffer[BUFFER_SIZE] = {0};

    printf("I'm waiting for the file\n");

    //takes the start time
    gettimeofday(&starting_time, NULL);

    //The loop that gets the half file. 
    //The loop will continue until you get packets whose size sum equals the size of half the file (or until there is an error)
    while ((valrecv = recv(new_socket, buffer, BUFFER_SIZE, 0)) > 0)
    {
        countHalfSizeFile += valrecv;
        if (countHalfSizeFile >= HALF_FILE_SIZE)
        {
            break;
        }
    }
    //takes the end time
    gettimeofday(&ending_time, NULL); 

    if (valrecv == -1)
    {
        printf("recv() in resvAndsend failed: %d \n", errno);
        close(new_socket);
        exit(1);
    }

    //Function that return how long it took in total to send
    double current_time = getTime(starting_time, ending_time);
    //Insert the time into the array
    arrTime[*arrTimelen] = current_time;
    //Increase the number of members in the array
    *arrTimelen = *arrTimelen + 1;

    

    printf("client send file\n");

    //send back to the customer the verification
    if (send(new_socket, msg, strlen(msg), 0) == -1)
    {
        printf("send() in resvAndsend failed: %d\n", errno);
        close(new_socket);
        exit(1);
    }
    printf("i send: %s \n", msg);
}


//A function waits to receive a message from the client whether to accept the file again or to close the socket.
//return 1- repeat the loop
//return 0 - exit the loop
int questionsContinued(int new_socket)
{
    printf("\nWaiting for you or no..\n");

    //bufferto enter the answer
    char buffer[BUFFER_SIZE] = {0};

    //Waiting for the recv
    if (recv(new_socket, buffer, BUFFER_SIZE, 0)== -1)
    {
        printf("recv() in questionsContinued failed: %d \n", errno);
        close(new_socket);
        exit(1);
    }
    printf("buffer %s\n", buffer);

    //if buffer equal to no send OK to client and return 0 
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
    //if buffer equal to yes send OK to client and return 1
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
    //if is not equal to yes and no, something went wrong 
    else
    {
        printf("\nyes????no???  %s\n ", buffer);
        close(new_socket);
        exit(1);
    }
}


//Function to print the times we saved
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
    if (cubicTimelen != 1)
    {
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
    if (renoTimelen != 1)
    {
        average = average / (renoTimelen - 1);
    }

    printf("average times(reno TCP): %f\n", average);
}


//Function that return how long it took in total to send
double getTime(struct timeval starting_time, struct timeval ending_time)
{
    return((ending_time.tv_sec * 1000000 + ending_time.tv_usec)-(starting_time.tv_sec * 1000000 + starting_time.tv_usec));
}
