#include "ap20-ifc.h"
#include <stdlib.h>
#include "msp430g2553.h"
#include "uartbuf.h"

const char magicWord[6] = "FADER ";
const char* mwPos = magicWord;
char rxedNumber[2] = "00";
char* rxPos = rxedNumber;

uint8_t current_level = 0;

enum ap20_parser_state {
    LISTEN_STATE, NUMBER_STATE
} ap20_state = LISTEN_STATE;

void
ap20_init(void)
{
    UART_Init();
}

void
ap20_change_level(int8_t increments)
{

}

void
ap20_process_bytes(void)
{
    char rx_byte = 0;
    while (UartGetNext(&rx_byte)) {
        switch (ap20_state){
        case LISTEN_STATE:	// waiting for magic word to complete
            if(*mwPos++ != rx_byte){
                mwPos = magicWord;
            }
            else if(mwPos - magicWord == 6){
                mwPos = magicWord;
                ap20_state = NUMBER_STATE;
            }
            break;
        case NUMBER_STATE:
            *rxPos++ = rx_byte;
            if(rxPos - rxedNumber == 2){
                current_level = (char)(atoi(rxedNumber));
                rxPos = rxedNumber;
                ap20_state = LISTEN_STATE;
            }
        }
    }
}

uint8_t
ap20_current_level(void)
{
    return current_level;
}
