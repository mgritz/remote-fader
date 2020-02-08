#ifndef UART_h
#define UART_h

#include <stdbool.h>

#define UART_BUFFER_SIZE 16

void UART_Init(void);

void UartPutChar(char character);
void UartPutStr(const char* const str, int length);

bool UartGetNext(char* c);

#endif  //UART_h
