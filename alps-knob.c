#include "alps-knob.h"

#include "msp430g2553.h"

/** Timer TA0 is used for ALPS-knob debounce */
static inline void
TA0Init(void)
{
	/* Timer runs @ 1500Hz */
	TA0CTL = TASSEL_1 + ID_3 + MC_0 + TAIE; // ACLK is source, divider /8, timer halted, periode interrupt one
	TA0CCR0 = 150;	// 1/10 sec at this timer frequ.
}

/** Start TA0 now. */
static inline void
TA0_START(void)
{
    TA0R = 0;
    TA0CTL |= MC0;
}

/** Stop TA0 now. */
static inline void
TA0_STOP(void)
{
    TA0CTL &= ~MC0;
}

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
	P2IE &= ~BIT0;
	// ebable anti-beat timer
	TA0_START();

	/* count increments */
	if (P2IN & BIT2){
		if (P2IN & BIT1){
			increments += KNOB_INC_PRESSED;
		} else {
			increments += KNOB_INC_NORMAL;
		}
	} else if (~(P2IN & BIT2)){
		if(P2IN & BIT1){
			increments -= KNOB_INC_PRESSED;
		} else {
			increments -= KNOB_INC_NORMAL;
		}
	}
}

/** ISR for timer-based anti-beat */
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TA0_A0_ISR(void) {
	TA0_STOP();
	// Clear knob interrupt flag and reenable interrupt
	P2IFG &= ~BIT0;
	P2IE |= BIT0;
}

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

int8_t
knob_get_changes(void)
{
    int8_t retval = 0;
    if (TA0CTL & MC0){
        /* Debounce timer is running, come back later. */
        return 0;
    }

    /* disable interrupt for fetching */
	P2IE &= ~BIT0;
	retval = increments;
	increments = 0;
	P2IE |= BIT0;
	return retval;
}

bool
knob_changed(void)
{
    return (increments != 0);
}
