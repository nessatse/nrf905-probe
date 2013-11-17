#include "defines.h"
#include "lcd.h"
#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "version.h"

static char * utoa1(uint16_t n,uint8_t decimals);

volatile uint32_t ticks;

volatile uint8_t buttons_pressed; // = BUTTON_MASK;
volatile uint8_t button_event;

int8_t enc_states[16] PROGMEM = {
    0, //0000
    1, //0001
    -1,//0010
    0, //0011
    -1,//0100
    0, //0101
    0, //0110
    1, //0111
    1, //1000
    0, //1001
    0, //1010
    -1,//1011
    0, //1100
    -1,//1101
    1, //1110
    0  //1111
};

typedef struct {
    uint8_t state;
    volatile uint16_t value;
    uint16_t inc;
    uint16_t inc_fine;
    uint16_t inc_med;
    uint16_t inc_coarse;
    int8_t count;
    uint32_t speed;
    uint32_t last_tick;
} encoder_t;

encoder_t encoders[2];

volatile uint8_t encoder_flags;

char enc1_disp;
char enc2_disp;

volatile uint16_t adc_voltage;
volatile uint16_t adc_current;
volatile uint8_t adc_flags;
// Temp
volatile uint16_t adc_therm;
// Running sums for averaging
uint16_t adc_voltage_sum;
uint16_t adc_current_sum;

enum  { SETMODE = 0, LOCKEDMODE } displaymode_v,displaymode_i;

#define BUTTON_PRESSED(b) (button_event & (b))
#define BUTTON_DOWN(b) (buttons_pressed & (b))
#define BUTTON_UP(b) (~buttons_pressed & (b))
#define BUTTON_CLEAR(b)   (button_event &= ~(b))
#define BUTTONS_CLEAR()	  (button_event &= ~K_ALL)

#define ENCODER_CHANGED(e) (encoder_flags & (e))
#define CLEAR_ENC_FLAG(e) (encoder_flags &= ~(e))

static void check_enc_speed(uint8_t enc_no)
{
    encoders[enc_no].speed = ticks - encoders[enc_no].last_tick;
    encoders[enc_no].last_tick = ticks;
    if (encoders[enc_no].speed < SPEED_MED) {
        if (encoders[enc_no].speed < SPEED_HI) {
            encoders[enc_no].inc = encoders[enc_no].inc_coarse;
        } else {
            encoders[enc_no].inc = encoders[enc_no].inc_med;
        }
    } else {
        encoders[enc_no].inc = encoders[enc_no].inc_fine;
    }
}
static void update_encoder(uint8_t enc_no,uint8_t buttons)
{
    encoders[enc_no].count += (int8_t)pgm_read_byte(&(enc_states[buttons]));
    if (encoders[enc_no].count > 3) {
        check_enc_speed(enc_no);
        if (encoders[enc_no].value < (ENCODERMAX-encoders[enc_no].inc)) {
            encoders[enc_no].value+=encoders[enc_no].inc;
        } else {
            encoders[enc_no].value = ENCODERMAX;
        }
        encoder_flags |= 0x01 << enc_no;
        encoders[enc_no].count = 0;
    } else if (encoders[enc_no].count < -3) {
        check_enc_speed(enc_no);
        if (encoders[enc_no].value >= encoders[enc_no].inc) {
            encoders[enc_no].value-=encoders[enc_no].inc;
        } else {
            encoders[enc_no].value = 0;
        }
        encoder_flags |= 0x01 << enc_no;
        encoders[enc_no].count = 0;
    }
    encoders[enc_no].state = (buttons & 0b0011) << 2;
}

ISR (TIMER0_COMPA_vect)
{
    uint8_t buttons;
    static uint8_t new_button;
    static uint8_t bounce_count;
    static uint8_t button_state;

    ticks++;
    // check buttons
    buttons = ~BUTTON_PORTIN & BUTTON_MASK;
    switch (button_state) {
    case 0:
        if (buttons != buttons_pressed) {
            button_state = 1;
            new_button = buttons;
            bounce_count = BOUNCE_LIMIT;
        }
        break;
    case 1:
        if (buttons == new_button) {
            if (--bounce_count == 0) {
                // Ok this is a valid change.  Now we must detect up/down events, i.e. rising edge
                button_event = buttons ^ buttons_pressed;
                buttons_pressed = buttons;
                button_state = 0;
            }
        } else {
            button_state = 0;
        }
        break;
    }
    // check encoder
    if (displaymode_v == SETMODE) {
        buttons = encoders[0].state | (PINB & (_BV(SW1A)|_BV(SW1B)));
        update_encoder(0,buttons);
    }
    if (displaymode_i == SETMODE) {
        // convoluted logic, but encoder 2 is on portc 2&3
        buttons = encoders[1].state | ((PINC & (_BV(SW2A)|_BV(SW2B))) >> 2);
        update_encoder(1,buttons);
    }
}

