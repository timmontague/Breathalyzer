#include "serial.h"

/**
	putch waits until the serial transmit line is clear, then
	sends a single byte over serial.
*/
void putch(char c)
{
	while(!TXIF); // wait until previous character is done sending
	TXREG = c;
}

/**
	print transmits a string over serial
*/
void print (const char *s)
{
	while(*s)
		putch((char)*s++);
}

/**
	Waits for a character to be inputed via serial and returns it.
*/
char rxChar(void)
{
	// wait for input
	while(!serialReceived);
	serialReceived = 0;
	return receivedChar;
}
