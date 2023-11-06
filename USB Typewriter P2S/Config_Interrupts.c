/*
 * Config_Interrupts.c
 *
 * Created: 12/9/2014 11:30:04 AM
 *  Author: Jack
 */ 
#include <avr/interrupt.h>
#include "Keyboard.h"
#include "IO_Macros.h"

#define TIMER1_PERIOD_US 10000UL //10ms period for TIMER1 -- max is 65535

#define TIMER1_COMPARE TIMER1_PERIOD_US  // each clock tick is 1 us, if prescaler is 8 and system clock is 8MHZ
#define TIMER1_COMPARE_LOW TIMER1_COMPARE //low byte of comparison register
#define TIMER1_COMPARE_HIGH TIMER1_COMPARE>>8 //high byte of comparison register

#define CTC1 WGM12 //this bit has two names in the datasheet


void Config_Interrupts(){

	
	bit_clr(PRR0,PRTIM1); // clear power-reduction bit for timer1
	TCCR1B = BIT(CTC1) | BIT(CS11); //set CTC (clear timer on compare equal mode) and set tmr prescaler to 8 -- page 125 of datasheet
	
	OCR1AH = TIMER1_COMPARE_HIGH;//high register MUST be written before low register.  Datasheet says so!
	OCR1AL = UINT8_C(TIMER1_COMPARE_LOW); //we only want the first 8 bits of the "low" variable.
	
	bit_set(TIMSK1,OCIE1A); //enable output compare interrupt for timer1
	
}




	



