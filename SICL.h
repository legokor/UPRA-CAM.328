#ifndef SICL_H
#define SICL_H

#define DEBUG
#define DEBUG2

//UPRA BUS
#define SICL_BAUD 9600
#define SICLRX    0
#define SICLTX    1
#define BUSBUSY   A0

#define CAM_PAYLOAD_INDEX 1

String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

int msg_idx;
char msg[80];


#endif
