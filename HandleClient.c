/*
    HandleTCPClient.c
    This file contains code that reads and sends local file to the client.
*/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "packet.h"
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>


#define BUFFERSIZE 81 //80 printable character and 1 null terminating character 

void DieWithError(char *errorMessage);
int SimulateLoss(float pktLossRatio);
short alternateNum(short n);

void HandleClient(int servSocket, struct sockaddr_in clntAdd, unsigned int clnAddrLen, struct packet * filePkt, float lossRatio) {
    
    FILE *fp;                               // File to read
    char buff[BUFFERSIZE];                  // Buffer to save read line
    short seq_num = 1;                      // Sequence number
    struct packet pkt;                      // Packet to send
    struct ack ack_recv;                    // ACK received
    int recvACKSize;                        // ACK received size
    
    fp = fopen(filePkt->data, "r");
    if(fp == NULL) {
        DieWithError("fopen() Error");
    }
    printf("\n");

    /* Send file packet */
    while (fgets(buff, BUFFERSIZE, fp) != NULL) {
        pkt.seq = htons(seq_num);
        pkt.count = htons(strlen(buff));
        strcpy(pkt.data, buff);

        // Send Packet
        if (SimulateLoss(lossRatio) == 0) {
            if (sendto(servSocket, &pkt, sizeof(pkt), 0, (struct sockaddr *) &clntAdd, clnAddrLen) != sizeof(pkt)) {
                DieWithError("sendto() sent a different number of bytes than expected");
            }
        }

        // Waiting for ACK
        while((recvACKSize = recvfrom(servSocket, &ack_recv, sizeof(ack_recv), 0, (struct sockaddr *) &clntAdd, &clnAddrLen)) < 0) {
            printf("Timout reached. Resending segment %d\n", seq_num);
            if (SimulateLoss(lossRatio) == 0) {
                if (sendto(servSocket, &pkt, sizeof(pkt), 0, (struct sockaddr *) &clntAdd, clnAddrLen) != sizeof(pkt)) {
                    DieWithError("sendto() sent a different number of bytes than expected");
                }
            }
        }
        
        // Alternate Sequence #
        seq_num = alternateNum(seq_num); 
    }

    /* Send EOT Packet */
    memset(buff, 0, sizeof(buff));
    strcpy(pkt.data, buff);
    pkt.seq = htons(seq_num);
    pkt.count = strlen(buff);
    if (sendto(servSocket, &pkt, sizeof(pkt), 0, (struct sockaddr *) &clntAdd, clnAddrLen) != sizeof(pkt)) {
            DieWithError("sendto() EOT sent a different number of bytes than expected");
    }

    fclose(fp);
}
