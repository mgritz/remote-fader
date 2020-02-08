#include "uartbuf.h"
#include "msp430g2553.h"

#include <ring-buffer.h>

RING_BUFFER_API(bbuf, char)
RING_BUFFER(bbuf, char)

char rx_buffer_mem[UART_BUFFER_SIZE];
bbuf rx_buffer;

/************************************************************************************************************
 * 																											*
 * The recommended USCI initialization/re-configuration process is:											*
 * 																											*
 * 1) Set UCSWRST (BIS.B  #UCSWRST,&UCAxCTL1)																*
 * 2) Initialize all USCI registers with UCSWRST = 1 (including UCAxCTL1)									*
 * 3) Configure ports.																						*
 * 4) Clear UCSWRST via software (BIC.B  #UCSWRST,&UCAxCTL1)												*
 * 5) Enable interrupts (optional) via UCAxRXIE and/or UCAxTXIE												*
 * 																											*
 ************************************************************************************************************/
void UART_Init(void)
{
    // prepare rx buffer
    bbuf_init(&rx_buffer, rx_buffer_mem, UART_BUFFER_SIZE);

	// configure pins as UART
	P1SEL |= BIT1 | BIT2;
	P1SEL2 |= BIT1 | BIT2;

	UCA0CTL1 |= UCSSEL_2;                     // SMCLK
	UCA0BR0 = 104;                            // 1MHz 9600
	UCA0BR1 = 0;                              // 1MHz 9600
	UCA0MCTL = UCBRS0;                        // Modulation UCBRSx = 1

	// activate UART
	UCA0CTL1 &= ~UCSWRST;

	// Delete initial interrupt flags
	IFG2 &= ~UCA0RXIFG;

	// enable RX interrupt
	IE2 |= UCA0RXIE;
}

void UartPutChar(char character){
	while (!(IFG2&UCA0TXIFG));
		UCA0TXBUF = character;
}

void UartPutStr(const char* const str, int length){
	int i = 0;
	for(i=0;i<length;i++){
		UartPutChar(str[i]);
		if(str[i] == '\0') break;
	}
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
	bbuf_put(&rx_buffer, UCA0RXBUF);
}

bool UartGetNext(char* c)
{
    __disable_interrupt();
    const bool rc = !bbuf_empty(&rx_buffer);
    *c = bbuf_get(&rx_buffer);
    __enable_interrupt();
    return rc;
}
