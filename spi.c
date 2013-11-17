#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include "spi.h"



void spi_init(void)
{
    // SCK,MOSI low
    MOSI_LO;
    SCLK_LO;
    // SCK,MOSI output, MISO input
    DDRD |= (_BV(MOSI));
    DDRD &= ~(_BV(MISO));
    DDRA |= (_BV(SCK));
}

uint8_t spi_rw(uint8_t c)
{
    uint8_t cin;
    SCLK_LO;
    for (uint8_t i=0; i<8; i++) {
        if (c & 0x80)
            MOSI_HI;
        else
            MOSI_LO;
        c <<= 1;
        SCLK_HI;
        cin <<= 1;
        if (MISO_IN)
            cin |= 0x01;
        SCLK_LO;
    }
    return cin;
}

void spi_wr(uint8_t c)
{
    SCLK_LO;
    for (uint8_t i=0; i<8; i++) {
        if (c & 0x80)
            MOSI_HI;
        else
            MOSI_LO;
        c <<= 1;
        SCLK_HI;
        SCLK_LO;
    }
}

uint8_t spi_rd(void)
{
    uint8_t cin;
    SCLK_LO;
    MOSI_HI;
    for (uint8_t i=0; i<8; i++) {
        cin <<= 1;
        SCLK_HI;
        if (MISO_IN)
            cin |= 0x01;
        SCLK_LO;
    }
    return cin;
}

void spi_wr_buf(uint8_t *buf,uint8_t len)
{
    while (len--) {
        spi_rw(*buf++);
    }
}

void spi_rd_buf(uint8_t *buf,uint8_t len)
{
    while (len--) {
        *buf++ = spi_rd();
    }
}
