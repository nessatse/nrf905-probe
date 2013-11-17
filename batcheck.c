#include <stdint.h>
#include <avr/io.h>
#include "batcheck.h"

void setup_batcheck(void)
{
    // disable digital inputs to save
    DIDR = _BV(AIN1D) | _BV(AIN0D);
    // Disable comparator
    ACSR = _BV(ACD);
}
uint8_t check_battery(void)
{
    uint8_t bat_state;
    DDRB |= _BV(BATCHK_GND);
    // Enable comparator
    ACSR = 0x00;
    PORTB &= ~_BV(BATCHK_GND);
    // Comp output will be high if battery is ok
    bat_state = ACSR & _BV(ACO);
    // Disable comparator
    ACSR = _BV(ACD);
    DDRB &= ~_BV(BATCHK_GND);
    return bat_state;
}
