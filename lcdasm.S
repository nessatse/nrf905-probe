#include "defines.h"
#include "asmdelay.h"

#include <avr/io.h>



.macro PINHI p
	sbi _SFR_IO_ADDR(LCD_PORTOUT),\p
.endm
.macro PINLO p
	cbi _SFR_IO_ADDR(LCD_PORTOUT),\p
.endm

    .global init_lcd
	.func init_lcd

init_lcd:
	ldi	r18,LCD_DATABITS | _BV(LCD_E) | _BV(LCD_RS)
	out _SFR_IO_ADDR(LCD_DDR),r18

	PINLO   LCD_RS
	delay_us 40000
	ldi		r24,0x30
	rcall	lcd_wr4
	delay_us 4100
	rcall	lcd_wr4
	delay_us 100
	rcall	lcd_wr4
	ldi		r24,0x20
	rcall	lcd_wr4
	delay_us 100  //???
	// now in 4 bit mode


	ldi		r24,0b00101000		;$28 [0 0 1 DL.N F x x]
	rcall	lcd_instr			;5) Interface data length 4 bits (DL), 2 line (N), Font 5*8 (F)

	ldi	r24,0b00001000		;$08 [0 0 0 0 1 D C B]
	rcall	lcd_instr			;6) Display, Cursor, Blinking OFF

	ldi	r24,0b00000001		;$01 [0 0 0 0.0 0 0 1]
	rcall	lcd_instr			;6) Display clear
	delay_us 2000

	ldi	r24,0b00000110		;$06 [0 0 0 0 0 1 I/D SH]
	rcall	lcd_instr			;6) Entry mode

	ldi	r24,0b00001100		;$0C [0 0 0 0.1 D C B]
	rcall	lcd_instr			;7) Display ON, cursor OFF, blink OFF

	ret

	.endfunc

	.func lcd_pulse_e
lcd_pulse_e:
	PINHI LCD_E
	.if F_CPU_MHZ > 2
	.rept F_CPU_MHZ/2
	nop
	.endr
	.endif
	PINLO LCD_E
	ret
	.endfunc

	.global lcd_wrchar
	.func lcd_wrchar
	; byte in r24
lcd_wrchar:
	cli
	in		r20,_SFR_IO_ADDR(LCD_PORTOUT) ; save PORT contents
	PINHI	LCD_RS
	cbi		_SFR_IO_ADDR(PORTB),LED_CTL
	rcall	lcd_write
	out		_SFR_IO_ADDR(LCD_PORTOUT),r20 ; restore port
	sbi		_SFR_IO_ADDR(PORTB),LED_CTL
	sei
	delay_us	40
	ret

	.endfunc

	.func lcd_write
	; byte in r24
lcd_write:
	mov		r0,r24
	rcall	lcd_wr4
	mov		r24,r0
	swap	r24
	; fall through
	.endfunc

	.func lcd_wr4
lcd_wr4:
	PINHI LCD_E
	andi	r24,0xf0
	in	r25,_SFR_IO_ADDR(LCD_PORTOUT);
	andi	r25,0x0f
	or		r25,r24
	out _SFR_IO_ADDR(LCD_PORTOUT),r25
	rcall lcd_pulse_e
	ret
	.endfunc


	.global lcd_instr
	.func	lcd_instr
lcd_instr:
	cli
	in		r20,_SFR_IO_ADDR(LCD_PORTOUT) ; save PORT contents
	PINLO	LCD_RS
	cbi		_SFR_IO_ADDR(PORTB),LED_CTL
	rcall	lcd_write
	out		_SFR_IO_ADDR(LCD_PORTOUT),r20 ; restore port
	sbi		_SFR_IO_ADDR(PORTB),LED_CTL
	sei
	delay_us	40
	ret
	.endfunc

	.global lcd_ldchars
	.func	lcd_ldchars
lcd_ldchars:
	// set CGRAM address to 0
	ldi		r24,0b01000000
	rcall	lcd_instr
	ldi		ZL,lo8(char_table)
	ldi		ZH,hi8(char_table)
	ldi		r19,64
1:
	LPM		r24,Z+
	rcall	lcd_wrchar
	dec		r19
	brne	1b
	// Clear and home
	ldi 	r24,0x00000001
	rcall	lcd_instr
	delay_us	1520
	ret


	.endfunc
char_table:
.byte 	0b01110,0b10001,0b10001,0b11111,0b11011,0b11011,0b11111,0b00000
.byte   0b00000,0b01010,0b11010,0b01010,0b01011,0b01010,0b00000,0b00000

.byte 	0x1f, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x18
.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x0f
.byte	0x0b, 0x1b, 0x18, 0x18, 0x18, 0x18, 0x1b, 0x0b
.byte	0x08, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1b, 0x0b
.byte	0x10, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x10
//.byte	0x0f, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03
//.byte	0x0b, 0x1b, 0x18, 0x18, 0x18, 0x18, 0x18, 0x08

	.global bignum_map
bignum_map:
.byte	   7,5,4,5		// 0
.byte	   32,5,32,5	// 1
.byte	   6,5,4,1		// 2
.byte	   6,5,2,5		// 3
.byte	   4,5,32,5		// 4
.byte	   3,1,2,5		// 5
.byte	   7,32,3,5		// 6
.byte	   0,5,32,5		// 7
.byte	   3,5,4,5		// 8
.byte	   3,5,2,5		// 9
