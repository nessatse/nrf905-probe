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
 * $Id: lcd.h,v 1.1 2005/12/28 21:38:59 joerg_wunsch Exp $
 */
#ifndef _LCD_H_
#define _LCD_H_

#include <avr/pgmspace.h>

#define LCD_CMD_CLR 0x01
#define LCD_CMD_HOME 0x02
#define LCD_CMD_ENTMODE 0x04
#	define LCD_ENTMODE_ID	0x02
#	define LCD_ENTMODE_S	0x01
#define LCD_CMD_DISPCTL 0x08
#	define LCD_DTSPCTL_ON	0x04
#	define LCD_DTSPCTL_CUR	0x02
#	define LCD_DTSPCTL_BLK	0x01
#define LCD_CMD_SHIFT   0x10
#	define LCD_SHIFT_SC	0x08
#	define LCD_SHIFT_RL	0x04
#define LCD_CMD_FNSET	0x20
#	define LCD_FNSET_DL	0x10
#	define LCD_FNSET_N	0x08
#	define LCD_FNSET_F	0x04
#define LCD_CMD_CGRAM	0x40
#define LCD_CMD_DDRAM	0x80

// Custom Characters
#define CHAR_LOCK   0x00
#define CHAR_ADJ    0x01
/*
 * Initialize LCD controller.  Performs a software reset.
 */
void	lcd_init(void);

// from assembler file
void lcd_wrchar(char c);
void lcd_instr(uint8_t c);
void	init_lcd(void);
void lcd_ldchars(void);
/*
 * Send one character to the LCD.
 */
inline  void lcd_putchar(char c)
{
    lcd_wrchar(c);
};
void lcd_puts(const char *str);
void lcd_puts_P(PGM_P str);
void lcd_home(void);
void lcd_clear(void);
void lcd_gotoxy(uint8_t x,uint8_t y);
inline void lcd_cursor_on(void)
{
    lcd_instr(LCD_CMD_DISPCTL | LCD_DTSPCTL_ON | LCD_DTSPCTL_CUR); //| LCD_DTSPCTL_BLK);
};
inline void lcd_cursor_off(void)
{
    lcd_instr(LCD_CMD_DISPCTL | LCD_DTSPCTL_ON );
};
void lcd_bignum(uint8_t x,uint8_t n);

#endif
