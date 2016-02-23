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
#define response_buffer_len 32
char response[response_buffer_len]; //array to store bluetooth module's responses to commands


bool BT_Asleep; //BT

char cmd_buffer[] = "AT+KR=A1,01,00,00,00,00,00,00,00,00";

const char SET_FRIENDLY_NAME[] PROGMEM = "AT+NM=USB Typewriter BT";
const char ENABLE_UI[] PROGMEM = "AT+UI=01";
const char SET_PROXY_MODE[] PROGMEM = "AT+BP=00,00";
const char SET_HID_PARAMS[] PROGMEM = "AT+PF=00,01,00,00,00";
const char SET_MODULE_FEATURES[] PROGMEM = "AT+FT=00,01,FF,05,01,0258";
const char DISABLE_MIM[] PROGMEM = "AT+MM=00"; 
const char DISABLE_PIN[] PROGMEM = "AT+IO=03";
const char CLEAR_PAIRED_LIST[] PROGMEM = "AT+CP";
const char ENABLE_SLEEP[] PROGMEM = "AT+SP=01";
const char DISABLE_SLEEP[] PROGMEM = "AT+SP=00";
const char MAKE_DISCOVERABLE[] PROGMEM = "AT+MD";
const char CONNECT_TO_PAIRED_DEVICE[] PROGMEM = "AT+CI";
const char SEND_EMPTY_HID_REPORT[] PROGMEM = "AT+KR=A1,01,00,00,00,00,00,00,00,00";
const char SET_BYPASS_MODE[] PROGMEM = "AT+BP=04,07,00";



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
	
		uart_putc(0xA1);uart_putc(0x01);uart_putc(modifier);uart_putc(0);uart_putc(key);uart_putc(0);uart_putc(0);uart_putc(0);uart_putc(0);uart_putc(0);
		Delay_MS(5);
		uart_putc(0xA1);uart_putc(0x01);uart_putc(0);uart_putc(0);uart_putc(0);uart_putc(0);uart_putc(0);uart_putc(0);uart_putc(0);uart_putc(0);
}

/**
 * Initialize Bluetooth 
 * reset, enter command mode, set parameters, reset again to lock in parameters.
 * 
 * \return void
 */
void Bluetooth_Init(){
	uart_init(UART_BAUD_SELECT(9600,F_CPU));//reinitialize uart to 9600 baud
	
	Bluetooth_Reset(); //reset the module
	Delay_MS(1000);
	
	bool btbaudis9600 = Get_Response(true); //getresponse will only be intelligible if baud is 9600 still, because Atmega's baud rate is still 9600

	if(btbaudis9600){ //if bt module still has baud 9600, change it to 57600
		Bluetooth_Enter_Proxy_Mode();
		Bluetooth_Send_CMD("AT+BR=0B",false);//change baud rate to 57600 if 
	}
	
	uart_init(UART_BAUD_SELECT(57600,F_CPU));//reinitialize uart to match the BT module's baud rate.
	Bluetooth_Enter_Proxy_Mode();

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

	success &= Bluetooth_Send_PROGMEM_CMD(ENABLE_UI,true); //enable responses from bluetooth module -- if response is "OK" then bluetooth module is present and responsive.		
	Bluetooth_Send_PROGMEM_CMD(SET_FRIENDLY_NAME,true); //set friendly name
	Bluetooth_Send_PROGMEM_CMD(SET_PROXY_MODE,true); //bypass channel is proxy
	Bluetooth_Send_PROGMEM_CMD(SET_HID_PARAMS,true);//set hid parameters
	Delay_MS(500);
	Bluetooth_Send_PROGMEM_CMD(SET_MODULE_FEATURES,true); //configure module features (see manual):
	Bluetooth_Send_PROGMEM_CMD(DISABLE_MIM,true); //do not use man-in-middle protection
	Bluetooth_Send_PROGMEM_CMD(DISABLE_PIN,true);//set IO setting to "no input or output" hopefully this means no need for pin-code.
	Bluetooth_Send_PROGMEM_CMD(DISABLE_SLEEP,true);//no sleep mode.
	
	
 //
	/*disable the auto connection after power on as permanent mode;
	enable the auto connect after paired; -- was disabled by default.
	enable auto reconnect after link lost as permanent mode;
	set the interval of auto reconnect to 5s.
	configure the discover mode as 01: auto discoverable when empty.
	configure the timeout of discoverable as 600 seconds (10 min).
	This command is only needed when the first time use this Bluetooth module.*/
	
	Bluetooth_Send_PROGMEM_CMD(CLEAR_PAIRED_LIST,true); //clear paired device list, which forces the device to go into discovery mode.
	Bluetooth_Send_PROGMEM_CMD(SET_BYPASS_MODE,false);//go into hid bypass mode
	
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
    if(verbose){
		return Get_Response(verbose);
	}
	return true;
}

