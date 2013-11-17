#ifndef _NRF905_H_
#define _NRF905_H_

#include <stdint.h>
#include <avr/io.h>
#include "spi.h"

// Instructions
#define W_CONFIG     0b00000000
#define R_CONFIG     0b00010000
#define W_TX_PAYLOAD 0b00100000
#define R_TX_PAYLOAD 0b00100001
#define W_TX_ADDRESS 0b00100010
#define R_TX_ADDRESS 0b00100011
#define R_RX_PAYLOAD 0b00100100
#define CHANNEL_CONFIG 0b10000000
// Registers
#define NRF905_REG_CHANNEL     0x00
#define NRF905_REG_AUTO_RETRAN 0x01
#define NRF905_REG_LOW_RX      0x01
#define NRF905_REG_PWR         0x01
#define NRF905_REG_BAND        0x01
#define NRF905_REG_CRC_MODE    0x09
#define NRF905_REG_CRC         0x09
#define NRF905_REG_CLK         0x09
#define NRF905_REG_OUTCLK      0x09
#define NRF905_REG_OUTCLK_FREQ 0x09
#define NRF905_REG_RX_ADDRESS  0x05
#define NRF905_REG_RX_PAYLOAD_SIZE 0x03
#define NRF905_REG_TX_PAYLOAD_SIZE 0x04
#define NRF905_REG_ADDR_WIDTH      0x02
// Register masks
#define NRF905_MASK_CHANNEL 0xFC
#define NRF905_MASK_AUTO_RETRAN 0x20
#define NRF905_MASK_LOW_RX 0x10
#define NRF905_MASK_PWR 0x0c
#define NRF905_MASK_BAND 0x02
#define NRF905_MASK_CRC_MODE 0x80
#define NRF905_MASK_CRC 0x40
#define NRF905_MASK_CLK 0x38
#define NRF905_MASK_OUTCLK 0x04
#define NRF905_MASK_OUTCLK_FREQ 0x03

#define NRF905_BAND_433 0x00
#define NRF905_BAND_868 0x02
#define NRF905_BAND_915 0x02
#define NRF905_PWR_n10 0x00
#define NRF905_PWR_n2 0x04
#define NRF905_PWR_6 0x08
#define NRF905_PWR_10 0x0C
#define NRF905_LOW_RX_ENABLE 0x10
#define NRF905_LOW_RX_DISABLE 0x00
#define NRF905_AUTO_RETRAN_ENABLE 0x20
#define NRF905_AUTO_RETRAN_DISABLE 0x00
#define NRF905_OUTCLK_ENABLE 0x04
#define NRF905_OUTCLK_DISABLE 0x00
#define NRF905_OUTCLK_4MHZ 0x00
#define NRF905_OUTCLK_2MHZ 0x01
#define NRF905_OUTCLK_1MHZ 0x02
#define NRF905_OUTCLK_500KHZ 0x03
#define NRF905_CRC_ENABLE 0x40
#define NRF905_CRC_DISABLE 0x00
#define NRF905_CRC_MODE_8 0x00
#define NRF905_CRC_MODE_16 0x80
#define NRF905_XOF_16 0x18

#define N_CE      PB4
#define N_CSN     PA0
#define N_PWR     PB3
#define N_CD      PD2
#define N_TXEN    PD6

#define nrf_DDRA_out   (DDRA |= (_BV(N_CSN)))
#define nrf_DDRA_in
#define nrf_DDRB_out   (DDRB |= (_BV(N_CE)|_BV(N_PWR)))
#define nrf_DDRB_in
#define nrf_DDRD_out   (DDRD |= (_BV(N_TXEN)))
#define nrf_DDRD_in    (DDRD &= ~(_BV(N_CD)))

#define nrf_CSN_hi     (PORTA |= _BV(N_CSN))
#define nrf_CSN_lo     (PORTA &= ~_BV(N_CSN))
#define nrf_CE_hi      (PORTB |= _BV(N_CE))
#define nrf_CE_lo      (PORTB &= ~_BV(N_CE))
#define nrf_PWR_hi     (PORTB |= _BV(N_PWR))
#define nrf_PWR_lo     (PORTB &= ~_BV(N_PWR))
#define nrf_TXEN_hi    (PORTD |= _BV(N_TXEN))
#define nrf_TXEN_lo    (PORTD &= ~_BV(N_TXEN))

#define nrf_CD_IN (PIND & _BV(N_CD))

#define nrf_PAYLOAD    32

// SA Bands
// 433.05-434.79 (Ch 107-124) (Actually 123.9)
// 868-868.6
// 868.7-869.2 (Ch 12)
// 869.4-869.65
// 869.7-870

#define nrf_powerup() (nrf_PWR_hi)
#define nrf_powerdown() (nrf_PWR_lo)
void nrf_init();
void nrf_config();
void nrf_wr_config(uint8_t reg, uint8_t val);
uint8_t nrf_rd_config(uint8_t reg);
void nrf_send(uint8_t * data, uint8_t len);
void nrf_tx_addr(uint32_t addr);
uint32_t nrf_rd_tx_addr(void);
void nrf_tx_payload(uint8_t *data,uint8_t len);
uint8_t nrf_status(void);

extern volatile uint8_t nrf_int;

#endif // _NRF905_H_
