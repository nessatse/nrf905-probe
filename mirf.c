
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "mirf.h"
#include "nrf24L01.h"
#include "spi.h"

// Defines for setting the MiRF registers for transmitting or receiving mode
#define TX_POWERUP mirf_config_register(CONFIG, mirf_CONFIG | ((1<<PWR_UP) | (0<<PRIM_RX) ))
#define RX_POWERUP mirf_config_register(CONFIG, mirf_CONFIG | ((1<<PWR_UP) | (1<<PRIM_RX) ))

// Flag which denotes transmitting mode
volatile uint8_t PTX;
volatile uint8_t TxCount;

void mirf_init()
// Initializes pins ans interrupt to communicate with the MiRF module
// Should be called in the early initializing phase at startup.
{
    spi_init();
    // Define CSN and CE as Output and set them to default
//    DDRB |= ((1<<CSN)|(1<<CE));
    mirf_CE_lo;
    mirf_CSN_hi;

//   MCUCR = ((1<<ISC11)|(0<<ISC10));
//   GIMSK = _BV(INT1);
    // Initialize spi module

}

void mirf_config()
// Sets the important registers in the MiRF module and powers the module
// in receiving mode
{
    // Set RF channel
    mirf_config_register(RF_CH,mirf_CH);

    // Set length of incoming payload
    mirf_config_register(RX_PW_P0, mirf_PAYLOAD);
    // Set RADDR and TADDR as the transmit address since we also enable auto acknowledgement
    mirf_write_register(RX_ADDR_P0, TADDR, 5);
    mirf_write_register(TX_ADDR, TADDR, 5);

    mirf_config_register(RF_SETUP, 1<<RF_DR_LOW);
    mirf_config_register(SETUP_RETR, 7<<ARD);
    // Enable RX_ADDR_P0 address matching
    mirf_config_register(EN_RXADDR, 1<<ERX_P0);
    mirf_config_register(DYNPD, 1<<DPL_P0);
    mirf_config_register(FEATURE, 1<<EN_DPL);

    // Start transmitter
    PMODE = TXMODE; // Start in transmitting mode
    PTX = 0;
    TX_POWERUP;
    // enable IRQ
    GIMSK |= (1<<INT1);
//    mirf_CE_hi;     // Listening for pakets
}

void mirf_set_RADDR(uint8_t * adr)
// Sets the receiving address
{
    mirf_CE_lo;
    mirf_write_register(RX_ADDR_P0,adr,5);
    mirf_CE_hi;
}

void mirf_set_TADDR(uint8_t * adr)
// Sets the transmitting address
{
    mirf_write_register(TX_ADDR, adr,5);
}

// Interrupt handler
ISR (INT1_vect)
{
    uint8_t status;
    mirf_config_register(STATUS,(1<<TX_DS)|(1<<MAX_RT)); // Reset status register
    // If still in transmitting mode then finish transmission
    if (PTX) {

        TxCount++;
        // Read MiRF status
//        mirf_CSN_lo;                                // Pull down chip select
//        status = spi_rw(NOP);               // Read status register
//        mirf_CSN_hi;                                // Pull up chip select
        PTX = 0;                                // Set to receiving mode
        MIRF_POWERDOWN;         // go to pawer down mode after successfull TX
    }
    // Reset status register for further interaction
}

extern uint8_t mirf_data_ready()
// Checks if data is available for reading
{
    if (PTX) return 0;
    uint8_t status;
    // Read MiRF status
    mirf_CSN_lo;                                // Pull down chip select
    status = spi_rw(NOP);               // Read status register
    mirf_CSN_hi;                                // Pull up chip select
    return status & (1<<RX_DR);
}

extern void mirf_get_data(uint8_t * data)
// Reads mirf_PAYLOAD bytes into data array
{
    mirf_CSN_lo;                               // Pull down chip select
    spi_wr( R_RX_PAYLOAD );            // Send cmd to read rx payload
    spi_rd_buf(data,mirf_PAYLOAD); // Read payload
    mirf_CSN_hi;                               // Pull up chip select
    mirf_config_register(STATUS,(1<<RX_DR));   // Reset status register
}

void mirf_config_register(uint8_t reg, uint8_t value)
// Clocks only one byte into the given MiRF register
{
    mirf_CSN_lo;
    spi_wr(W_REGISTER | (REGISTER_MASK & reg));
    spi_wr(value);
    mirf_CSN_hi;
}

void mirf_read_register(uint8_t reg, uint8_t * value, uint8_t len)
// Reads an array of bytes from the given start position in the MiRF registers.
{
    mirf_CSN_lo;
    spi_wr(R_REGISTER | (REGISTER_MASK & reg));
    spi_rd_buf(value,len);
    mirf_CSN_hi;
}

void mirf_write_register(uint8_t reg, uint8_t * value, uint8_t len)
// Writes an array of bytes into inte the MiRF registers.
{
    mirf_CSN_lo;
    spi_wr(W_REGISTER | (REGISTER_MASK & reg));
    spi_wr_buf(value,len);
    mirf_CSN_hi;
}

void mirf_send(uint8_t * value, uint8_t len)
// Sends a data package to the default address. Be sure to send the correct
// amount of bytes as configured as payload on the receiver.
{
//   while (PTX) {}                  // Wait until last paket is send

    mirf_CE_lo;

    PTX = 1;                        // Set to transmitter mode
    TX_POWERUP;                     // Power up

    mirf_CSN_lo;                    // Pull down chip select
    spi_wr( FLUSH_TX );     // Write cmd to flush tx fifo
    mirf_CSN_hi;                    // Pull up chip select

    mirf_CSN_lo;                    // Pull down chip select
    spi_wr( W_TX_PAYLOAD ); // Write cmd to write payload
    spi_wr_buf(value,len);   // Write payload
    mirf_CSN_hi;                    // Pull up chip select

    mirf_CE_hi;                     // Start transmission
    _delay_us(20);
    mirf_CE_lo;
}

// Checks if MAX_RT has been reached
uint8_t mirf_max_rt_reached(void)
{
    mirf_CSN_lo; // Pull down chip select
    spi_wr(R_REGISTER | (REGISTER_MASK & STATUS));
    uint8_t status = spi_rw(NOP); // Read status register
    mirf_CSN_hi; // Pull up chip select
    return status & (1<<MAX_RT);
}
uint8_t mirf_status(void)
{
    mirf_CSN_lo;
    uint8_t status =  spi_rw(NOP);
    mirf_CSN_hi;
    return status;
}
