#ifndef _MAIN_H__
#define _MAIN_H__

#define _XTAL_FREQ 20000000 // 20 MHz
#include <pic.h>
#include <conio.h>
#include <stdlib.h>
#include <stdio.h>

// functions
void interrupt interruptFunction(void);
unsigned int readAD(char);
void sleepms(unsigned int);
void spi_write(char);

// SPI lines
#define SCK RB0
#define SI RB1
#define CSN RB2
#endif