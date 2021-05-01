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

void DieWithError(char *errorMessage);          // Report Error
int SimulateLoss(float pktLossRatio);           // Data packet loss simulation
short alternateNum(short n);                    // Alternate number between 0 and 1: 0 -> 1; 1 -> 0

void HandleClient(int servSocket, struct sockaddr_in clntAdd, unsigned int clnAddrLen, struct packet filePkt, float lossRatio) {
    
    /* Attrubutes for sending data packets and receiving ACK packets */
    FILE *fp;                               // File to read
    char buff[BUFFERSIZE];                  // Buffer to save read line
    short seq_num = 1;                      // Sequence number
    short data_size;                        // Size of data in a packet
    struct packet pkt;                      // Packet to send
    struct ack ack_recv;                    // ACK received
    int recvACKSize;                        // ACK received size
    
    /* Attributes for printing Summary */
    int num_of_pkt = 0;                     // number of data packets (initial only)
    int total_num_pkt = 0;                  // total number of data packets (include initial and generated)
    int total_pkt_bytes = 0;                // total bytes of data packets (initial only)
    int packet_loss = 0;                    // number of packet loss
    int packet_success = 0;                 // number of packet transmitted successfully
    int ack_received = 0;                   // number of ACK received
    int count_timeout = 0;                  // timeout times
    
    /* Open file to read */
    fp = fopen(filePkt.data, "r");
    if(fp == NULL) {
        DieWithError("fopen() Error");
    }
    printf("\n");

    /* Read Line, Send data packets, Receive ACK */
    while (fgets(buff, BUFFERSIZE, fp) != NULL) {
        // Set Data Packet
        data_size = strlen(buff);
        pkt.seq = htons(seq_num);
        pkt.count = htons(data_size);
        strcpy(pkt.data, buff);

        // Send Packet
        printf("Packet %d generated for transmission with %d data bytes\n", seq_num, data_size);
        num_of_pkt++;
        total_num_pkt++;
        total_pkt_bytes += data_size;
        if (SimulateLoss(lossRatio) == 0) {
            if (sendto(servSocket, &pkt, sizeof(pkt), 0, (struct sockaddr *) &clntAdd, clnAddrLen) != sizeof(pkt)) {
                DieWithError("sendto() sent a different number of bytes than expected");
            }
            printf("    Packet %d successfully transmitted with %d data bytes\n", seq_num, data_size);
            packet_success++;
        } else {
            packet_loss++;
            printf("    Packet %d lost\n", seq_num);
        }

        // Waiting for ACK
        while((recvACKSize = recvfrom(servSocket, &ack_recv, sizeof(ack_recv), 0, (struct sockaddr *) &clntAdd, &clnAddrLen)) < 0) { // wait for timeout
            printf("    Timeout expired for packet numbered %d\n", seq_num);
            count_timeout++;
            printf("Packet %d generated for re-transmission with %d data bytes\n", seq_num, data_size);
            total_num_pkt++;
            if (SimulateLoss(lossRatio) == 0) {// packet send successfully
                if (sendto(servSocket, &pkt, sizeof(pkt), 0, (struct sockaddr *) &clntAdd, clnAddrLen) != sizeof(pkt)) {
                    DieWithError("sendto() sent a different number of bytes than expected");
                }
                printf("    Packet %d successfully transmitted with %d data bytes\n", seq_num, data_size);
                packet_success++;
            } else {    // packet loss
                packet_loss++;
                printf("    Packet %d lost\n", seq_num);
            }
        }
        ack_received++;
        printf("    ACK %d received\n", ntohs(ack_recv.ack_num));

        
        // Alternate Sequence #
        seq_num = alternateNum(seq_num); 
        printf("\n");
    }

    /* Send EOT Packet */
    memset(buff, 0, sizeof(buff));
    strcpy(pkt.data, buff);
    pkt.seq = htons(seq_num);
    pkt.count = strlen(buff);
    if (sendto(servSocket, &pkt, sizeof(pkt), 0, (struct sockaddr *) &clntAdd, clnAddrLen) != sizeof(pkt)) {
            DieWithError("sendto() EOT sent a different number of bytes than expected");
    }
    printf("End of Transmission Packet with sequence number %d transmitted\n\n\n", seq_num);
    fclose(fp);


    /* Printing Summary */
    printf("Server Summary\n");
    printf("=====================================================================\n");
    printf("Number of data packets generated for transmission (initial only): %d\n", num_of_pkt);
    printf("Total number of data bytes generated for transmission (initial only): %d\n", total_pkt_bytes);
    printf("Total number of data packets generated for retransmission: %d\n", total_num_pkt);
    printf("Number of data packets dropped due to loss: %d\n", packet_loss);
    printf("Number of data packets transmitted successfully: %d\n", packet_success);
    printf("Number of ACKs received: %d\n", ack_received);
    printf("Count of how many times timeout expired: %d\n\n", count_timeout);

    // Question: EOT also loss?
    
}
