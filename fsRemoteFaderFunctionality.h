#include "msp430g2553.h"
#include "uartbuf.h"
#include "i2c.h"
#include "conf_MAX7300.h"
#include <string.h>
#include <stdlib.h>



volatile char idleFlag = 0;			// states the controller is idle
volatile char requestLevelFlag = 0;	// main loop notified that the level is to be requested
volatile char faderLevel = 0;		// used to store fader level

void clockInit(void) {
	/* Setting clocks to:
	 * MCLK = SMCLK = 1MHz
	 * ACLK = 12kHz
	 */
    DCOCTL = CALDCO_1MHZ;		// set DCO to 1MHz
    BCSCTL1 = CALBC1_1MHZ;		// set DCO Range Sel correctly
    BCSCTL1 |= DIVA_0;			// ACLK-divider off
    BCSCTL2 = SELM_0 + DIVM_0 + DIVS_0; // MCLK <- DCO, clock input dividers off


    BCSCTL3 = LFXT1S_2;		// source ACLK from internal VLO @ 12kHz / divider
//    BCSCTL3 = LFXT1S_0 + XCAP_3; // source ACLK from external 32762Hz-osc.
    __delay_cycles(800000);
}

/** Timer TA0 is used for ALPS-knob debounce */
#define TA0_START {TA0R = 0; TA0CTL |= MC0;}
#define TA0_STOP {TA0CTL &= ~MC0;}
void TA0Init(void){
	/* Timer runs @ 1500Hz */
	TA0CTL = TASSEL_1 + ID_3 + MC_0 + TAIE; // ACLK is source, divider /8, timer halted, periode interrupt one
	TA0CCR0 = 150;	// 1/10 sec at this timer frequ.
}

/** Timer TA1 is used for cyclic fader level request if uC is idle */
#define TA1_START {TA1R = 0; TA1CTL |= MC0;}
#define TA1_STOP {TA1CTL &= ~MC0;}
#define TA1_RESTART {TA1_STOP TA1_START}
void TA1Init(void){
	/* Timer runs @ 1500Hz */
	TA1CTL = TASSEL_1 + ID_3 + MC_0 + TAIE; // ACLK is source, divider /8, timer halted, periode interrupt off
	TA1CCR0 = 7500;	// 5 sec at this timer frequ.
}


/**The Alps knob is connected to pins 2.0, 2.1 and 2.2 where 2.1 is the push
 * and the others are the turn indications. Only interrupt on 2.0 is enabled.
 * Operation mode on interrupt is:
 * 2.2 high and 2.1 low: reduce volume by 1
 * 2.2 high and 2.1 high: reduce volume by 3
 * 2.2 low and 2.1 low: increase vol by 1
 * 2.2 low and 2.1 high: increase vol by 3
 *
 * Relatively slowly manual operation allows for complete message sending
 * before next input interrupt is accepted.
 */
void knobSetup(){
	// Set up pins 2.0, 2.1 and 2.2 for digital input with pull-down resistors
	P2SEL &= ~(BIT0 + BIT1 + BIT2);
	P2SEL2 &= ~(BIT0 + BIT1 + BIT2);
	P2DIR &= ~(BIT0 + BIT1 + BIT2);
	P2REN |= (BIT0 + BIT1 + BIT2);
	P2OUT &= ~(BIT0 + BIT1 + BIT2);
	// Enable input interrupts for pin 2.0 only, clear interrupt flags
	P2IE = BIT0;
	P2IES |= BIT0;
	P2IFG &= ~BIT0;

	TA0Init();
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

/**ISR for Alps knob operation */
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void){
	// Disable interrupt until handling complete
	P2IE &= ~BIT0;

	TA0_START	// ebable anti-beat timer

	// reset idle flag
	idleFlag = 0;

	// Execute Command
	if(P2IN & BIT2){
		if(P2IN & BIT1){
			UartPutStr("@FADER +3\r", 10);
		}
		else{
			UartPutStr("@FADER +1\r", 10);
		}
	}
	else if(~(P2IN & BIT2)){
		if(P2IN & BIT1){
			UartPutStr("@FADER -3\r", 10);
		}
		else{
			UartPutStr("@FADER -1\r", 10);
		}
	}
}

/** ISR for timer-based anti-beat */
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TA0_A0_ISR(void) {
	TA0_STOP
	// Clear knob interrupt flag and reenable interrupt
	P2IFG &= ~BIT0;
	P2IE |= BIT0;
	idleFlag = 1;
}

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
