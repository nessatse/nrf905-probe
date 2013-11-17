#include <avr/io.h>
#include "uart.h"

void uart_init(void)
{
    UBRRH = (unsigned char)(UBRRVAL>>8);
    UBRRL = (unsigned char)UBRRVAL;
    /* Enable receiver and transmitter */
    UCSRB = _BV(RXEN)|_BV(TXEN);
    /* Set frame format: 8data, 1stop bit (defaulr)*/
    //UCSRC = _BV(UCSZ1)|_BV(UCSZ0);
    UCSRC = _BV(USBS)|_BV(UCSZ1)|_BV(UCSZ0);
}
void uart_tx( unsigned char data )
{
    /* Wait for empty transmit buffer */
    while ( !( UCSRA & (1<<UDRE)) )
        ;
    /* Put data into buffer, sends the data */
    UDR = data;
}
void uart_completetx(void)
{
    while (!( UCSRA & (1<<TXC)));
    UCSRA &= ~(_BV(TXC));
}

unsigned char uart_rx( void )
{
    /* Wait for data to be received */
    while ( !(UCSRA & (1<<RXC)) )
        ;
    /* Get and return received data from buffer */
    return UDR;
}
void uart_str(char *s)
{
    while (*s) {
        uart_tx(*s++);
    }
}

void uart_tx_hex( unsigned char data)
{
    unsigned char b = data >> 4;
    if (b < 10) uart_tx(b+'0'); else uart_tx(b+'a'-10);
    b = data&0x0f;
    if (b < 10) uart_tx(b+'0'); else uart_tx(b+'a'-10);
}
