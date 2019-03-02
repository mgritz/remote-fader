#include <msp430.h> 
#include  "msp430g2553.h"
#include "fsRemoteFaderFunctionality.h"

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    clockInit();
//    UART_Init(&faderLevel);
//    knobSetup();
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
