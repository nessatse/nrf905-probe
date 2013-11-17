#ifndef _UART_H_
#define _UART_H_

#define  BAUD 2400
#define UBRRVAL (F_CPU/16/BAUD-1)

void uart_init(void);
void uart_tx( unsigned char data );
unsigned char uart_rx( void );
void uart_str(char *s);
void uart_tx_hex( unsigned char data);
void uart_completetx(void);

#endif
