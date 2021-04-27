/*
    Server.c 
    
*/
#include <stdio.h> /*for printf() and fprintf()*/
#include <sys/socket.h> /*for socket(), connect(), send(), and recv()*/
#include <arpa/inet.h> /*for sockaddr_in and inet_addr()*/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "packet.h"
#include "time.h"


void DieWithError(char *errorMessage);
void HandleClient(int servSocket, struct sockaddr_in clntAdd, unsigned int clnAddrLen, struct packet * filePkt, float lossRatio, int tOut);

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
    struct packet * pkt_buff;             // packet buffer

    printf("Initiating Server...\n");

    if (argc != 4) {
        fprintf(stderr, "Usage: %s <Server Port> <Time Out> <Packet Loss Ratio>\n", argv[0]) ;
		exit(1);
    }

    /* Assign values*/
    servPort = atoi(argv[1]);
    timeOut = atoi(argv[2]);
    pktLossRatio = atof(argv[3]);

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
        // TODO: receive from client
        if ((recvPktSize = recvfrom(sock, pkt_buff, PKTSIZE, 0, (struct sockaddr *) &clntAddr, &cliAddrLen)) < 0) {
            DieWithError("recvfrom() failed");
        }
        // printf("NMSL: %s\n", pkt_buff->data);
        HandleClient(sock, clntAddr, cliAddrLen, pkt_buff, pktLossRatio, timeOut);
        
    }
}
