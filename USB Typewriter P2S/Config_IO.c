/*
 * Config_IO.c
 *
 * Created: 12/9/2014 9:39:47 AM
 *  Author: Jack
 */ 

#include "IO_Macros.h"
#include "Config_IO.h"
#include <avr/io.h>

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

	configure_as_input(SD_MISO);
	pullup_on(SD_MISO);
	
	configure_as_input(SD_MOSI);
	pullup_on(SD_MOSI);
	
	configure_as_output(SENSE_CLK);
	
	configure_as_input(SENSE_SER);
	pullup_on(SENSE_SER);

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
	
	set_high(LED1);
	configure_as_output(LED1);
	
	set_high(LED2);
	configure_as_output(LED2);
	
}




