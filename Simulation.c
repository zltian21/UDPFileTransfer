/*
    Simulation.c
    Contains utility functions
*/
#include <stdio.h> 
#include <stdlib.h>
#include <time.h>

float generateRandom();


//For Server
int SimulateLoss(float pktLossRatio) {
    return generateRandom() < pktLossRatio ? 1 : 0;
}

int SimulateACKLoss(float ACKLossRatio) {
    return generateRandom() < ACKLossRatio ? 1 : 0;
}

float generateRandom() {
    return ((float)rand())/RAND_MAX;
}

short alternateNum(short n) {
    return n == 0 ? 1 : 0;
}