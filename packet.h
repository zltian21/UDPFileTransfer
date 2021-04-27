#define PKTSIZE 90 // for some unknow reason, not 88

struct packet { 
     char data[81]; //80 printable character and 1 null terminating character 
     short count;
     short seq;
     char padding[3];
 };

 struct ack {
     short ack_num;
 };