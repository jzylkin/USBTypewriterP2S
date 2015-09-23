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
	
//	if(!eeprom_read_byte((uint8_t*)BLUETOOTH_CONFIGURED_ADDR)){ // check if bluetooth needs to be configured, and if so:
//		eeprom_write_byte((uint8_t*)BLUETOOTH_CONFIGURED_ADDR, (uint8_t)Bluetooth_Configure()); //configure, then tell eeprom if bluetooth configured successfully or not.
//	}
	
}

bool Bluetooth_Configure(){
	bool cmdmode;
	uint8_t attempt_count = 0;
	bool success;
	
//No longer necessary, since module is permanently fixed to 9600 baud.
//	USBSendString("BT RESET...");
   // set_high(BT_BAUD); //sets the baud rate to 9600 upon reset

    Bluetooth_Reset(); //reset the module
	
	//Try to enter command mode, repeat if necessary:
	do {
		attempt_count++;
		cmdmode = Bluetooth_Enter_CMD_Mode();
		Delay_MS(200);
	}while((attempt_count < 5)&&(!cmdmode));
	
	if(!cmdmode){ //if the bluetooth failed to enter command mode, fail out.
		Typewriter_Mode = PANIC_MODE;
		return false;
	}

	Bluetooth_Send_CMD("SF,1"); //SF command resets factory defaults;
	Delay_MS(1000); //delay for a little bit to make sure bluetooth has time to respond.
	
	success = true;
	success &= Bluetooth_Send_CMD("SC,0000"); //DEFAULT; SC and SD commands set COD device identifier (identify as a keyboard);
	success &= Bluetooth_Send_CMD("SD,0540");
	success &= Bluetooth_Send_CMD("SA,1");//use pin-code
	success &= Bluetooth_Send_CMD("SH,0302"); //set hid flags -- hid forced on by hid pin, ios keyboard toggles, device is keyboard, 2 devices stored in reconnect queue on power-up
	success &= Bluetooth_Send_CMD("SM,6"); //bluetooth is in slave connect mode
	Bluetooth_Send_CMD("SP,1234"); // set pin code, if used
//	success &= Bluetooth_Send_CMD("SS,Keyboard"); // set "service name"
	success &= Bluetooth_Send_CMD("ST,255"); // set configuration timer to never timeout
	//	Bluetooth_Send_CMD("SW,8050"); // set sniff mode to 50ms intervals, with deep sleep enabled
	success &= Bluetooth_Send_CMD("SY,0010"); // set transmit power -- default is 000C (see datasheet)
	Bluetooth_Send_CMD("S-,USB TYPEWRITER"); // set friendly name
	//	Bluetooth_Send_CMD("S|,0A01");// cycle 10s off and 1s on when waiting for a connection

	success &= Bluetooth_Send_CMD("S~,6"); // set profile to HID
	success &= Bluetooth_Send_CMD("R,1");
	
	Delay_MS(BLUETOOTH_RESET_DELAY);
	return success; //if any of the commands failed, success will be false.
}

/**
 * Bluetooth Connect
 * Connect to the last host we remember connecting to successfully.
 * Do not call this function from command mode, or module will freeze up.
 * \return bool
 */
bool Bluetooth_Connect(){ 
	bool cmdmode;
	Bluetooth_Disconnect();
	
	cmdmode = Bluetooth_Enter_CMD_Mode();
	
	if(!cmdmode){ //if the bluetooth failed to enter command mode, fail out.
		Typewriter_Mode = PANIC_MODE;
		return false;
	}
	
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
bool Bluetooth_Send_CMD(char* command){
	
	int i = 0;
	
	uart_clear_rx_buffer();
	
	while (command[i] != '\0'){//loop until end of string
		uart_putc(command[i]); //send first character of command
		i++;
	}
	uart_putc('\r'); //send return carriage
	
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


bool Bluetooth_Enter_CMD_Mode(){

	uart_clear_rx_buffer();
	uart_putc('$');
	uart_putc('$');
	uart_putc('$'); //send $$$ to enter command mode
	
	if (Get_Response()){ //first character sent back from host should be 'C' (For "CMD")
		return true; //entering command mode succeeded.
	}
	else{
		return false; //failed to enter command mode
	}
}

void Bluetooth_Exit_CMD_Mode(){


	uart_clear_rx_buffer();
	uart_putc('-');
	uart_putc('-');
	uart_putc('-'); //send --- to exit command mode
	Get_Response();

}

//void Bluetooth_Save_Address(){
//	Bluetooth_Enter_CMD_Mode();
//	Bluetooth_Send_CMD(GR);
//}

bool Get_Response(){
		char response[5];
		
		Delay_MS(BLUETOOTH_RESPONSE_DELAY);
		response[0] = uart_getc() & 0xFF;
		response[1] = uart_getc() & 0xFF;
		response[2] = uart_getc() & 0xFF;
		response[3]=  uart_getc() & 0xFF;
		response[4] = '\0';
		
	//	USBSendString(response);
	//	USBSend(KEY_ENTER,LOWER);
	
		

		if ((response[0] == 'C')||(response[0] == 'A')|| (response[0] == 'R')){  //if response is "CMD" or "AOK" or "REBOOT", bt module has received command successfully
			return true;
		}
		else{
			return false;
		}
		
}
	

