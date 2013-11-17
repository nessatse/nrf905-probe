#ifndef _WD_TIMER_H_
#define _WD_TIMER_H_

#include <stdint.h>
#include <avr/wdt.h>

void wd_timer_init(uint8_t value);
extern volatile uint16_t wd_ticks;
extern volatile uint8_t wd_total_ticks;


#endif // _WD_TIMER_H_
