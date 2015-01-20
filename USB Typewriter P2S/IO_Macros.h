/*
 * CustomMacros.h
 *
 * Created: 12/8/2014 7:25:02 PM
 *  Author: Jack
 */ 
#include <avr/io.h>

#ifndef CUSTOMMACROS_H_
#define CUSTOMMACROS_H_

/***** Configure IO *****/
#define PIN(x) (*(&x - 2)) // Address Of Data Direction Register Of Port x
#define DDR(x) (*(&x - 1)) // Address Of Input Register Of Port x

#define configure_as_input(bit)             {DDR(bit ## _PORT) &= ~(1 << bit);}
#define configure_as_output(bit)            {DDR(bit ## _PORT) |= (1 << bit);} //ddr register is port+1

#define pullup_on(bit)                      {bit ## _PORT |= (1 << bit);}
#define pullup_off(bit)                     {bit ## _PORT &= ~(1 << bit);} 

#define disable_digital_input(channel)      {DIDR0 |= (1 << channel);}
#define enable_digital_input(channel)       {DIDR0 &= ~(1 << channel);}

#define enable_pin_change_interrupt(pin)    {pin ## _PCMSK |= (1 << pin ## _PCINT);}
#define disable_pin_change_interrupt(pin)   {pin ## _PCMSK &= ~(1<< pin ## _PCINT);}


/***** Manipulate Outputs *****/
#define set_high(bit)                       {bit ## _PORT |= (1 << bit);}
#define set_low(bit)                        {bit ## _PORT &= ~(1 << bit);}


/***** Test Inputs *****/
#define is_high(bit)                        (PIN(bit ## _PORT) & (1 << bit))
#define is_low(bit)                         (!(PIN(bit ## _PORT) & (1 << bit))) // if the pin is 0, it will and to 0, then flip to 1. if it is 1 it will and to 1, then flip to 0


/****REGISTER MANIPULATION MACROS*****/
#define BIT(x) (0x01 << (x))
#define bit_get(p,m) ((p) & BIT(m)) // get a single bit
#define bit_set(p,m) ((p) |= BIT(m)) //set a single bit
#define bit_clr(p,m) ((p) &= ~BIT(m)) //clear a single bit


#define reg_set(p,m) ((p) |= (m)) //register-wide masked set
#define reg_clr(p,m) ((p) &= ~(m)) //register-wide masked clear
#define reg_write(p,m) ((p) = (m))

/****IO DEFINITIONS******/

#define SD_DETECT				PB7
#define SD_DETECT_PORT			PORTB
#define SD_DETECT_DDR			DDRB
#define SD_DETECT_PIN			PINB

#define SENSE_CLK				PB6
#define SENSE_CLK_PORT			PORTB

#define REED_4					PB5
#define REED_4_PORT				PORTB
#define REED_3					PB4
#define REED_3_PORT				PORTB
#define REED_2					PD7
#define REED_2_PORT				PORTD
#define REED_1					PD6
#define REED_1_PORT				PORTD

#define SD_MISO					PB3
#define SD_MISO_PORT			PORTB

#define SD_MOSI					PB2
#define SD_MOSI_PORT			PORTB

#define SD_CLK					PB1
#define SD_CLK_PORT			PORTB

#define PIO_5					PB0
#define PIO_5_PORT				PORTB

#define SENSE_SER				PC6
#define SENSE_SER_PORT			PORTC

#define SENSE_CLR				PC7
#define SENSE_CLR_PORT			PORTC

#define DUMMY_LOAD				PD5
#define DUMMY_LOAD_PORT			PORTD

#define POK						PD4
#define POK_PORT				PORTD

#define TX						PD3
#define TX_PORT					PORTD

#define RX						PD2
#define RX_PORT					PORTD

#define SD_CHIP_SELECT				PD1
#define SD_CHIP_SELECT_PORT			PORTD

#define SD_POWER				PD0
#define SD_POWER_PORT			PORTD

#define PIO_6					PE6
#define PIO_6_PORT				PORTE

#define SENSE_POWER				PE2
#define SENSE_POWER_PORT		PORTE

#define S3						PF7
#define S3_PORT					PORTF
#define S2						PF6
#define S2_PORT					PORTF
#define S1						PF5
#define S1_PORT					PORTF

#define BT_RESET				PF4
#define BT_RESET_PORT			PORTF

#define LED1					PF1
#define LED1_PORT				PORTF

#define LED2					PF0
#define LED2_PORT				PORTF

							



#endif /* CUSTOMMACROS_H_ */