#include <avr/io.h>

/* CPU frequency */
#define F_CPU_MHZ 20

#define LCD_PORT D
#define LCD_RS 2
//#define LCD_RW 1
#define LCD_E  3
/* The data bits have to be in ascending order. */
#define LCD_D4 4
#define LCD_D5 5
#define LCD_D6 6
#define LCD_D7 7

#define GLUE(a, b)     a##b
#define PORT(x)        GLUE(PORT, x)
#define PIN(x)         GLUE(PIN, x)
#define DDR(x)         GLUE(DDR, x)

#define LCD_PORTOUT    PORT(LCD_PORT)
#define LCD_PORTIN     PIN(LCD_PORT)
#define LCD_DDR        DDR(LCD_PORT)

#define LCD_DATABITS (_BV(LCD_D4)|_BV(LCD_D5)|_BV(LCD_D6)|_BV(LCD_D7))

#define BUTTON_PORTIN	PINC
#define BUTTON_PORTOUT	PORTC
#define BUTTON_PORTDDR	DDRC

#define SW1PB	PC4
#define SW2PB	PC5

#define BUTTON_MASK (_BV(SW1PB)|_BV(SW2PB))
#define BUTTON_SW1PB _BV(SW1PB)
#define BUTTON_SW2PB _BV(SW2PB)
#define K_ANY	BUTTON_MASK
#define K_ALL	BUTTON_MASK
// ticks for debounce
#define BOUNCE_LIMIT 20

#define SW1A	PB0
#define SW1B	PB1
#define SW2A	PC2
#define SW2B	PC3
/* UART baud rate */
#define UART_BAUD  9600

#define LED_PORTIN	PIND
#define LED_PORTOUT	PORTD
#define LED_PORTDDR	DDRD

#define LED_CTL	PB4

#define LED1A	PD7
#define LED1B	PD6
#define LED2A	PD5
#define LED2B	PD4

#define LED1AON	(PORTD |= _BV(LED1A))
#define LED1BON	(PORTD |= _BV(LED1B))
#define LED2AON	(PORTD |= _BV(LED2A))
#define LED2BON	(PORTD |= _BV(LED2B))
#define LED1AOFF	(PORTD &= ~_BV(LED1A))
#define LED1BOFF	(PORTD &= ~_BV(LED1B))
#define LED2AOFF	(PORTD &= ~_BV(LED2A))
#define LED2BOFF	(PORTD &= ~_BV(LED2B))
#define LED1ATOGGLE	(PIND |= _BV(LED1A))

#define DISABLE_LEDS	(PORTB &= ~_BV(LED_CTL))
#define ENABLE_LEDS		(PORTB |= _BV(LED_CTL))

// ADC Channels
#define ADC_THERM	1
#define ADC_VOLTAGE 7
// Note hack for current channel since I fucked up the ADC6 pin
#define ADC_CURRENT 0
#define VOLTAGE_FLAG 1
#define CURRENT_FLAG 2
#define THERM_FLAG	 4
// Non ideal ADC parameters
#define ADC_V_OFF	13
#define ADC_V_GAIN_NUM	9976UL
#define ADC_V_GAIN_DEN	10000
// DAC Channels
#define VREF 1
#define IREF 0
// DAC Pins
#define DAC_SD	PB3
#define DAC_SCK PB5
#define DAC_SS  PB2

// Tick period in us
#define TICKTIME	1000
#define T0VALUE	0x4D

// PORT Setups
#define PORTB_DDR	(_BV(DAC_SD)|_BV(DAC_SCK)|_BV(DAC_SS)|_BV(LED_CTL))
#define PORTB_PU	(_BV(SW1A)|_BV(SW1B))

#define PORTC_DDR	(0)
#define PORTC_PU	(_BV(SW2A)|_BV(SW2B)|_BV(SW1PB)|_BV(SW2PB))

#define PORTD_DDR	(_BV(LCD_D4)|_BV(LCD_D5)|_BV(LCD_D6)|_BV(LCD_D7)|_BV(LCD_E)|_BV(LCD_RS))
#define PORTD_PU	(0)

// Encoder limits and increments
// 4 transitions for i count
#define ENDCODERSCALE 4
#define ENCODERMAX	(3000)
#define ENC1_FINE	1
#define ENC1_MEDIUM	10
#define ENC1_COARSE	50
#define ENC2_FINE	10
#define ENC2_MEDIUM	20
#define ENC2_COARSE	50
// Endcoder speed limits
// Speed is actually number of ticks between clicks
#define SPEED_LOW   (TICKTIME/2)
#define SPEED_MED   (TICKTIME/20)
#define SPEED_HI   (TICKTIME/50)
// Change flags for encoders
#define ENC1 0b01
#define ENC2 0b10



