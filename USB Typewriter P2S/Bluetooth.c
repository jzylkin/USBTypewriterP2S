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

#define response_buffer_len 64
char response[response_buffer_len]; //array to store bluetooth module's responses to commands
uint8_t BT_State; //BT state is "inactive" by default
bool BT_Asleep; //BT

/**
 * Bluetooth Send -- send keystrokes to bluetooth module using "shorthand" protocol
 * 
 * \param key 
 * \param modifier
 * 
 * \return void
 */
void Bluetooth_Send(uint8_t key, uint8_t modifier){
	char cmd_buffer[] = "AT+KR=A1,01,00,00,00,00,00,00,00,00";
//	set_high(BT_CTS);//toggle cts to wake module from deep sleep.
//	_delay_us(100);

	Bluetooth_Send_CMD("AT+UI=01",false);
	Delay_MS(1);
		if (key&FORCE_UPPER){ //in this program, we use the MSB of code to indicate that this key MUST be sent as upper case.
			reg_clr(key,FORCE_UPPER); //clear the MSB,
			modifier = UPPER; //and set the modifier to upper case.
		}
		
		if(key == KEY_EXECUTE){ // the "execute" command is for posting emails -- it actually sends a "CTRL+ENTER" command.
			key = KEY_ENTER;
			modifier = HID_KEYBOARD_MODIFIER_LEFTCTRL;
		}
		sprintf(cmd_buffer,"AT+KR=A1,01,%02x,00,%02x,00,00,00,00,00",modifier,key);
		Bluetooth_Send_CMD(cmd_buffer, false);
			//clear the keystroke
		Delay_MS(1);
		sprintf(cmd_buffer,"AT+KR=A1,01,%02x,00,%02x,00,00,00,00,00",0,0);
		Bluetooth_Send_CMD(cmd_buffer, false);

}

/**
 * Initialize Bluetooth 
 * reset, enter command mode, set parameters, reset again to lock in parameters.
 * 
 * \return void
 */
void Bluetooth_Init(){
	Bluetooth_Reset(); //reset the module
	Delay_MS(1000);
	Get_Response(true);
	Bluetooth_Send_CMD("AT+BP=00,00", true); //get into proxy mode so that commands work correctly.
	Delay_MS(1000);
	BT_State = INITIALIZED;
}

bool Bluetooth_Configure(){
	bool success;
	success = true;
	
	#ifdef BT_DEBUG
		while(USB_DeviceState != DEVICE_STATE_Configured){;} //usb must be connected for debug to work
		Delay_MS(500);
		USBSendString("BT DEBUG MODE");
	#endif
	
	Bluetooth_Init();

	success &= Bluetooth_Send_CMD("AT+UI=01",true); //enable responses from bluetooth module -- if response is "OK" then bluetooth module is present and responsive.		
	Bluetooth_Send_CMD("AT+NM=USB Typewriter BT",true); //set friendly name
	Bluetooth_Send_CMD("AT+BP=00,00",true); //bypass channel is proxy
	Bluetooth_Send_CMD("AT+PF=00,01,00,00,00",true);//set hid parameters
	Delay_MS(500);
	Bluetooth_Send_CMD("AT+FT=00,01,FF,05,01,0258",true); //configure module features (see manual):
	Bluetooth_Send_CMD("AT+MM=00",true); //do not use man-in-middle protection
	Bluetooth_Send_CMD("AT+IO=03",true);//set IO setting to "no input or output" hopefully this means no need for pin-code.
 //
	/*disable the auto connection after power on as permanent mode;
	enable the auto connect after paired; -- was disabled by default.
	enable auto reconnect after link lost as permanent mode;
	set the interval of auto reconnect to 5s.
	configure the discover mode as 01: auto discoverable when empty.
	configure the timeout of discoverable as 600 seconds (10 min).
	This command is only needed when the first time use this Bluetooth module.*/
	
	Bluetooth_Send_CMD("AT+CP",true); //clear paired device list, which forces the device to go into discovery mode.
	
	return success; //if any of the commands failed, success will be false.
}


/**
 * Bluetooth Send
 * 
 * \param command -- string containing formatted command to send to bluetooth module
 * 
 * \return void
 */
bool Bluetooth_Send_CMD(char* command, bool verbose){
	
	int i = 0;
	
	uart_clear_rx_buffer();
	#ifdef BT_DEBUG
	if(verbose){USBSendString("sending command\n");}
	#endif
	
	while (command[i] != '\0'){//loop until end of string
		uart_putc(command[i]); //send first character of command
		i++;
	}
	uart_putc('\r');
	uart_putc('\n'); //send return carriage

	return Get_Response(verbose);
}

	
void Bluetooth_Reset(){
	set_low(BT_RESET);//reset the bluetooth module
	Delay_MS(50); //hold in reset for 50ms
	set_high(BT_RESET); //reactivate bluetooth module
	Delay_MS(BLUETOOTH_RESET_DELAY);//takes 500ms from power on for module to be able to receive commands.
	
	set_low(BT_CTS);//this wakes the buetooth module
	Delay_MS(5);//it takes 5ms to wake up from low-power state
	
}


bool Get_Response(bool verbose){
	uint16_t tmpchar;
	if(verbose){
		Delay_MS(BLUETOOTH_RESPONSE_DELAY); //wait for the response to be sent.
		response[0] = '\0'; //clear response string
		response[1] = '\0';
		response[2] = '\0';
		response[4] = '\0';
	
		for(uint8_t i=0; i<response_buffer_len; i++){
			tmpchar = uart_getc();
			if ((tmpchar & 0xFF00) != 0){ //check upper byte of getc for error code -- most commonly this would be "buffer empty"
				response[i]='\0'; //mark the string as having ended.
				break; // reception is complete -- no more characters to retrieve.
			}
			
			response[i] = tmpchar & 0xFF; //store lower byte of getc as a uart character received
			
		}
		#ifdef BT_DEBUG
			USBSendString(response);
		#endif
		
		if ((response[0] == 'O')||(response[2] == '=')){  //if response is "OK" or "XX=...", bt module has received command successfully
			return true;
		}
		else{
			return false;
		}
	}
	else{
		return true;
	}

		
}

bool BluetoothInquire(){
	bool success = true;
	success &= Bluetooth_Send_CMD("AT+CP",true); //clear the paired device list.  This makes bluetooth enter discoverable state by default.
	return success;
}

uint8_t Get_Bluetooth_State(){
	return BT_State;
}

void BT_Sleep(){
	if (!BT_Asleep){ //if not already asleep:
		Bluetooth_Send_CMD("AT+SP=01",true); //tell bt module to sleep
		BT_Asleep = true;
	}
}

void BT_Wake(){
	if (BT_Asleep){
		Bluetooth_Send_CMD("AT+SP=00",true); //disable sleep mode
		BT_Asleep = false; //bt is no longer asleep
	}
}

bool Bluetooth_Connect(){
//	if (is_low(BT_CONNECTED)) // if bluetooth is not already connected:
	const char s[4] = "MD=";
	USBSendString("connect\n");
	Delay_MS(1000);
	Bluetooth_Send_CMD("AT+UI=01",false); //enable ui
	Bluetooth_Send_CMD("AT+MD",true);
	char* numericalresponse = strtok(response,s); //parse result so we only get the part after MD=
	numericalresponse = strtok(NULL, s); // calling function again accesses everything after the delimiter
	if (numericalresponse != NULL){ //if there is anything to report.
		if((numericalresponse[1] == '0')){ //if this is not discoverable mode...
			Bluetooth_Send_CMD("AT+CI",true); //then attempt a new connection.
		}
	}
	return true;
}




