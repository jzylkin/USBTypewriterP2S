/*
 * Init_Mode.c
 *
 * Created: 12/9/2014 1:45:29 PM
 *  Author: Jack
 */ 

#include "Init_Mode.h"
#include "IO_Macros.h"

extern int Typewriter_Mode;

void Init_Mode(){
	if (is_low(S1)&&is_high(S2)&&is_high(S3)){ //hold down S1 during initialization to calibrate
		Typewriter_Mode = CAL_MODE;
	}
	else if (is_high(S1)&&is_high(S2)&&is_low(S3)){ //hold down S3 to enter LED indication mode to test reed switches.
		Typewriter_Mode = TEST_MODE;
	}
	else{
		Typewriter_Mode = USB_MODE; //otherwise just go into normal USB mode.
		set_high(BT_RESET);//and turn off the bluetooth module.
	}
}
