/*
 * Bluetooth.c
 *
 * Created: 2/12/2015 11:09:37 PM
 *  Author: Jack
 */ 
#include "Keyboard.h"
#include "Bluetooth.h"
#include "globals.h"
#include <util/delay.h>

/**
 * Bluetooth Send -- send keystrokes to bluetooth module using "shorthand" protocol
 * 
 * \param key 
 * \param modifier
 * 
 * \return void
 */
void Bluetooth_Send(uint8_t key, uint8_t modifier){
	
	set_high(BT_CTS);//toggle cts to wake module from deep sleep.
	_delay_us(100);
	set_low(BT_CTS);
	Delay_MS(5);//5 ms is worst-case wakeup time
		
	uart_putc(0xFE);//report is a "keyboard shorthand" report
    uart_putc(0x01);//1 key is sent at a time
	uart_putc(modifier);//modifier goes here
	uart_putc(key);

	//clear the keystroke
	
	uart_putc(0xFE);
	uart_putc(0x00);//send shorthand report length zero
	
	set_high(BT_CTS); //enable deep sleep
	
}

/**
 * Initialize Bluetooth 
 * reset, enter command mode, set parameters, reset again to lock in parameters.
 * 
 * \return void
 */
void Bluetooth_Init(){

	bool cmdmode;
	uint8_t attempt_count = 0;
	
	set_high(BT_BAUD); //sets the baud rate to 9600 upon reset
	
	Bluetooth_Reset(); //reset the module
	
	while(attempt_count < 5){
		attempt_count++;
		cmdmode = Bluetooth_Enter_CMD_Mode();
		if(Bluetooth_Enter_CMD_Mode()){
			break;
		}
		else{
			Delay_MS(1000); //if cmd mode fails, wait 1 second and try again.
		}
	}
	
	if(!cmdmode){ //if the bluetooth failed to enter command mode, fail out.
		Typewriter_Mode = PANIC_MODE; 
		return;
	}

	Bluetooth_Send_CMD("SF,1"); //SF command resets factory defaults;
	Delay_MS(BLUETOOTH_RESPONSE_DELAY); //delay for a little bit to make sure bluetooth has time to respond.
	Bluetooth_Send_CMD("SC,0000"); //SC and SD commands set COD device identifier (identify as a keyboard);
	Bluetooth_Send_CMD("SD,0540");
	Bluetooth_Send_CMD("SH,0301"); //set hid flags -- hid forced on by hid pin, ios keyboard toggles, device is keyboard, 1 device stored in reconnect queue on power-up
	Bluetooth_Send_CMD("SM,04");
	Bluetooth_Send_CMD("SP,1234"); // set pin code, if used
	Bluetooth_Send_CMD("SS,USB Typewriter"); // set "service name"
	Bluetooth_Send_CMD("SW,8050"); // set sniff mode to 50ms intervals, with deep sleep enabled
//	Bluetooth_Send_CMD("SY,0004"); // set transmit power -- default is 000C (see datasheet)
	Bluetooth_Send_CMD("S~,6"); // set profile to HID 
	Bluetooth_Send_CMD("S-,USB Typewriter"); // set friendly name
//	Bluetooth_Send_CMD("S|,0A01");// cycle 10s off and 1s on when waiting for a connection

	Bluetooth_Reset();

	
}

/**
 * Bluetooth Connect
 * Connect to the last host we remember connecting to successfully.
 * Do not call this function from command mode, or module will freeze up.
 * \return bool
 */
bool Bluetooth_Connect(){ 
	Bluetooth_Disconnect();
	Bluetooth_Enter_CMD_Mode();
	Bluetooth_Send_CMD("C");
	
	return true; //todo: check connection status pin to see if device connects or not
}

/**
 * Bluetooth Disconnect
 * Disconnect from current host. Do not call this from command mode, or module will freeze up.
 * 
 * \return bool
 */
bool Bluetooth_Disconnect(){ 
	uart_putc(0);
	Delay_MS(BLUETOOTH_RESPONSE_DELAY);
	
	return true; //todo: check connection status pin to see if device disconnects or not
}


/**
 * Bluetooth Send
 * 
 * \param command -- string containing formatted command to send to bluetooth module
 * 
 * \return void
 */
void Bluetooth_Send_CMD(char* command){
	int i = 0;
	while (command[i] != '\0'){//loop until end of string
		uart_putc(command[i]); //send first character of command
	}
	uart_putc('\r'); //send return carriage
	
		Delay_MS(BLUETOOTH_RESPONSE_DELAY); //delay for a little bit to make sure bluetooth has time to respond.

	}
	
void Bluetooth_Reset(){
	set_low(BT_RESET);//reset the bluetooth module
	Delay_MS(100); //hold in reset for 100ms
	set_high(BT_RESET); //reactivate bluetooth module
	Delay_MS(700);//takes 500ms from power on for module to be able to receive commands.
	
	set_low(BT_CTS);//this wakes the buetooth module
	Delay_MS(5);//it takes 5ms to wake up from low-power state
	
}


bool Bluetooth_Enter_CMD_Mode(){
	
	char response[5];//buffer to store and parse module response
	
	uart_putc('$');
	uart_putc('$');
	uart_putc('$'); //send $$$ to enter command mode

	Delay_MS(10);//wait to receive response
	
	uart_clear_rx_buffer();
	
	response[0] = (char)uart_getc();
	response[1] = (char)uart_getc();
	response[2] = (char)uart_getc(); //module sends "CMD" as a confirmation.
	
	if (response[0] != 'C'){ //if 'CMD' string is not received from module, an error occurred in intialization.
		return false; //entering command mode failed.
	}
	else{
		return true; //successfully entered command mode
	}
}
	

