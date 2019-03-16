#include "msp430g2553.h"
#include "uartbuf.h"
#include "i2c.h"
#include "conf_MAX7300.h"
#include <string.h>
#include <stdlib.h>



volatile char idleFlag = 0;			// states the controller is idle
volatile char requestLevelFlag = 0;	// main loop notified that the level is to be requested
volatile char faderLevel = 0;		// used to store fader level

/** Timer TA1 is used for cyclic fader level request if uC is idle */
#define TA1_START {TA1R = 0; TA1CTL |= MC0;}
#define TA1_STOP {TA1CTL &= ~MC0;}
#define TA1_RESTART {TA1_STOP TA1_START}
void TA1Init(void){
	/* Timer runs @ 1500Hz */
	TA1CTL = TASSEL_1 + ID_3 + MC_0 + TAIE; // ACLK is source, divider /8, timer halted, periode interrupt off
	TA1CCR0 = 7500;	// 5 sec at this timer frequ.
}



void nixieSetup(){
	// define nixie enable pin as output, turned off
	P1OUT &= ~(BIT5);
	P1DIR |= BIT5;

	// setup i2c driver
	i2cSetup(MAX7300_SLAVE_ADDRESS);

	// define MAX7300 ports correctly (output)
	i2cWrite(MAX7300_RA_PCONF12TO15, 0x55);
	i2cWrite(MAX7300_RA_PCONF16TO19, 0x55);
	i2cWrite(MAX7300_RA_PCONF20TO23, 0x55);
	i2cWrite(MAX7300_RA_PCONF24TO27, 0x55);
	i2cWrite(MAX7300_RA_PCONF28TO31, 0x55);

	// get MAX7300 ready for action (Shutdown disabled, Transit detect off)
	i2cWrite(MAX7300_RA_CONFIG, 0x01);

	// enable nixie supply
	P1OUT |= BIT5;

	// enable cyclic lavel request timer
//	TA1Init();
//	TA1_START;

	//FIXME Make the timer Interrupts work correctly
}

void nixieWrite(){
	int lowDigit = 0, highDigit = 0, i = 9, tenI = 90;
	char lowDigitAddress = MAX7300_RA_P22, highDigitAddress = MAX7300_RA_P12;

	for(tenI = 90; tenI >= 0; tenI -= 10){
		if(faderLevel >= tenI){
			highDigit = i;
			break;
		}
		else{
			i--;
		}
	}

	lowDigit = faderLevel - (highDigit * 10);

	// calculate digits to turn on
	if(highDigit > 0){
		highDigitAddress = MAX7300_RA_P21 + 1 - highDigit;
	}
	if(lowDigit > 0){
		lowDigitAddress = MAX7300_RA_P31 + 1 - lowDigit;
	}

	// turn nixie power off
	P1OUT &= ~BIT5;

	// disable all digits
	i2cWrite(MAX7300_RA_P12TO19, 0x00);
	i2cWrite(MAX7300_RA_P20TO27, 0x00);
	i2cWrite(MAX7300_RA_P27TO31, 0x00);

	// enable new digits
	i2cWrite(highDigitAddress, 0x01);
	i2cWrite(lowDigitAddress, 0x01);

	// turn nixie power back on
	P1OUT |= BIT5;
}

/*
void getCorrectedFaderLevel(){
	char string[] = "XXXXX";
	const char rightAnswer[] = "FADER";
	UartFlushBuffer();
	UartPutStr("@FADER \r", 8);
	UartGetString(string, 8, 1);
	char* found = strstr(string, rightAnswer);
	if(found != NULL){
		int receivedNumber = atoi(found);
		if(receivedNumber >= 0 && receivedNumber <=99)
			faderLevel = receivedNumber;
	}
}
*/



/** ISR for idle-state diplay update */
#pragma vector=TIMER1_A0_VECTOR
__interrupt void TA1_A0_ISR(void) {
	if(idleFlag){
		requestLevelFlag = 1;
	}
	idleFlag = 1;
}

#pragma vector=TIMER0_A1_VECTOR
__interrupt void TA0_A1_ISR(void){
	UartPutStr("TA0_A1_ISR\r", 11);
}

#pragma vector=TIMER1_A1_VECTOR
__interrupt void TA1_A1_ISR(void){
	UartPutStr("TA1_A1_ISR\r", 11);
}
