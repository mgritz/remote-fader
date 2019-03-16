#include <msp430.h> 
#include  "msp430g2553.h"

#include "uartbuf.h"
#include "nixie-disp.h"

static inline void
clockInit(void) {
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

volatile char faderLevel = 0;		// used to store fader level
volatile char lastFaderLevel = 1;

int
main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    clockInit();
    UART_Init(&faderLevel);
    nixieSetup();
    __enable_interrupt();

    faderLevel = 0;
    while(1){
    	if(faderLevel != lastFaderLevel){
    		nixieWrite(faderLevel);
    	}
    	lastFaderLevel = faderLevel;
    }
}

/* Uart und I2C funktionieren jeweils für sich.
 * Empfangener Fader-Level wird gelegentlich nicht verarbeitet. --> Ausführung von atoi prüfen.
 *
 * Timer Interrupts kommen nicht, bzw. werden getrappt. --> Bezeichnung der Vektoren prüfen.
 *
 * P2 Interrupts kommen "einfach so". Vermutlich wg. offener Klemmen.
 *
 */
