/*
 * CustomMacros.h
 *
 * Created: 12/8/2014 7:25:02 PM
 *  Author: Jack
 */ 
#include <avr/io.h>
#include <stdbool.h>

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
#define set_high(bit)                       bit ## _PORT |= (1 << bit)
#define set_low(bit)                        bit ## _PORT &= ~(1 << bit)

#define toggle(bit)							PIN(bit ## _PORT) |= (1 << bit)


/***** Test Inputs *****/
#define is_high(bit)                        (bool)(PIN(bit ## _PORT) & (1 << bit))
#define is_low(bit)                         (bool)(!(PIN(bit ## _PORT) & (1 << bit))) // if the pin is 0, it will and to 0, then flip to 1. if it is 1 it will and to 1, then flip to 0


/****REGISTER MANIPULATION MACROS*****/
#define BIT(x) (0x01 << (x))
#define LONGLONGBIT(x) ((unsigned long long)0x0000000000000001 << (x))
#define bit_get(p,m) ((p) & BIT(m)) // get a single bit
#define bit_set(p,m) ((p) |= BIT(m)) //set a single bit (in a generic uint)
#define longlongbit_set(p,m) ((p) |= LONGLONGBIT(m)) //set a single bit in a long long type

#define KEYMASK(x) LONGLONGBIT(63-x) //set a specific bit in the key sensor array.

#define bit_clr(p,m) ((p) &= ~BIT(m)) //clear a single bit (generic int)

#define reg_set(p,m) ((p) |= (m)) //register-wide masked set
#define reg_clr(p,m) ((p) &= ~(m)) //register-wide masked clear
#define reg_write(p,m) ((p) = (m))

#define increment(x) if(x+1) x++ //meaning, if the value x does not overflow to zero, it is safe to increment
#define decrement(x) if(x) x-- //if x is not zero, it is ok to decrement it.

/****IO DEFINITIONS******/

#define SD_DETECT				PB0
#define SD_DETECT_PORT			PORTB

#define SENSE_CLK				PE2
#define SENSE_CLK_PORT			PORTE

#define REED_4					PB4
#define REED_4_PORT				PORTB
#define REED_3					PD7
#define REED_3_PORT				PORTD
#define REED_2					PD6
#define REED_2_PORT				PORTD
#define REED_1					PD4
#define REED_1_PORT				PORTD

#define SD_MISO					PB3
#define SD_MISO_PORT			PORTB

#define SD_MOSI					PB2
#define SD_MOSI_PORT			PORTB

#define SD_CLK					PB1
#define SD_CLK_PORT			PORTB

#define SENSE_SER				PB5
#define SENSE_SER_PORT			PORTB

#define DUMMY_LOAD				PD5
#define DUMMY_LOAD_PORT			PORTD

#define TX						PD3
#define TX_PORT					PORTD

#define RX						PD2
#define RX_PORT					PORTD

#define SD_CHIP_SELECT				PD1
#define SD_CHIP_SELECT_PORT			PORTD

#define S3						PF6
#define S3_PORT					PORTF
#define S2						PF5
#define S2_PORT					PORTF
#define S1						PF4
#define S1_PORT					PORTF

#define BT_RESET				PF7
#define BT_RESET_PORT			PORTF

#define BT_BAUD					PC7
#define BT_BAUD_PORT			PORTC

#define BT_CONNECTED			PF1
#define BT_CONNECTED_PORT		PORTF

#define BT_CTS					PE6
#define BT_CTS_PORT				PORTE

#define BT_AT_MODE				PF0
#define BT_AT_MODE_PORT	        PORTF

#define LED1					PC6
#define LED1_PORT				PORTC

#define RED_LED					LED2
#define RED_LED_PORT			LED2_PORT

#define GREEN_LED				LED1
#define GREEN_LED_PORT			LED1_PORT

#define LED2					PB6
#define LED2_PORT				PORTB

#define CTRL_KEY				S1
#define CTRL_KEY_PORT			S1_PORT

#define ALT_KEY					S2
#define ALT_KEY_PORT			S2_PORT

#define CMD_KEY					S3
#define CMD_KEY_PORT			S3_PORT

#define FN_KEY					ALT_KEY
#define FN_KEY_PORT				ALT_KEY_PORT
#define FN_MODIFIER				HID_KEYBOARD_MODIFIER_LEFTALT  //the alt key is used as the fn key.

							



#endif /* CUSTOMMACROS_H_ */