/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <joerg@FreeBSD.ORG> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.        Joerg Wunsch
 * ----------------------------------------------------------------------------
 *
 * Stdio demo, upper layer of LCD driver.
 *
 * $Id: lcd.c,v 1.1 2005/12/28 21:38:59 joerg_wunsch Exp $
 */

#include "defines.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <avr/io.h>
#include <avr/pgmspace.h>

#include <util/delay.h>

#include "lcd.h"

//extern uint8_t PROGMEM bignum_map[];
extern prog_char bignum_map[];
/*
 * Setup the LCD controller.  First, call the hardware initialization
 * function, then adjust the display attributes we want.
 */
void lcd_init(void)
{

    init_lcd();

    /*
     * Clear the display.
     */
    lcd_instr(LCD_CMD_CLR);
    _delay_us(2000);

    /*
     * Entry mode: auto-increment address counter, no display shift in
     * effect.
     */
    lcd_instr(LCD_CMD_ENTMODE | LCD_ENTMODE_ID);

    /*
     * Enable display, activate non-blinking cursor.
     */
    lcd_instr(LCD_CMD_DISPCTL | LCD_DTSPCTL_ON);
    lcd_instr(LCD_CMD_DDRAM | 0);
    lcd_ldchars();
}

void lcd_bignum(uint8_t x,uint8_t n)
{
    PGM_P p = bignum_map + (n << 2);
    lcd_gotoxy(x,0);
    lcd_wrchar(pgm_read_byte (p++));
    lcd_wrchar(pgm_read_byte (p++));
    lcd_gotoxy(x,1);
    lcd_wrchar(pgm_read_byte (p++));
    lcd_wrchar(pgm_read_byte (p));
}
/*
 * Send character c to the LCD display.
 */

void lcd_puts(const char *str)
{
    while (*str) {
        lcd_putchar(*str++);
    }
}
void lcd_puts_P(PGM_P str)
{
    char c;
    while ((c = pgm_read_byte (str++))) {
        lcd_putchar(c);
    }
}
void lcd_home(void)
{
    lcd_instr(LCD_CMD_DDRAM | 0);
}
/*
inline void lcd_cursor_on(void)
{
	lcd_instr(LCD_CMD_DISPCTL | LCD_DTSPCTL_ON | LCD_DTSPCTL_CUR); //| LCD_DTSPCTL_BLK);
}
inline void lcd_cursor_off(void)
{
	lcd_instr(LCD_CMD_DISPCTL | LCD_DTSPCTL_ON );
}*/


void lcd_clear(void)
{
    lcd_instr(LCD_CMD_CLR);
    _delay_us(2000);
}

void lcd_gotoxy(uint8_t x,uint8_t y)
{
    lcd_instr(LCD_CMD_DDRAM | (x + (y << 6)));
}
