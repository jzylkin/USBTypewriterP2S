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

uint8_t BT_State; //BT state is "inactive" by default

#if MODULE_NAME==EHONG
#define response_buffer_len 60
char response[response_buffer_len]; //array to store bluetooth module's responses to commands


bool BT_Asleep; //BT

char cmd_buffer[] = "AT+KR=A1,01,00,00,00,00,00,00,00,00";

// see https://www.feasycom.com/wp-content/uploads/2022/03/1648629572-FSC_BT8XX_Programming_User_Guide_V3_3.pdf
const char FC_TEST_UART[] PROGMEM = "AT";
const char FC_ENABLE_SPECIAL_GPIOS[] PROGMEM = "AT+PIOCFG=1,1";
const char FC_FACTORY_RESET[] PROGMEM = "AT+RESTORE";
const char FC_SET_FRIENDLY_NAME[] PROGMEM = "AT+NAME=Typewriter";
const char FC_SET_BLE_NAME[] PROGMEM = "AT+LENAME=Typewriter LE";
const char FC_SET_COD_FEATURES[] PROGMEM = "AT+COD=000540";
const char FC_SET_HID_MODE[] PROGMEM = "AT+MODE=2"; // 2 = HID mode 
const char FC_FLOW_CONTROL_OFF[] PROGMEM = "AT+FLOWCTL=0";
const char FC_TURN_ON_SSP_PAIRING[] PROGMEM = "AT+SSP=1";
const char FC_TURN_ON_AUTORECONNECT[] PROGMEM = "AT+AUTOCONN=1";
const char FC_IOS_ON_SCREEN_KB[] PROGMEM = "AT+HIDOSK";
const char FC_HID_SEND_DELAY[] PROGMEM = "AT+HIDDLY=10"; //Delay between transmissions...May need optimizing for supporting different phones?
const char FC_CLEAR_DEVICE_PAIRED_LIST[] PROGMEM = "AT+PLIST=0";
const char FC_SEND_EMPTY_HID_REPORT[] PROGMEM = "AT+HIDSEND=10, \xA1 \x01 \x00 \x00 \x00 \x00 \x00 \x00 \x00 \x00";
const char FC_CONNECT[] PROGMEM = "AT+HIDCONN";
const char FC_MAKE_DISCOVERABLE[] PROGMEM = "AT+PAIR=1";
const char FC_MAKE_SLAVE[] PROGMEM = "AT+ROLE=0";
const char FC_DISCONNECT[] PROGMEM = "AT+DSCA";
const char FC_RAW_MODE_SELECT[] PROGMEM = "AT+HIDMODE=0";


const char CHANGE_BAUD_RATE_MESSAGE[] PROGMEM = "CHANGING BAUD";
const char SENDING_MESSAGE[] PROGMEM = "SENDING";
//const char SET_GAP_IDENTIFICATION[] PROGMEM = "AT+GA=03C1";



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

//	Bluetooth_Send_CMD("AT+UI=00",false); // disable the ui
//	Delay_MS(10);
		if (key&FORCE_UPPER){ //in this program, we use the MSB of code to indicate that this key MUST be sent as upper case.
			reg_clr(key,FORCE_UPPER); //clear the MSB,
			modifier = UPPER; //and set the modifier to upper case.
		}
		
		if(key == KEY_EXECUTE){ // the "execute" command is for posting emails -- it actually sends a "CTRL+ENTER" command.
			key = KEY_ENTER;
			modifier = HID_KEYBOARD_MODIFIER_LEFTCTRL;
		}
	
		uart_putc(0xA1);uart_putc(0x01);uart_putc(modifier);uart_putc(key);//uart_putc(0);uart_putc(0);uart_putc(0);uart_putc(0);uart_putc(0);uart_putc(0);
		Delay_MS(1);
		uart_putc(0xA1);uart_putc(0x01);uart_putc(0);uart_putc(0);//uart_putc(0);uart_putc(0);uart_putc(0);uart_putc(0);uart_putc(0);uart_putc(0);
		
		Delay_MS(1); // TODO: NOT NEEDED?

		Get_Response(false);
}

//Toggle iOS virtual keyboard on and off.
void Bluetooth_Toggle_iOS_Keyboard(){
	uart_putc(0xA1);uart_putc(0x02);uart_putc(0x08);uart_putc(0); //this is the command to toggle a virtual keyboard.
	Delay_MS(5);
	uart_putc(0xA1);uart_putc(0x02);uart_putc(0);uart_putc(0);//now release the "toggle" key.
	Get_Response(true);

}


