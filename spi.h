#ifndef __SPI_H__
#define __SPI_H__
#include <stdint.h>

void spi_init(void);
uint8_t spi_rw(uint8_t c);
void spi_wr(uint8_t c);
uint8_t spi_rd(void);
void spi_wr_buf(uint8_t *buf,uint8_t len);
void spi_rd_buf(uint8_t *buf,uint8_t len);

#define SPIPORT PORTD
#define SPIDDR  DDRD
#define SPIPIN  PIND

#define MOSI    PD5
#define MISO    PD4
#define SCK     PA1

#define SCLK_HI  (PORTA |= _BV(SCK))
#define SCLK_LO  (PORTA &= ~_BV(SCK))
#define MOSI_HI  (PORTD |= _BV(MOSI))
#define MOSI_LO  (PORTD &= ~_BV(MOSI))

#define MISO_IN (PIND & _BV(MISO))

#endif