bool Bluetooth_Send_PROGMEM_CMD(const char* progcommand, bool verbose){
		strcpy_P(StringBuffer, (char*) progcommand);
		return Bluetooth_Send_CMD(StringBuffer,verbose);

\
}

	
void Bluetooth_Reset(){
	set_low(BT_RESET);//reset the bluetooth module
	Delay_MS(50); //hold in reset for 50ms
	set_high(BT_RESET); //reactivate bluetooth module
	Delay_MS(BLUETOOTH_RESET_DELAY);//takes 500ms from power on for module to be able to receive commands.
	
	set_low(BT_CTS);//this wakes the buetooth module
	Delay_MS(5);//it takes 5ms to wake up from low-power state
	
}

void Bluetooth_Enter_Proxy_Mode(){
	Delay_MS(1000);
	Bluetooth_Send_PROGMEM_CMD(SET_PROXY_MODE,false);
	Delay_MS(1000);
}

void Bluetooth_Exit_Proxy_Mode(){
	Bluetooth_Send_PROGMEM_CMD(SET_BYPASS_MODE,false);
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
		
		if ((response[0] == 'O')||(response[2] == '=')||(response[0] == 'C')){  //if response is "OK" or "XX=...", or "Copyright" bt module has received command successfully
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
	success &= Bluetooth_Send_PROGMEM_CMD(CLEAR_PAIRED_LIST,true); //clear the paired device list.  This makes bluetooth enter discoverable state by default.
	return success;
}

uint8_t Get_Bluetooth_State(){
	return BT_State;
}

void BT_Sleep(){
	if (!BT_Asleep){ //if not already asleep:
		Bluetooth_Send_PROGMEM_CMD(ENABLE_SLEEP,true); //tell bt module to sleep
		BT_Asleep = true;
	}
}

void BT_Wake(){
	if (BT_Asleep){
		Bluetooth_Send_PROGMEM_CMD(DISABLE_SLEEP,true); //disable sleep mode
		BT_Asleep = false; //bt is no longer asleep
	}
}

bool Bluetooth_Connect(){
//	if (is_low(BT_CONNECTED)) // if bluetooth is not already connected:
	const char s[4] = "MD=";
	
	#ifdef BT_DEBUG
		USBSendString("connect\n");
	#endif
	
	Delay_MS(1000);
	Bluetooth_Send_PROGMEM_CMD(ENABLE_UI,false); //enable ui
	Bluetooth_Send_PROGMEM_CMD(MAKE_DISCOVERABLE,true);
	char* numericalresponse = strtok(response,s); //parse result so we only get the part after MD=
	numericalresponse = strtok(NULL, s); // calling function again accesses everything after the delimiter
	if (numericalresponse != NULL){ //if there is anything to report.
		if((numericalresponse[1] == '0')){ //if this is not discoverable mode...
			Bluetooth_Send_PROGMEM_CMD(CONNECT_TO_PAIRED_DEVICE,true); //then attempt a new connection.
		}
	}
	return true;
}

#elif MODULE_NAME==RN42
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
	BT_State = INITIALIZED;
	
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

    Bluetooth_Init(); //reset the module
	
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
#else
#error "BT MODULE NAME INVALID"
#endif

uint8_t Get_Bluetooth_State(){
	return BT_State;
}