/**
 * Initialize Bluetooth 
 * reset, enter command mode, set parameters, reset again to lock in parameters.
 * 
 * \return void
 */
void Bluetooth_Init(){
	uart_clear_rx_buffer();
	
	uart_init(UART_BAUD_SELECT_DOUBLE_SPEED(115200,F_CPU));//reinitialize uart to 9600 baud
	
	Bluetooth_Reset(); //TODO: Actually reset the module
	Delay_MS(1000); // TODO: Why is this needed.

	BT_State = INITIALIZED;
}

bool Bluetooth_Test(){
		bool success = false;
		Bluetooth_Init();
		Bluetooth_Enter_Proxy_Mode();
		success = Bluetooth_Send_PROGMEM_CMD(FC_CLEAR_DEVICE_PAIRED_LIST,true); //If response is "OK" then bluetooth module is present and responsive.
		Bluetooth_Send_PROGMEM_CMD(FC_MAKE_SLAVE, true);
		Bluetooth_Send_PROGMEM_CMD(FC_MAKE_DISCOVERABLE, true);
		
		Bluetooth_Exit_Proxy_Mode();//enter bypass mode
		
		return success;
}

bool Bluetooth_Configure(){
	bool success;
	success = true;
	
	#ifdef BT_DEBUG
		while(USB_DeviceState != DEVICE_STATE_Configured){;} //usb must be connected for debug to work
		Delay_MS(500);
//		USBSendString("BT DEBUG MODE");
	#endif
	
	Bluetooth_Init();
	
	Bluetooth_Enter_Proxy_Mode();

    //enable responses from bluetooth module -- if response is "OK" then bluetooth module is present and responsive.
	int attempts = 1000;
	while(attempts > 0 && !Bluetooth_Send_PROGMEM_CMD(FC_TEST_UART,true)) {
	  attempts--;
	  Delay_MS(10);
	}
	success = (attempts != 0);
	
	if (success)
	{
		Bluetooth_Send_PROGMEM_CMD(FC_ENABLE_SPECIAL_GPIOS,true);
		//Bluetooth_Send_PROGMEM_CMD(FC_FACTORY_RESET,true);
		//Delay_MS(200);
		//Bluetooth_Send_PROGMEM_CMD(FC_DISCONNECT,true);
		//Bluetooth_Send_PROGMEM_CMD(FC_CLEAR_DEVICE_PAIRED_LIST,true);

		Bluetooth_Send_PROGMEM_CMD(FC_SET_COD_FEATURES,true);//set hid parameters
		Bluetooth_Send_PROGMEM_CMD(FC_SET_FRIENDLY_NAME,true); 
		Bluetooth_Send_PROGMEM_CMD(FC_SET_BLE_NAME,true); 
		Bluetooth_Send_PROGMEM_CMD(FC_TURN_ON_AUTORECONNECT,true);
		Bluetooth_Send_PROGMEM_CMD(FC_TURN_ON_SSP_PAIRING,true);

		Bluetooth_Send_PROGMEM_CMD(FC_HID_SEND_DELAY,true); 
		Bluetooth_Send_PROGMEM_CMD(FC_FLOW_CONTROL_OFF,true); 
		Bluetooth_Send_PROGMEM_CMD(FC_SET_HID_MODE,true);
		Bluetooth_Send_PROGMEM_CMD(FC_RAW_MODE_SELECT, true); // Use Raw HID Reports when in bypass throughput mode (ie when not sending AT commands).
		
	 // THESE WERE THE SETTINGS WHEN USING EHONG EH-MA41: NOT SURE WHAT THE FEASYCOM EQUIVALENTS ARE!
		/*enable the auto connection after power on as permanent mode;
		enable the auto connect after paired; -- was disabled by default.
		enable auto reconnect after link lost as permanent mode;
		set the interval of auto reconnect to 5s.
		configure the discover mode as 01: auto discoverable when empty.
		configure the timeout of discoverable as 600 seconds (10 min).
		This command is only needed when the first time use this Bluetooth module.*/
	
		//Bluetooth_Send_PROGMEM_CMD(FC_CLEAR_DEVICE_PAIRED_LIST,true); //clear paired device list, which forces the device to go into discovery mode.
		//Bluetooth_Send_PROGMEM_CMD(FC_MAKE_SLAVE, true);
		//Bluetooth_Send_PROGMEM_CMD(FC_MAKE_DISCOVERABLE, true);
	}
	
	Bluetooth_Exit_Proxy_Mode();//enter bypass mode
	
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
	if(verbose){USBSendString("snd ");}
    if(verbose){USBSendString(command);}
	if(verbose){USBSendString("\n");}
	#endif
	
	while (command[i] != '\0'){//loop until end of string
		uart_putc(command[i]); //send first character of command
		i++;
	}
	uart_putc('\r');
	uart_putc('\n'); //send return carriage
    if(verbose){
		return Get_Response(verbose);
	}
	return true;
}

