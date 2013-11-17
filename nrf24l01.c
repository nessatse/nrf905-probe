#include <avr/io.h>
#include "nrf24l01.h"
#include "spi.h"

#define CSN_HI  (SPIPORT |= _BV(CSN))
#define CSN_LO  (SPIPORT &= ~_BV(CSN))
#define CE_HI  (SPIPORT |= _BV(CE))
#define CE_LO  (SPIPORT &= ~_BV(CE))

static volatile uint8_t PTX;

void nrf_init(void)
{
    spi_init();
    CE_LO;
    CSN_HI;
    // Initialise INT1
    MCUCR = ((1<<ISC11)|(0<<ISC10));
    GIMSK = _BV(INT1);
}

ISR (INT1_vect)
{
    PTX = 0;
}

void nrf_send(uint8_t * value, uint8_t len)
// Sends a data package to the default address. Be sure to send the correct
// amount of bytes as configured as payload on the receiver.
{
    while (PTX) {}                  // Wait until last paket is send

    CE_LO;

    PTX = 1;                        // Set to transmitter mode
 //   TX_POWERUP;                     // Power up

    CSN_LO;                    // Pull down chip select
    spi_wr( FLUSH_TX );     // Write cmd to flush tx fifo
    CSN_HI;                    // Pull up chip select

    CSN_LO;                    // Pull down chip select
    spi_wr( W_TX_PAYLOAD ); // Write cmd to write payload
    spi_wr_buf(value,len);   // Write payload
    CSN_HI;                    // Pull up chip select

    CE_HI;                     // Start transmission
}
