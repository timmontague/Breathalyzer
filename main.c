/**
	Firmware for Breathalyzer - PIC16F876A
	
	HI-TECH ANSI C Compiler (Lite version)
	Command line option "-ICD" for bootloader support
	
	Copyright 2011, Tim Montague
*/

#include "main.h"
#include "serial.h"

#define SMOOTHING_FACTOR 3
#define SMOOTHING_N (1<<SMOOTHING_FACTOR)

//#define DEBUG

char version[] = "1.0.0";

void main(void)
{
	char i;
	char prevValIdx = 0;
	unsigned int val;
	unsigned int prevVal[SMOOTHING_N];
	char buf[20];
	
	//*******************************************************
	// SPI I/O
	//*******************************************************
	TRISB0 = 0; // SCK  - output
	TRISB1 = 0; // MOSI - output
	TRISB2 = 0; // CSN  - output
	
	SCK = 0;
	SI = 0;
	CSN = 1;
	
	//*******************************************************
	// setup A to D converter
	//*******************************************************
	
	// A/D inputs
	TRISA0 = 1;
	TRISA1 = 1;
	TRISA2 = 1;
	TRISA3 = 1;
	TRISA5 = 1;
	
	// T_AD = Fosc/64 -- Max device freq is 20 MHz
	ADCS2 = 1;
	ADCS1 = 1;
	ADCS0 = 0;
	
	ADFM = 1; // right justified (6 MSBs of ADRESH are read as 0)
	
	// AN4-AN0 set to analog input, AN7-AN5 set to digital I/O
	PCFG3 = 0;
	PCFG2 = 0;
	PCFG1 = 1;
	PCFG0 = 0;
	
	ADON = 1; // turn on A/D converter
		
	//*******************************************************
	// setup serial port
	//*******************************************************
	TRISC6 = 0; // tx port as output
	TRISC7 = 1; // rx port as input
	
	// baud rate = 20,000,000/(64 * (SPBRG + 1))
	SPBRG = 7; // baud = 39062 (~ 38400)
	BRGH = 0; // low-speed
    
	SYNC = 0; // asynchronous
	SPEN = 1; // enable serial port
	RCIE = 1; // enable interrupt on serial receive
	
	TXEN = 1; // enable transmit
	CREN = 1; // enable continuous receive
	
	receivedChar = 0;
	serialReceived = 0;
	
	GIE = 1;  // enable general interrupts
	PEIE = 1; // enable peripheral interrupts
	
	// initialize 7-segment display
	CSN = 0;
	// clear
	spi_write(0x76);
	// set brightness
	spi_write(0x7A);
	spi_write(5);
	CSN = 1;
	// get initial A/D reading
	val = 0;
	for (i=0; i < SMOOTHING_N; i++) {
		prevVal[i] = readAD(4);
		val += prevVal[i];
	}

#ifndef DEBUG
	while(1)
	{
		val -= prevVal[prevValIdx]; // subtract oldest reading
		prevVal[prevValIdx] = readAD(4);
		val += prevVal[prevValIdx]; // add new reading
		prevValIdx++;
		prevValIdx &= (SMOOTHING_N-1); // make sure index doesn't go out of bounds
		
		// write A/D value to the 7-segment display
		sprintf(buf, "%4d", val>>SMOOTHING_FACTOR);
		CSN = 0;
		spi_write(buf[0]);
		spi_write(buf[1]);
		spi_write(buf[2]);
		spi_write(buf[3]);
		CSN = 1;
		__delay_ms(100);
	}
#else
	//*******************************************************
	// debug main loop
	//*******************************************************
	while(1)
	{
		 // wait until character is received
		receivedChar = rxChar();
		
		switch (receivedChar)
		{
			case 'v': // print version number
				printf("version: %s\r\n", version);
				break;
			case 'd': // print drunkenness
				CSN = 0;
				val = readAD(4);
				sprintf(buf, "%4d", val);
				printf("buf: %s\r\n", buf);
				spi_write(buf[0]);
				spi_write(buf[1]);
				spi_write(buf[2]);
				spi_write(buf[3]);
				CSN = 1;
				break;
			default:
				// bad input
				break;
		}
	}
#endif
}

void spi_write(char c) {
	char i=8;
	while (i) {
		i--;
		SCK = 0;
		SI = (c>>i)&1;
		__delay_us(10);
		SCK = 1;
		__delay_us(10);
	}
	SCK = 0;
	__delay_us(10);
}

/**
	sleepms waits for 't' milliseconds
*/
void sleepms(unsigned int t)
{
	while (t--)
		__delay_ms(1);
}

/**
	readAD returns the 10-bit analog value of specified
	channel (between 0 and 4)
*/
unsigned int readAD(char channel)
{
	unsigned int ret = 0;
	char buf[10];
	CHS0 = channel & 1;
	CHS1 = (channel & 2) >> 1;
	CHS2 = (channel & 4) >> 2;
	
	__delay_us(10); // give capacitor in A/D converter enough time to charge
	
	GODONE = 1; // start A/D converter
	while(GODONE); // wait until conversion is done
	
	ret = (ADRESH << 8) | ADRESL;
	
	return ret;
}

/**
	interruptFunction is called every time an interrupt occurs.
	If a serial receive interrupt occured, It does error 
	checking on the serial line and then reads all the data,
	if available.
*/
void interrupt interruptFunction(void)
{
	// if the interrupt is a serial receive interrupt
	if(RCIF)
	{
		// overrun error (too many bytes sent at once)
		if (OERR) 
		{
			// reset OERR
			CREN=0;
			CREN=1;
		}
		
		// receive all bytes sent (dropping some if sent too fast)
		while (RCIF)
			receivedChar = RCREG;
		
		serialReceived = 1;
	}
}