bool Bluetooth_Send_PROGMEM_CMD(const char* progcommand, bool verbose){
		Bluetooth_Enter_Proxy_Mode();
		strcpy_P(StringBuffer, (char*) progcommand);
		return Bluetooth_Send_CMD(StringBuffer,verbose);
}

	
void Bluetooth_Reset(){
	//set_low(BT_RESET);//reset the bluetooth module
	//Delay_MS(75); //hold in reset for 50ms
	//set_high(BT_RESET); //reactivate bluetooth module
	//Delay_MS(BLUETOOTH_RESET_DELAY);//takes 500ms from power on for module to be able to receive commands.
	
//	set_low(BT_CTS);//this wakes the buetooth module
//	Delay_MS(5);//it takes 5ms to wake up from low-power state
	
}

void Bluetooth_Enter_Proxy_Mode(){
	Delay_MS(10);
	set_high(BT_AT_MODE);
	Delay_MS(10);
}

void Bluetooth_Exit_Proxy_Mode(){
	set_low(BT_AT_MODE);
	Delay_MS(10);
}

bool Get_Response(bool verbose){
	uint16_t tmpchar;
	if(verbose){
		Delay_MS(200); //wait for the response to be sent.
		response[0] = '\0'; //clear response string
		response[1] = '\0';
		response[2] = '\0';
		response[4] = '\0';
	
		for(uint8_t i=0; i<response_buffer_len ; i++){
			tmpchar = uart_getc();
			if ((tmpchar & 0xFF00) != 0){ //check upper byte of getc for error code -- most commonly this would be "buffer empty"
				response[i]='\0'; //mark the string as having ended.
				break; // reception is complete -- no more characters to retrieve.
			}
			else if (i == response_buffer_len-1){ //if i is the maximum index, string must end.
				response[i] = '\0';
				break;
			}
			else{
				response[i] = tmpchar & 0xFF; //store lower byte of getc as a uart character received
			}
			
		}
		#ifdef BT_DEBUG
			USBSendString(response);
		#endif
		
		if (strstr(response, "OK\r\n")){  //if response is "OK"
			return true;
		}
		else{

					#ifdef BT_DEBUG
					USBSendString(" N\n");
					#endif
					
								return false;
		}
	}
	else{
		return true;
	}

		
}

bool BluetoothInquire(){
	bool success = true;
	success &= Bluetooth_Send_PROGMEM_CMD(FC_CLEAR_DEVICE_PAIRED_LIST,true); //clear the paired device list.  This makes bluetooth enter discoverable state by default.
	return success;
}



bool Bluetooth_Connect(){

	// TODO: Check connection not already connected first e.g. if (is_low(BT_CONNECTED)) // if bluetooth is not already connected:
	
	Bluetooth_Enter_Proxy_Mode();
	
	#ifdef BT_DEBUG
		USBSendString("connect\n");
	#endif
	
	Bluetooth_Send_PROGMEM_CMD(FC_CONNECT,true); //then attempt a new connection.
	
	Bluetooth_Exit_Proxy_Mode();
	return true;
}

void Bluetooth_Enter_Pin(){
	uint32_t pin = 0;
	uint8_t key = 0 ;
	uint8_t code = 0;
	uint32_t pinplace = 100000;
	while(pinplace){
		key = GetKey();
		code = GetASCIIKeyCode(key, LOWER);
		if (code == 'l'){code = '1';} //lower case L is a 1.
		if(code)
		{
			code = code - '0'; //code number is converted from ascii to decimal by subtracting ascii 0.
			pin = pin + (pinplace*code);
			pinplace = pinplace / 10;
		}
	}
	Bluetooth_Send_Pin(pin);
}


#else
#error "BT MODULE NAME INVALID"
#endif

uint8_t Get_Bluetooth_State(){
	return BT_State;
}





