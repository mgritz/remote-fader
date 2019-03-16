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