ISR (ADC_vect)
{
    static uint8_t conv_cnt;  // count number of conversions for oversamplng
    static uint16_t adc_oversample;


    if (ADMUX == ADC_THERM) {
        adc_flags |= THERM_FLAG;
        adc_therm = ADC;
        ADMUX = ADC_VOLTAGE;
    } else {
        conv_cnt++;
        adc_oversample += ADC;
        if (conv_cnt == 32) {
            adc_oversample >>= 3;  // sample down to 12 bits
            if (ADMUX == ADC_VOLTAGE) {
                adc_voltage = adc_oversample;
                adc_flags |= VOLTAGE_FLAG;
                ADMUX = ADC_CURRENT;  //switch channels
            } else {
                adc_current = adc_oversample;
                adc_flags |= CURRENT_FLAG;
                ADMUX = ADC_THERM;
            }
            adc_oversample = 0;
            conv_cnt = 0;
        }
    }
}

void dac_set(uint16_t value,uint8_t chan)
{
    // ensure SCK is low
    PORTB &= ~_BV(DAC_SCK);
    if (chan)
        // Channel 1, set CS high
        PORTB |= _BV(DAC_SS);
    else
        PORTB &= ~_BV(DAC_SS);
    value |= 0b0111000000000000; // DACA,BUFFERED,GAIN=1,OUTPUT ON
    for (uint8_t i=0; i<16; i++) {
        if (value & 0x8000)
            PORTB |= _BV(DAC_SD);
        else
            PORTB &= ~_BV(DAC_SD);
        PORTB |= _BV(DAC_SCK);
        PORTB &= ~_BV(DAC_SCK);
        value <<= 1;
    }
    // Toggle SS high
    if (chan)
        PORTB &= ~_BV(DAC_SS);
    else
        PORTB |= _BV(DAC_SS);
}

static void adcsetup(void)
{
    ADMUX = 0 | ADC_CURRENT; // Channel 6
    DIDR0 = _BV(ADC0D) | _BV(ADC1D);
    // trigger on timer 0 compare match A
    ADCSRB = _BV(ADTS1) | _BV(ADTS0);
    // Enable, auth trigger enable, interrupt and start conversion,div=128
    ADCSRA = _BV(ADEN) | _BV(ADSC) | _BV(ADATE) | _BV(ADIE) |_BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);
}

static void setup(void)
{
    DDRB = PORTB_DDR;
    PINB = PORTB_PU;
    DDRC = PORTC_DDR;
    PINC = PORTC_PU;
    DDRD = PORTD_DDR;
    PIND = PORTD_PU;

    // timer 0 setup - 1kHz interrupt
    TCCR0A = _BV(WGM01);
    TCCR0B = _BV(CS02);  // clkdiv/256
    OCR0A = T0VALUE;
    TIMSK0 = _BV(OCIE0A);
    lcd_init();

    encoders[0].speed = 0xffff;
    encoders[0].inc = ENC1_FINE;
    encoders[0].inc_fine = ENC1_FINE;
    encoders[0].inc_med = ENC1_MEDIUM;
    encoders[0].inc_coarse = ENC1_COARSE;

    encoders[1].speed = 0xffff;
    encoders[1].inc = ENC2_FINE;
    encoders[1].inc_fine = ENC2_FINE;
    encoders[1].inc_med = ENC2_MEDIUM;
    encoders[1].inc_coarse = ENC2_COARSE;
    enc1_disp = enc2_disp = 'm';

    LED1AON;
    LED1BON;
    LED2AON;
    LED2BON;
    ENABLE_LEDS;

    // Output off
    dac_set(0,IREF);
    dac_set(0,VREF);
    adcsetup();
}
static void disp_set(uint8_t line,uint16_t val,uint8_t dp, char close_tok)
{
    lcd_gotoxy(0,line);
    lcd_puts(utoa1(val,dp));
    lcd_putchar(' ');
    lcd_putchar(close_tok);
    lcd_putchar(' ');
}
static void disp_vset(void)
{
    disp_set(0,encoders[0].value,2,CHAR_ADJ);
}
static void disp_iset(void)
{
    disp_set(1,encoders[1].value,3,CHAR_ADJ);
}
static void disp_vlock(void)
{
    disp_set(0,encoders[0].value,2,CHAR_LOCK);
}
static void disp_ilock(void)
{
    disp_set(1,encoders[1].value,3,CHAR_LOCK);
}
static void checkbuttons(void)
{
    if (BUTTON_PRESSED(BUTTON_SW1PB)) {
        if (BUTTON_UP(BUTTON_SW1PB)) {
            switch (displaymode_v) {
            case SETMODE:
                displaymode_v = LOCKEDMODE;
                LED1AOFF;
                LED1BON;
                disp_vlock();
                break;
            case LOCKEDMODE:
                displaymode_v = SETMODE;
                enc1_disp = 'f';
                LED1AON;
                LED1BON;
                disp_vset();
                break;
            }
            BUTTON_CLEAR(BUTTON_SW1PB);
        }
    }
    if (BUTTON_PRESSED(BUTTON_SW2PB)) {
        if (BUTTON_UP(BUTTON_SW2PB)) {
            switch (displaymode_i) {
            case SETMODE:
                displaymode_i = LOCKEDMODE;
                LED2AOFF;
                LED2BON;
                disp_ilock();
                break;
            case LOCKEDMODE:
                displaymode_i = SETMODE;
                enc2_disp = 'f';
                LED2AON;
                LED2BON;
                disp_iset();
                break;
            }
            BUTTON_CLEAR(BUTTON_SW2PB);
        }
    }
}
// utoa1 only valid for 0-9999
// NB - no error checking/validation!!
static char * utoa1(uint16_t n,uint8_t decimals)
{
    static char ibuf[6] ;
    char *b = &ibuf[5];

    char * dp = &ibuf[5-decimals];
    if (n) {
        while (n) {
//			div_t d = div(n,10);
//			*--b = (char)((d.rem)+'0');
//			n = d.quot;
            *--b = (char)((n % 10) +'0');
            n /= 10;
            if (decimals && b == dp) {
                *--b = '.';
            }
        }
    } else {
        *--b = '0';
    }
    if (decimals) {
        if (b > dp) {
            while (b > dp)
                *--b = '0';
            *--b = '.';
        }
        if (b == (dp - 1))   // Leading 0 if required
            *--b = '0';
    }
    while (b > ibuf) {
        *--b = ' ';
    }
    return b;
}

