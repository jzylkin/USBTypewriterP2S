/*
 * Config_IO.c
 *
 * Created: 12/9/2014 9:39:47 AM
 *  Author: Jack
 */ 

#include "IO_Macros.h"
#include "Config_IO.h"
#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint8_t GlowDirection; //global variable that sets direction of led glow (fade up or down)
	#define BRIGHTEN 0
	#define DIM 1

void Config_IO(){
	
	configure_as_input(SD_DETECT);
	pullup_on(SD_DETECT);
	
	configure_as_input(REED_1);
	pullup_on(REED_1);
	
	configure_as_input(REED_2);
	pullup_on(REED_2);	

	configure_as_input(REED_3);
	pullup_on(REED_3);	
	
	configure_as_input(REED_4);
	pullup_on(REED_4);	
	
	
	set_high(SD_MISO);
	configure_as_input(SD_MISO);
	pullup_on(SD_MISO);
	
	set_high(SD_MOSI);
	configure_as_input(SD_MOSI);
	pullup_on(SD_MOSI);
	
	configure_as_output(SENSE_CLK);
	
	configure_as_input(SENSE_SER);
	pullup_on(SENSE_SER);

	set_high(SD_CLK);
	configure_as_input(SD_CLK);
	pullup_on(SD_CLK);
	
	set_high(SD_CHIP_SELECT);
	configure_as_output(SD_CHIP_SELECT);
	
	configure_as_input(DUMMY_LOAD);
	//no pullup;
	
	set_high(TX);
	configure_as_output(TX);
	
	configure_as_input(RX);
	pullup_on(RX);
	
	configure_as_input(S1);
	pullup_on(S1);
	
	configure_as_input(S2);
	pullup_on(S2);
	
	configure_as_input(S3);
	pullup_on(S3);
	
	set_low(BT_RESET);// bt is off by default
	configure_as_output(BT_RESET); 
	
	set_high(BT_BAUD);
	configure_as_output(BT_BAUD);
	
	configure_as_input(BT_CONNECTED);
	pullup_on(BT_CONNECTED);
	
	set_high(BT_CTS);
	configure_as_output(BT_CTS);
	
	set_high(LED1);
	configure_as_output(LED1);
	
	set_high(LED2);
	configure_as_output(LED2);
	
}

void GlowGreenLED(uint8_t speed){

	//green led is also called the ~OC4A pin
	cli();//disable interrupts;
	OCR4C = 0xFF; //clear tmr4 when reaching this value
	TC4H = 0x00; //clearing this register sets timer4 to 8-bit mode
	OCR4A = 0x08; //when counter reaches this value, it triggers LED.
	bit_set(TCCR4E,OC4OE0);//enable the ~oc4a output
	bit_set(TCCR4A,COM4A0); //set the bit for ~OC4A pin to be active in fast pwm mode
	bit_set(TCCR4A,PWM4A);//activate fast pwm mode
	bit_set(TIMSK4,TOIE4);//enable timer overflow interrupts.
	switch(speed){
		case 0:
			TCCR4B = BIT(CS43)|BIT(CS40);
		case 1:
			bit_set(TCCR4B,CS43);//enable 1:128 prescaler (should make each tick worth about 10khZ).
		break;
		case 2:
			TCCR4B = BIT(CS42)|BIT(CS41)|BIT(CS40);
		break;
		case 3:
			TCCR4B = BIT(CS42)|BIT(CS41);
		break;
		default:
			bit_set(TCCR4B,CS43);
		break;
	}
		
		
	GlowDirection = BRIGHTEN;
	TCNT4 = 0;//clear the timer to 0;
	sei();//enable interrupts again.
}

ISR(TIMER4_OVF_vect){ //called each time timer1 counts up to the OCR1A register (every couple ms)

	uint8_t temp;
	temp = OCR4A;
	if (GlowDirection == BRIGHTEN){
		if(temp==0xFF){
			GlowDirection = DIM;
		}
		else{
			temp++;
		}
	}
	else if (GlowDirection == DIM){
		if(temp == 0x00){
			bit_clr(TCCR4A,COM4A0); //disconnect green led output pin
			TCCR4B = 0;//clear the timer4 register (disable the timer);
			GlowDirection = BRIGHTEN;
		}
		else{
			temp--;
		}
	}
	OCR4A = temp;
}
	





