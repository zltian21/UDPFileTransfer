#define PKTSIZE 90 // for some unknow reason, not 88, but works fine.

/* Data Packet */
struct packet { 
     char data[81];     // 80 printable character and 1 null terminating character 
     short count;       // count of character
     short seq;         // packet sequence number
     char padding[3];   // packet padding
 };

/* ACK Packet */
 struct ack {
     short ack_num;     // ACK number
 };