static inline uint16_t encodertorange(uint16_t val)
{
    return (val * 4095UL)/3000;
}

static void checkencoders(void)
{
    if (displaymode_v == SETMODE) {
        if (ENCODER_CHANGED(ENC1)) {
            CLEAR_ENC_FLAG(ENC1);
        }
        disp_vset();
        dac_set(encodertorange(encoders[0].value),VREF);
    }
    if (displaymode_i == SETMODE) {
        if (ENCODER_CHANGED(ENC2)) {
            CLEAR_ENC_FLAG(ENC2);
        }
        disp_iset();
        dac_set(encodertorange(encoders[1].value),IREF);
    }
}
static uint16_t adctoreal(uint16_t adc)
{
    // converts 12 bit (0-4095) to (0-3000) with adc offset and gain correction
    return (( (((adc + ADC_V_OFF)*ADC_V_GAIN_NUM)/ADC_V_GAIN_DEN) * 3000UL)/4095);
}

static uint16_t adc_avg(uint16_t adc,uint16_t *adcsum)
{
    *adcsum = *adcsum-(*adcsum>>4)+adc;
    return (*adcsum >> 4);
}

static void check_adc(void)
{
    if (adc_flags & VOLTAGE_FLAG) {
        adc_flags &= ~VOLTAGE_FLAG;
        // Average
        adc_voltage = adc_avg(adc_voltage,&adc_voltage_sum);
        // convert and display voltage
        lcd_gotoxy(9,2);
        lcd_puts(utoa1(adctoreal(adc_voltage),2));
        lcd_putchar(' ');
        lcd_putchar('V');
    }
    if (adc_flags & CURRENT_FLAG) {
        adc_flags &= ~CURRENT_FLAG;
        adc_current = adc_avg(adc_current,&adc_current_sum);
        lcd_gotoxy(9,1);
        lcd_puts(utoa1(adctoreal(adc_current),3));
        lcd_putchar(' ');
        lcd_putchar('A');
    }
    /*   if (adc_flags & THERM_FLAG) {
           adc_flags &= ~THERM_FLAG;
           lcd_gotoxy(9,1);
           lcd_puts(utoa1(adc_therm,0));
           lcd_putchar(' ');
       }*/
}
/*
static void leds(uint8_t l)
{
	(l & 1) ? LED1AON : LED1AOFF;
	(l & 2) ? LED1BON : LED1BOFF;
	(l & 4) ? LED2AON : LED2AOFF;
	(l & 8) ? LED2BON : LED2BOFF;
}*/


static void banner(void)
{
    lcd_home();
    lcd_puts("EDS PSU V");
    lcd_puts(FULLVERSION_STRING);
    while (ticks < 5000);
    lcd_clear();
}
int main(void)
{
    setup();
    disp_vset();
    banner();
    disp_iset();
    set_sleep_mode(SLEEP_MODE_IDLE);
    while (1) {
        sleep_enable();
        sei();
        sleep_cpu();
        sleep_disable();
        checkbuttons();
        checkencoders();
        check_adc();
    }
}
