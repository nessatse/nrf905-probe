#ifndef _LM75_H_
#define _LM75_H_

#include <stdint.h>

#define LM75_BASE_ADDRESS 0x48
#define LM75_ADDRESS(a) ((a)+LM75_BASE_ADDRESS)
/* Pointer register values */
#define LM75_PTR_CONF   0x01
#define LM75_PTR_TEMP   0x00
#define LM75_PTR_TOS    0x03
#define LM75_PTR_THYST  0x02

void lm75_conf(uint8_t addr,unsigned char c);
int16_t lm75_temp(uint8_t addr);

#define lm75_pd(a) lm75_conf((a),1)
#define lm75_pu(a) lm75_conf((a),0)

#define makeint(w,hibyte,lobyte)    \
    __asm__ __volatile__ (          \
        "mov %A0,%2"    "\n\t"      \
        "mov %B0,%1"    "\n\t"      \
        : "=r" (w)                  \
        : "r" (hibyte), "r" (lobyte)\
    )

#endif
