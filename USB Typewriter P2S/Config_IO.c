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
	
	set_low(PIO_5);
	configure_as_output(PIO_5);//PIO_5 is the CONNECT/DISCONNECT button for the BlueCore firmware
	
	configure_as_input(PIO_6);//PIO_6 is the READY signal for the BlueCore's UART to accept a new packet, SUPPOSEDLY.  Should appear every 20ms or so, but DOESNT. -- IF we begin to see activity on this line one day, I guess we should react somehow.
	pullup_on(PIO_6);
	
	set_low(DUMMY_LOAD);
	configure_as_output(DUMMY_LOAD);
	
	configure_as_input(POK);
	pullup_on(POK);
	
	configure_as_input(TX);//tx will start as an input, so that the bluecore programmer can connect for debugging.
	pullup_on(TX);
	
	configure_as_input(RX);
	pullup_on(RX);
	
	configure_as_input(TWI_DAT);
	pullup_on(TWI_DAT);
		
	configure_as_input(TWI_CLK);
	pullup_on(TWI_CLK);

	set_high(SENSE_POWER);
	configure_as_output(SENSE_POWER);
	
	configure_as_input(S1);
	pullup_on(S1);
	
	configure_as_input(S2);
	pullup_on(S2);
	
	configure_as_input(S3);
	pullup_on(S3);
	
	set_high(BT_RESET);
	configure_as_output(BT_RESET);
	
	set_high(LED1);
	configure_as_output(LED1);
	
	set_high(LED2);
	configure_as_output(LED2);
	
}




