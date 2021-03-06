#include "nixie-disp.h"

#include "msp430g2553.h"
#include "conf_MAX7300.h"
#include "i2c.h"
#include "port-helpers.h"

void
nixieSetup()
{
	// define nixie enable pin as output - should have been turned off before.
    PIN_setModeGPO(P1, BIT5);

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
}

void
nixieWrite(uint8_t value)
{
	char lowDigit = 0;
	char highDigit = 0;
	char i = 9;
	char tenI = 90;
	char lowDigitAddress = MAX7300_RA_P12;
	char highDigitAddress = MAX7300_RA_P22;

	/* split digits without division */
	for(tenI = 90; tenI >= 0; tenI -= 10){
		if(value >= tenI){
			highDigit = i;
			break;
		} else {
			i--;
		}
	}

	lowDigit = value - (highDigit * 10);

	// calculate digits to turn on
	if(highDigit > 0){
		highDigitAddress = MAX7300_RA_P31 + 1 - highDigit;
	}
	if(lowDigit > 0){
		lowDigitAddress = MAX7300_RA_P21 + 1 - lowDigit;
	}

	// turn nixie power off
	PIN_outLow(P1, BIT5);

	// reset the MAX7300 since we don't know its state
	nixieSetup();

	// disable all digits
	i2cWrite(MAX7300_RA_P12TO19, 0x00);
	i2cWrite(MAX7300_RA_P20TO27, 0x00);
	i2cWrite(MAX7300_RA_P27TO31, 0x00);

	// enable new digits
	i2cWrite(highDigitAddress, 0x01);
	i2cWrite(lowDigitAddress, 0x01);

	// turn nixie power back on
	PIN_outHigh(P1, BIT5);
}
