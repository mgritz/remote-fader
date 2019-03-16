#include <msp430.h> 
#include  "msp430g2553.h"
#include "fsRemoteFaderFunctionality.h"

void
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

int
main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    clockInit();
//    UART_Init(&faderLevel);
    nixieSetup();
    __enable_interrupt();

    faderLevel = 0;
    char lastFaderLevel = 1;
    while(1){
    	if(faderLevel != lastFaderLevel){
    		nixieWrite();
    	}
    	lastFaderLevel = faderLevel;
    	faderLevel++;
    	if (faderLevel > 99) faderLevel = 0;
    	__delay_cycles(100000);
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
