#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "nrf905.h"
#include "spi.h"

// rx address
#define RX_ADDR  0xe7e7e7e7

static volatile uint8_t PTX;
volatile uint8_t nrf_int;
void nrf_init()
{
    nrf_DDRA_out;
    nrf_DDRA_in;
    nrf_DDRB_out;
    nrf_DDRB_in;
    nrf_DDRD_out;
    nrf_DDRD_in;

    nrf_CSN_hi;
    nrf_CE_lo;
    nrf_PWR_lo;
    nrf_TXEN_lo;
    // enable interrupt
    MCUCR |= (1<<ISC11) | (1<<ISC10);
    GIMSK |= (1<<INT1);
    nrf_int=0;
}
void nrf_wr_config(uint8_t reg, uint8_t val)
{
    nrf_CSN_lo;
    spi_wr(reg);
    spi_wr(val);
    nrf_CSN_hi;
}
uint8_t nrf_rd_config(uint8_t reg)
{
    nrf_CSN_lo;
    spi_wr(R_CONFIG|reg);
    uint8_t value = spi_rd();
    nrf_CSN_hi;
    return value;
}
void nrf_config()
{
    nrf_wr_config(W_CONFIG|NRF905_REG_CHANNEL,117); // channel 107 (433.1)
    nrf_wr_config(W_CONFIG|1,(NRF905_PWR_10)); // +10 dBm
//    nrf_wr_config(W_CONFIG|1,(NRF905_PWR_n10)); // -10 dBm
    nrf_wr_config(W_CONFIG|NRF905_REG_ADDR_WIDTH,0x44); // 4/4
    nrf_wr_config(W_CONFIG|NRF905_REG_RX_PAYLOAD_SIZE,32);
    nrf_wr_config(W_CONFIG|NRF905_REG_TX_PAYLOAD_SIZE,32);
//    nrf_wr_config(W_CONFIG|9,(NRF905_CRC_MODE_16|NRF905_CRC_ENABLE|NRF905_XOF_16));
    nrf_wr_config(W_CONFIG|9,(NRF905_CRC_MODE_16|NRF905_CRC_ENABLE|NRF905_XOF_16));
    nrf_powerup();
}
void nrf_tx_addr(uint32_t addr)
{
    nrf_CSN_lo;
    spi_wr(W_TX_ADDRESS);
    spi_wr((uint8_t)(addr & 0xff));
    spi_wr((uint8_t)((addr >> 8) & 0xff));
    spi_wr((uint8_t)((addr >> 16) & 0xff));
    spi_wr((uint8_t)((addr >> 24) & 0xff));
    nrf_CSN_hi;
}
uint32_t nrf_rd_tx_addr(void)
{
    uint32_t addr;
    nrf_CSN_lo;
    spi_wr(R_TX_ADDRESS);
    spi_rd_buf((uint8_t *)&addr,4);
    nrf_CSN_hi;
    return addr;
}
void nrf_tx_payload(uint8_t *data,uint8_t len)
{
    nrf_CSN_lo;
    spi_wr(W_TX_PAYLOAD);
    spi_wr_buf(data,len);
    nrf_CSN_hi;
}
void nrf_send(uint8_t * data, uint8_t len)
{
    // Assume the device is powered up and in standby mode
    nrf_TXEN_hi;
    nrf_tx_payload(data,len);
    nrf_wr_config(W_CONFIG|NRF905_REG_TX_PAYLOAD_SIZE,len);
    // Send it
    PTX = 1;
    nrf_CE_hi;
    _delay_us(10);
    nrf_CE_lo;

}
uint8_t nrf_status(void)
{
    nrf_CSN_lo;
    uint8_t status=spi_rw(R_CONFIG);
    nrf_CSN_hi;
    return status;
}
// Interrupt handler
ISR (INT1_vect)
{
    if (PTX) {
        PTX = 0;
        nrf_powerdown();         // go to pawer down mode after successfull TX
    }
    nrf_int++;
}
