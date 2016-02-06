/*
 * Bluetooth.c
 *
 * Created: 2/12/2015 11:09:37 PM
 *  Author: Jack
 */ 
#include "Keyboard.h"
#include "Bluetooth.h"
#include "globals.h"
#include "KeyCodes.h"
#include <util/delay.h>

#define response_buffer_len 30
char response[response_buffer_len]; //array to store bluetooth module's responses to commands


/**
 * Bluetooth Send -- send keystrokes to bluetooth module using "shorthand" protocol
 * 
 * \param key 
 * \param modifier
 * 
 * \return void
 */
void Bluetooth_Send(uint8_t key, uint8_t modifier){
	
//	set_high(BT_CTS);//toggle cts to wake module from deep sleep.
//	_delay_us(100);

	set_low(BT_CTS);
	Delay_MS(10);//5 ms is worst-case wakeup time
	
		if (key&FORCE_UPPER){ //in this program, we use the MSB of code to indicate that this key MUST be sent as upper case.
			reg_clr(key,FORCE_UPPER); //clear the MSB,
			modifier = UPPER; //and set the modifier to upper case.
		}
		
		if(key == KEY_EXECUTE){ // the "execute" command is for posting emails -- it actually sends a "CTRL+ENTER" command.
			key = KEY_ENTER;
			modifier = HID_KEYBOARD_MODIFIER_LEFTCTRL;
		}
		
		
	uart_putc(0xFE);//report is a "keyboard shorthand" report
    uart_putc(0x02);//1 key is sent at a time
	uart_putc(modifier);//modifier goes here
	uart_putc(key);


	//clear the keystroke
//	Delay_MS(10);
	
	uart_putc(0xFE);
	uart_putc(0x00);//send shorthand report length zero
	
//	set_high(BT_CTS); //enable deep sleep
	
}

/**
 * Initialize Bluetooth 
 * reset, enter command mode, set parameters, reset again to lock in parameters.
 * 
 * \return void
 */
void Bluetooth_Init(){

	Bluetooth_Reset(); //reset the module
	
}

bool Bluetooth_Configure(){
	bool cmdmode;
	uint8_t attempt_count = 0;
	bool success;
	success = true;
	
	if(!Bluetooth_Send_CMD("AT+NM")){ // read in friendly name;
		//if the response is not received, BT module is not present, or there is an error.
		Typewriter_Mode = PANIC_MODE;
		return false;//tell calling function that BT Module is not present/functional
	}

	Bluetooth_Send_CMD("AT+NM=USBTYPEWRITER"); //set friendly name
	Bluetooth_Send_CMD("AT+PF=00,01,00");//set hid parameters
	Delay_MS(BLUETOOTH_RESET_DELAY);
	while(!uart_check_rx){} // wait for something to be received.
	success &= Get_Response(); //get OK! response from module, hopefully
	success &= Bluetooth_Send_CMD("AT+FT=FF,01,FF,05,01,0258\r\n"); //configure module features (see manual):
	/*enable the auto connection after power on as permanent mode;
	enable the auto connect after paired; -- was disabled by default.
	enable auto reconnect after link lost as permanent mode;
	set the interval of auto reconnect to 5s.
	configure the discover mode as 01: auto discoverable when empty.
	configure the timeout of discoverable as 600 seconds (10 min).
	This command is only needed when the first time use this Bluetooth module.*/
	
	return success; //if any of the commands failed, success will be false.
}


/**
 * Bluetooth Send
 * 
 * \param command -- string containing formatted command to send to bluetooth module
 * 
 * \return void
 */
bool Bluetooth_Send_CMD(char* command){
	
	int i = 0;
	
	uart_clear_rx_buffer();
	
	while (command[i] != '\0'){//loop until end of string
		uart_putc(command[i]); //send first character of command
		i++;
	}
	uart_putc('\r\n'); //send return carriage
	
	return Get_Response();

	}
	
void Bluetooth_Reset(){
	set_low(BT_RESET);//reset the bluetooth module
	Delay_MS(50); //hold in reset for 50ms
	set_high(BT_RESET); //reactivate bluetooth module
	Delay_MS(BLUETOOTH_RESET_DELAY);//takes 500ms from power on for module to be able to receive commands.
	
	set_low(BT_CTS);//this wakes the buetooth module
	Delay_MS(5);//it takes 5ms to wake up from low-power state
	
}


bool Get_Response(){
		
		Delay_MS(BLUETOOTH_RESPONSE_DELAY); //wait for the response to be sent.
		response[0] = '\0'; //clear response string
		response[1] = '\0';
		response[2] = '\0';
		for(uint8_t i=0; i<response_buffer_len; i++)
			if (response[i] & 0xFF00 != 0){ //check upper byte of getc for error code -- most commonly this would be "buffer empty"
				response[i]='\0'; //mark the string as having ended.
				break; // reception is complete -- no more characters to retrieve.
			}
			response[i] = uart_getc() & 0xFF; //store lower byte of getc as a uart character received
		}
				
		if (((response[0] == 'O')&&(response[1] == 'K'))||(response[2] == '=')){  //if response is "OK" or "XX=...", bt module has received command successfully
			return true;
		}
		else{
			return false;
		}
}

bool BluetoothInquire(){
	bool success = true;
	success &= Bluetooth_Send_CMD("AT+CP"); //clear the paired device list.  This makes bluetooth enter discoverable state by default.
	return success;
}



