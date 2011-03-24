#ifndef _SERIAL_H__
#define _SERIAL_H__

#include "main.h"

char receivedChar;
bit serialReceived; // set to '1' when a byte is received
void print (const char *);
char rxChar(void);
void putch(char);

#endif