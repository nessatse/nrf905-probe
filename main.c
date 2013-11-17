#include <stdint.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "USI_TWI_Master.h"
#include "uart.h"
#include "lm75.h"
#include "spi.h"
#include "nrf905.h"
#include "packet.h"
#include "wd_timer.h"
#include "batcheck.h"
#include "debug.h"

//#define TESTING

#define UNIT_ID 1

// number of WD timeoute per measuring cycle
#define WDCYCLES 1
// Number or cycles for battery check
#define BATCYCLES 4
uint8_t batcycles;

// Use our own WD constants for easy OR'ing
#define WDT_16MS    0b00000000
#define WDT_32MS    0b00000001
#define WDT_64MS    0b00000010
#define WDT_125MS   0b00000011
#define WDT_250MS   0b00000100
#define WDT_500MS   0b00000101
#define WDT_1SEC    0b00000110
#define WDT_2SEC    0b00000111
#define WDT_4SEC    0b00100000
#define WDT_8SEC    0b00100001

uint8_t buffer[nrf_PAYLOAD];

struct _rpt_pkt {
    pkt_hdr_t   hdr;
    tv_lm75_t   temp;
    tv_bat_t    bat;
} __attribute__((packed)) *rpt_pkt = buffer;

#ifdef TESTING
char test[] = "  Hello World abcdefghijklmnopq";
#endif

uint8_t sensors_cnt;
uint8_t sensors[8];

void scansensors(void)
{
    for (uint8_t a=0; a<8; ++a) {
        lm75_pu(LM75_ADDRESS(a));
        DBG_STR("Scanning:");
        DBG_PUTHEX(a);
        if (USI_TWI_Get_State_Info() == USI_TWI_NO_ACK_ON_ADDRESS) {
            DBG_STR(" Fail\r\n");
        } else {
            DBG_STR(" Ok\r\n");
            sensors[sensors_cnt++] = a;
        }
    }
    DBG_PUTHEX(sensors_cnt);
    DBG_STR(" sensors detected\r\n");
}
int main(void)
{
    int16_t temperature;

    // Make sure WD is not fucking us around
    MCUSR &= ~(1<<WDRF);
    WDTCSR |= (1<<WDCE) | (1<<WDE);
    WDTCSR = 0x00;

    setup_batcheck();
    batcycles = BATCYCLES;
    DBG_INIT();
    spi_init();
    USI_TWI_Master_Initialise();
    scansensors();
    lm75_pu(LM75_ADDRESS(1));
    nrf_init();
    // Wait for mirf to come up
    _delay_ms(50);
    // Configure mirf
    nrf_config();
    nrf_tx_addr(0xe7e7e7e7);
#ifdef TESTING
    DBG_STR("Start\r\n");
#else
    wd_timer_init(WDT_4SEC);
    sei();
#endif
    // prepare packet
    rpt_pkt->hdr.pktlen = sizeof(struct _rpt_pkt);
    rpt_pkt->hdr.unitid = UNIT_ID;
    rpt_pkt->hdr.pktid = 0;
    rpt_pkt->temp.id=SENS_LM75;
    rpt_pkt->bat.id=SENS_BAT;

#ifdef TESTING
    char c = '0';
    while(1) {
        temperature = lm75_temp(LM75_ADDRESS(1));
        test[0] = c;
        c++;
        if (c==('9'+1)) c='0';
        nrf_tx_addr(0xf0f0f0f0);
        nrf_send((uint8_t *)test,sizeof(test));
        DBG_PUTHEX(nrf_status());
        for (uint8_t i=0; i<10; i++) {
            DBG_PUTCH(' ');
            DBG_PUTHEX(nrf_rd_config(i));
        }
        DBG_PUTCH(' ');
        DBG_PUTHEX(nrf_status());
        DBG_STR("\r\n");
        _delay_ms(500);
        _delay_ms(500);
    }
#else
    while(1) {
        rpt_pkt->hdr.pktlen = sizeof(pkt_hdr_t);
        tv_lm75_t *p_pkt = (tv_lm75_t *)rpt_pkt->hdr.data;
        for (uint8_t s=0; s<sensors_cnt; s++) {
            p_pkt->value = lm75_temp(LM75_ADDRESS(sensors[s]));
            p_pkt->id=SENS_LM75;
            p_pkt++;
            rpt_pkt->hdr.pktlen += sizeof(tv_lm75_t);
        }
        if (--batcycles == 0) {
            batcycles = BATCYCLES;
            ((tv_bat_t *)p_pkt)->id = SENS_BAT;
            ((tv_bat_t *)p_pkt)->value = check_battery();
            rpt_pkt->hdr.pktlen += sizeof(tv_bat_t);
        }
        nrf_tx_addr(0xf0f0f0f0);

        nrf_send((uint8_t *)buffer,32); //sizeof(struct _rpt_pkt));

        DBG_PUTHEX(nrf_status());
        DBG_PUTCH(' ');
        DBG_PUTHEX(wd_total_ticks);
        DBG_PUTCH(' ');
        DBG_PUTHEX(nrf_int);
        DBG_STR(" Temp=");
        DBG_PUTHEX(temperature);
        /*        for (uint8_t i=0; i<10; i++) {
                    DBG_PUTCH(' ');
                    DBG_PUTHEX(nrf_rd_config(i));
                }*/
        DBG_PUTCH(' ');
        DBG_PUTHEX(sizeof(struct _rpt_pkt));
        DBG_PUTCH(' ');
        DBG_PUTHEX(nrf_status());
        DBG_STR("\r\n");
        DBG_TXDONE();
        _delay_ms(1);

        rpt_pkt->hdr.pktid++;
        //        rpt_pkt->hdr.pktid = TxCount;
        wd_timer_init(WDT_4SEC);
        set_sleep_mode(SLEEP_MODE_PWR_DOWN);
        //         power down peripherals
        lm75_pd(LM75_ADDRESS(1));
        while (wd_ticks < WDCYCLES) {
            sleep_mode();
        }
        // power up LM75's
        lm75_pu(LM75_ADDRESS(1));
        // Sleep for another 500ms to allow lm75 to sample
        wd_timer_init(WDT_125MS);
        sleep_mode();
        // wake up nrf, it needs 3 ms
        nrf_powerup();
        wd_timer_init(WDT_16MS);
        sleep_mode();
        // Reset WD normal delay
        wd_timer_init(WDT_8SEC);
        wd_ticks = 0;
    }
#endif
    return 0;
}

