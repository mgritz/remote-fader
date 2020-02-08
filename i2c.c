#include "i2c.h"

#include "msp430g2553.h"

#include "port-helpers.h"

#define SCL BIT6
#define SDA BIT7

static volatile unsigned char i2c_fault = 0;


/** Use Timer A0 to safeguard I2C access for hangups. */
static inline void
TA0Init(void)
{
	/* Timer runs @ 1500Hz */
	TA0CTL = TASSEL_1 + ID_3 + MC_0; // ACLK is source, divider /8, timer halted
	TA0CCR0 = 750;	// .5 sec at this timer frequ.
	TACCTL0 = CCIE; // Capture/Compare 0 Interrupt Enable
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
/** ISR for timer-based anti-beat */
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TA0_A0_ISR(void) {
	TA0_STOP();
	i2c_fault = 1;
	__bic_SR_register_on_exit(CPUOFF);	//exit LPM
}

void i2cSetup(int Adress){
	// Pulse SCL 9 times to bring slave into defined state

	PIN_setModeGPO(P1, SCL);
	PIN_setModeGPI_nopull(P1, SDA);
	for (int i = 9; (i > 0) && (!PIN_readIn(P1, SDA)); i--)
	{
		PIN_outHigh(P1, SCL);
		__delay_cycles(1);// If F_CPU = 1MHz this should be equivalent to 250kHz.
		PIN_outLow(P1, SCL);
		__delay_cycles(1);
	}
	PIN_setModeGPO(P1, SDA);
	PIN_outLow(P1, SDA);
	PIN_outHigh(P1, SCL);
	__delay_cycles(1);
	PIN_outHigh(P1, SDA);

	// Setup actual I2C module
    PIN_setModeSecPeriph(P1, SCL | SDA);
	UCB0CTL1 |= UCSWRST;				//stop UCB0
	UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;		//configure UCB0
	UCB0CTL1 = UCSSEL_2 + UCSWRST;
	UCB0BR0 = 2;					//frequency divider for ca. 1MHz /
	UCB0BR1 = 0;
	UCB0I2CSA = Adress;				//set sensor's I2C address
	UCB0CTL1 &= ~UCSWRST;				//start UCB0
	IE2 |= UCB0TXIE + UCB0RXIE;		//enable TX and RX interrupts

	TA0Init();
}

static volatile unsigned char TransmittedData;
static volatile int WriteMode = 0;
static volatile int phase = 0;
static volatile unsigned char WriteRegister;
static volatile char wdtHold = 0;

void i2cWrite(int Adress, int Value){
	while (UCB0CTL1 & UCTXSTP);			//check if STOP is sent
	WriteMode = 1;						//important for the ISR procedure
	WriteRegister = Adress;
	TransmittedData = Value;
	phase = 0;					//important for ISR procedure

	IE2 &= ~UCB0RXIE;			//clear RX interrupt-enable...
	IE2 |= UCB0TXIE;			//... and set TX interrupt-enable

	UCB0CTL1 |= UCTR + UCTXSTT;	//send START condition + address + Write
	TA0_START();
	__bis_SR_register(CPUOFF + GIE);	//go into a LPM and wait for ISR
	TA0_STOP();
}

volatile static unsigned char ReceivedData;

unsigned char i2cRequest(unsigned char Register){
	TransmittedData = Register;
	WriteMode = 0;			//important for ISR procedure
	phase = 0;				//important for ISR procedure
	IE2 &= ~UCB0RXIE;		//clear RX interrupt-enable
	IE2 |= UCB0TXIE;		//set TX interrupt-enable

	while (UCB0CTL1 & UCTXSTP);		//ensure STOP condition was sent
	UCB0CTL1 |= UCTR + UCTXSTT;		//send START condition + address + Write
	TA0_START();
	__bis_SR_register(CPUOFF + GIE);//go into a LPM and wait for ISR
	TA0_STOP();

	if (i2c_fault) {
		return 0;
	}

	while (UCB0CTL1 & UCTXSTP);		//ensure STOP was sent
	return ReceivedData;
}


#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void)
{
	if(WriteMode){
		if(phase == 0){
			UCB0TXBUF = WriteRegister;		//put the register address on the bus
			phase = 1;

		}
		else if (phase == 1){
			phase = 2;
			UCB0TXBUF = TransmittedData;		//put data on the bus

		}
		else if(phase == 2){
			IFG2 &= ~(UCB0TXIFG + UCB0RXIFG);	//disable RX and TX interrupts
			UCB0CTL1 |= UCTXSTP;				//send STOP condition
			__bic_SR_register_on_exit(CPUOFF);	//exit LPM
			WriteMode = 0;
			phase = 0;
		}
	}
	else{
		//Read-mode
		if(phase == 0){
			UCB0TXBUF = TransmittedData;		//put register address on the bus
			phase = 1;
		}
		else if (phase == 1){
			IE2 &= ~UCB0TXIE;			//disable TX interrupts
			IE2 |= UCB0RXIE;			//enable RX interrupts

			UCB0CTL1 &= ~UCTR;			//clear UCTR bit (means READ mode on next START condition)
			UCB0CTL1 |= UCTXSTT;			//send START condition

			while (UCB0CTL1 & UCTXSTT);			//wait for START to be sent
			UCB0CTL1 |= UCTXSTP;			//send STOP condition and wait for the sensor-data
			phase = 2;
		}
		else if (phase == 2){
			ReceivedData = UCB0RXBUF;		//read sensor-data from the bus
			IFG2 &= ~(UCB0TXIFG + UCB0RXIFG);	//disable interrupts
			__bic_SR_register_on_exit(CPUOFF);	//exit LPM
		}
	}
}
