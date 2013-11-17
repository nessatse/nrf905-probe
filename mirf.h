#ifndef _MIRF_H_
#define _MIRF_H_

#include <stdint.h>
#include <avr/io.h>
#include "spi.h"

// Mirf settings
#define RXMODE 1
#define TXMODE 0
#define mirf_CH         120
#define mirf_PAYLOAD    32
#define mirf_CONFIG     ( (1<<EN_CRC) | (1<<CRCO) )
#define RADDR            (uint8_t *)"\xe7\xe7\xe7\xe7\xe7"
#define TADDR            (uint8_t *)"\xe7\xe7\xe7\xe7\xe7"

// Definitions for selecting and enabling MiRF module
#define mirf_CSN_hi     (SPIPORT |= _BV(CSN))
#define mirf_CSN_lo     (SPIPORT &= ~_BV(CSN))
#define mirf_CE_hi      (SPIPORT |= _BV(CE))
#define mirf_CE_lo      (SPIPORT &= ~_BV(CE))

// Public standart functions
extern void mirf_init();
extern void mirf_config();
extern uint8_t mirf_status(void);
extern void mirf_send(uint8_t * value, uint8_t len);
extern void mirf_set_RADDR(uint8_t * adr);
extern void mirf_set_TADDR(uint8_t * adr);
extern uint8_t mirf_data_ready();
extern void mirf_get_data(uint8_t * data);
uint8_t mirf_max_rt_reached(void);

// Public extended functions
extern void mirf_config_register(uint8_t reg, uint8_t value);
extern void mirf_read_register(uint8_t reg, uint8_t * value, uint8_t len);
extern void mirf_write_register(uint8_t reg, uint8_t * value, uint8_t len);
#define MIRF_POWERDOWN mirf_config_register(CONFIG, mirf_CONFIG | ((0<<PWR_UP) | (0<<PRIM_RX) ))
// Flag which denotes transmitting or receiving mode
volatile uint8_t PMODE;
extern volatile uint8_t TxCount;
#endif /* _MIRF_H_ */
