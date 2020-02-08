#include "alps-knob.h"

#include "msp430g2553.h"
#include "port-helpers.h"

static volatile int8_t increments = 0;

/**ISR for Alps knob operation
 *
 * The Alps knob is connected to pins 2.0, 2.1 and 2.2 where 2.1 is the push
 * and the others are the turn indications. Only interrupt on 2.0 is enabled.
 * Operation mode on interrupt is:
 * 2.2 high and 2.1 low: reduce volume by 1
 * 2.2 high and 2.1 high: reduce volume by 3
 * 2.2 low and 2.1 low: increase vol by 1
 * 2.2 low and 2.1 high: increase vol by 3
 */
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void){
	// Disable interrupt until handling complete
	BIT_unset(P2IE, BIT0);

	const unsigned char input = PIN_readIn(P2, (BIT1 | BIT2));
	/* count increments */
	if (input & BIT2){
		if (input & BIT1){
			increments += KNOB_INC_PRESSED;
		} else {
			increments += KNOB_INC_NORMAL;
		}
	} else {
		if (input & BIT1){
			increments -= KNOB_INC_PRESSED;
		} else {
			increments -= KNOB_INC_NORMAL;
		}
	}

	// Wait a little for debounce
	__delay_cycles(100000);
	// Re-enable interrupt
	BIT_unset(P2IFG, BIT0);
	BIT_set(P2IE, BIT0);
}


void knob_setup(){
	// Set up pins 2.0, 2.1 and 2.2 for digital input with pull-down resistors
	PIN_setModeGPI_pulldown(P2, (BIT0 | BIT1 | BIT2));
	// Enable input interrupts for pin 2.0 only, clear interrupt flags
	P2IE = BIT0;
	BIT_set(P2IES, BIT0);
	BIT_unset(P2IFG, BIT0);
}

int8_t
knob_get_changes(void)
{
	// disable interrupt for reading
	BIT_unset(P2IE, BIT0);
	const int8_t retval = increments;
	increments = 0;
	BIT_set(P2IE, BIT0);
	return retval;
}
