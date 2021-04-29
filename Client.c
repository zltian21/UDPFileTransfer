/*
    Client.c 

*/
#include <stdio.h> /*for printf() and fprintf()*/
#include <sys/socket.h> /*for socket(), connect(), send(), and recv()*/
#include <arpa/inet.h> /*for sockaddr_in and inet_addr()*/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "packet.h"

void DieWithError(char *errorMessage);
short alternateNum(short n);
int SimulateACKLoss(float ACKLossRatio);


int main(int argc, char *argv[]) {
    srand((unsigned int)time(NULL));

    int sock;                           //Socket
    struct sockaddr_in servAddr;        // Server address
    struct sockaddr_in fromAddr;        // Source address
    unsigned short servPort;            // Port
    unsigned int fromSize;             // From size
    char *servIP;                       // Server IP addr
    char *filename;                     // Filename
    float ACKLossRatio;                 // ACK loss ratio
    struct packet pkt;                  // packet


    if (argc != 5) {
        fprintf(stderr, "Usage: %s <Server IP> <Port Number> <Filename> <ACK Loss Ratio>\n", argv[0]);
		exit(1);
    }
    
    /*Assign Value*/
    servIP = argv[1];
    servPort = atoi(argv[2]);
    filename = argv[3];
    ACKLossRatio = atof(argv[4]);
    strcpy(pkt.data, filename);
    pkt.seq = htons(0);
    pkt.count = htons(strlen(pkt.data));

    /* Create a datagram/UDP socket */
    if((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP))<0)
        DieWithError("socket() failed");
    
    /*Construct the server address structure*/
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr(servIP);
    servAddr.sin_port = htons(servPort);

    /*send filename to the server*/
    if (sendto(sock, &pkt, sizeof(pkt), 0, (struct sockaddr *) &servAddr, sizeof(servAddr)) != sizeof(pkt)) {
        DieWithError("send() sent a different number of bytes than expected");
    }

    /* Attributes for receiving packet */
    struct packet pkt_buff;             // Packet buffer for receiving
    int bytesRcvd;                      // Bytes actually receivied 
    short tempCount;                    // tempoary Count(data characters)
    short tempSeq;                      // tempoary packet sequence number
    FILE *fp;                           // File to write
    struct ack ack_send;                // ACK
    short tempACK;                      // temp ACK
    short expc_seq = 1;                 // Expected sequence number

    /* Attributes for printing summary */
    int total_pkt_received = 0;         // Total packets
    int dupli_pkt_received = 0;         // Number of duplicate packtes 
    int total_bytes = 0;                // Total bytes received
    int ack_success = 0;                // Number of ACK not loss
    int ack_loss = 0;                   // Number of ACK loss
    
    fp = fopen("./out.txt", "w");
    printf("\n");

    /*Receive packets from server*/
    fromSize = sizeof(fromAddr);
    do {
        if((bytesRcvd = recvfrom(sock, &pkt_buff, PKTSIZE, 0, (struct sockaddr *) &fromAddr, &fromSize)) != PKTSIZE) {
            DieWithError("recvfrom() failed");
        }

        tempSeq = ntohs(pkt_buff.seq);
        tempCount = ntohs(pkt_buff.count);
        
        // Write file
        if (tempCount > 0 && tempSeq == expc_seq) {
            fprintf(fp, "%s", pkt_buff.data);
            expc_seq = alternateNum(expc_seq);
            total_bytes += tempCount;
            printf("Packet %d received with %d data bytes\n", tempSeq, tempCount);
            printf("    Packet %d delivered to user\n", tempSeq);
        } else if (tempCount > 0) {
            dupli_pkt_received++;
            printf("Duplicate packet %d received with %d data bytes\n", tempSeq, tempCount);
        }
        tempACK = alternateNum(tempSeq);
        ack_send.ack_num = htons(tempACK);
        
        // Send ACK
        if (tempCount > 0) {
            total_pkt_received++;
            printf("    ACK %d generated for transmission\n", tempACK);
            if (SimulateACKLoss(ACKLossRatio) == 0) {
                if (sendto(sock, &ack_send, sizeof(ack_send), 0, (struct sockaddr *) &servAddr, sizeof(servAddr)) != sizeof(ack_send)) {
                    DieWithError("send() ACK sent a different number of bytes than expected");
                }
                ack_success++;
                printf("    ACK %d successfully transmitted\n", tempACK);
            } else {
                ack_loss++;
                printf("    ACK %d lost\n", tempACK);
            }
        } else {
            printf("End of Transmission Packet with sequence number %d received\n", tempSeq);
        }

        printf("\n");
    } while (tempCount > 0);
    fclose(fp);
	close(sock);

    /* Printing Summary */
    printf("Server Summary\n");
    printf("=====================================================================\n");
    printf("Total number of data packets received successfully: %d\n", total_pkt_received);
    printf("Number of duplicate data packets received: %d\n", dupli_pkt_received);
    printf("Number of data packets received successfully, not including duplicates: %d\n", (total_pkt_received - dupli_pkt_received));
    printf("Total number of data bytes received which are delivered to user: %d\n", total_bytes);
    printf("Number of ACKs transmitted without loss: %d\n", ack_success);
    printf("Number of ACKs generated but dropped due to loss: %d\n", ack_loss);
    printf("Total number of ACKs generated: %d\n\n", (ack_success + ack_loss));
    
	exit(0);
}