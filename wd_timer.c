#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include "wd_timer.h"

volatile uint16_t wd_ticks;
volatile uint8_t wd_total_ticks;

void wd_timer_init(uint8_t value)
{
    value |= (1<<WDIF)|(1<<WDIE)|(1<<WDCE )|(0<<WDE);
    /* Setup Watchdog */
    wdt_reset();
    // Use Timed Sequence for disabling Watchdog System Reset Mode if it has been enabled unintentionally.
    MCUSR  &=  ~(1<<WDRF);                                 // Clear WDRF if it has been unintentionally set.
    WDTCSR   =   (1<<WDCE )|(1<<WDE  );                     // Enable configuration change.
    WDTCSR   = value;
 //   WDTCSR   =   (1<<WDIF)|(1<<WDIE)|                     // Enable Watchdog Interrupt Mode.
 //               (1<<WDCE )|(0<<WDE  )|                     // Disable Watchdog System Reset Mode if unintentionally enabled.
 //               //value;
 //               (1<<WDP3 )|(0<<WDP2 )|(0<<WDP1)|(0<<WDP0); // Set Watchdog Timeout period to 4.0 sec.

    /* Enable Power Down Sleep Mode */
    MCUCR |= (1<<SM1) | (1<<SE);
}

ISR (WDT_OVERFLOW_vect)
{
    wd_ticks++;
    wd_total_ticks++;
}
