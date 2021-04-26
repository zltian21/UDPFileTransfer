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
    
    FILE *fp;                   // File to read
    char buff[BUFFERSIZE];      // Buffer to save read line
    short seq_num = 1;          // Sequence number
    struct packet pkt;          // Packet to send
    
    fp = fopen(filePkt->data, "r");
    if(fp == NULL) {
        DieWithError("fopen() Error");
    }
    printf("\n");

    //While it is not the end of the file: 
    //read a line to buffer, and send packets to client
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
        //TODO: Change seq after receive ACK
    }
    //Send EOT Packet
    memset(buff, 0, sizeof(buff));
    strcpy(pkt.data, buff);
    seq_num = alternateNum(seq_num);
    pkt.seq = htons(seq_num);
    pkt.count = strlen(buff);
    if (sendto(servSocket, &pkt, sizeof(pkt), 0, (struct sockaddr *) &clntAdd, clnAddrLen) != sizeof(pkt)) {
            DieWithError("sendto() EOT sent a different number of bytes than expected");
    }

    fclose(fp);

}
