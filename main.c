#include <msp430.h> 
#include "msp430g2553.h"

#include "nixie-disp.h"
#include "ap20-ifc.h"

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

volatile char lastFaderLevel = 1;

int
main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    clockInit();
    ap20_init();
    __enable_interrupt();

    while(1){
        /* see if the AP20 gave something over UART */
        ap20_process_bytes();
        const uint8_t current_lvl = ap20_current_level();
    	if(current_lvl != lastFaderLevel){
    		nixieWrite(current_lvl);
    	}
    	lastFaderLevel = current_lvl;
    }
}
