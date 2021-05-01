/*
    Server.c 
    
*/
#include <stdio.h> /*for printf() and fprintf()*/
#include <sys/socket.h> /*for socket(), connect(), send(), and recv()*/
#include <arpa/inet.h> /*for sockaddr_in and inet_addr()*/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include "packet.h"


void DieWithError(char *errorMessage);
void HandleClient(int servSocket, struct sockaddr_in clntAdd, unsigned int clnAddrLen, struct packet filePkt, float lossRatio);

int main(int argc, char *argv[]) {
    srand((unsigned int)time(NULL));
    
    int sock;                           // Socket
    struct sockaddr_in servAddr;        // Server address
    struct sockaddr_in clntAddr;        // Client address
    unsigned int cliAddrLen;            // Length of incoming message
    unsigned short servPort;            // Server port
    int timeOut;                        // Time our
    float pktLossRatio;                 // Packet Loss Ratio
    int recvPktSize;                    // Size of received packet
    struct packet pkt_buff;             // packet buffer
    struct timeval tv;                      // Time interval
    long micro_t_out;      // Time in microseconds
    time_t sec_t_out;
    printf("Initiating Server...\n");

    if (argc != 4) {
        fprintf(stderr, "Usage: %s <Server Port> <Time Out> <Packet Loss Ratio>\n", argv[0]) ;
		exit(1);
    }

    /* Assign values*/
    servPort = atoi(argv[1]);
    timeOut = pow(10, atoi(argv[2]));
    pktLossRatio = atof(argv[3]);
    
    sec_t_out = timeOut/1000000;
    micro_t_out = timeOut - sec_t_out*1000000;
    
    tv.tv_sec = sec_t_out;
    tv.tv_usec = micro_t_out; 

    /* Create socket for incoming connections */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    /* Construct local address structure*/
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(servPort);

    /* Bind the local address*/
    if (bind(sock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
        DieWithError(" bind() failed");
    
    /* Run forever*/
    for (;;) {
        /* Set the size of the in-out parameter */
        cliAddrLen = sizeof(clntAddr);

        if ((recvPktSize = recvfrom(sock, &pkt_buff, PKTSIZE, 0, (struct sockaddr *) &clntAddr, &cliAddrLen)) < 0) {
            printf("HERE: %d\n", recvPktSize);
            DieWithError("filename recvfrom() failed");
        }

        if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
            perror("setsockopt() Error");
        }
        // printf("NMSL: %s\n", pkt_buff->data);
        HandleClient(sock, clntAddr, cliAddrLen, pkt_buff, pktLossRatio);
        close(sock);
	    exit(0);
    }
}
