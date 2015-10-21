/*
 * Send.c
 *
 * Created: 12/27/2014 1:30:05 PM
 *  Author: Jack
 */ 

#include "Keyboard.h"
#include "Send.h"
#include "KeyCodes.h"
#include "globals.h"
#include <ctype.h> // allows toupper()

extern USB_ClassInfo_HID_Device_t Keyboard_HID_Interface; //import the keyboard interface object from main routine so we can call usbtask on it.

/*Send a character over usb with a given modifier (shift, ctrl, etc)*/
void USBSend(uint8_t code,uint8_t mod){	
	
	TMR1_Count = 0;
	while (KeyBuffer->KeyCode[0] && TMR1_Count < USB_SEND_TIMEOUT) {
		if((Typewriter_Mode == USB_COMBO_MODE) || (Typewriter_Mode == USB_LIGHT_MODE)){ //interrupts do not handle keyboard stuff during this mode.
			HID_Device_USBTask(&Keyboard_HID_Interface);
		} //if buffer is full, wait.  If timeout expires, stop waiting.
	}
	
	if (code&FORCE_UPPER){ //in this program, we use the MSB of code to indicate that this key MUST be sent as upper case.
		reg_clr(code,FORCE_UPPER); //clear the MSB,  
		mod = UPPER; //and set the modifier to upper case.
	}
	
	if(code == KEY_EXECUTE){ // the "execute" command is for posting emails -- it actually sends a "CTRL+ENTER" command.
		code = KEY_ENTER;
		mod = HID_KEYBOARD_MODIFIER_LEFTCTRL;
	}
	
	cli();//make sure there are no interrupts between setting code and setting the modifier that goes with it.
	KeyBuffer->KeyCode[0] = code; //cue up keycode to be sent during next LUFA HID callback function.
	KeyBufferMod = mod;
	HID_Device_USBTask(&Keyboard_HID_Interface); //Dean Camera says to call this function regularly -- right after sending a character seems like an appropriate time.
	sei();//re-enable the interrupts.
	
	Delay_MS(USB_SEND_DELAY);// wait X ms after sending each character.
	
	cli();//make sure no interrupts occur during the usb task.
	HID_Device_USBTask(&Keyboard_HID_Interface); //do LUFA hid usb tasks
	sei();
}

/*Send a string over USB. Only supports some characters.*/
void USBSendString(char *str){
	int length;
	uint8_t code;
	uint8_t modifier;
	length = strlen(str);
	for (int i=0; i<length; i++){
		modifier = LOWER;
		if(str[i] == ' '){
			code = KEY_SPACE;
		}
		else if(str[i] == '('){
			code = KEY_9;
			modifier = UPPER;
		}
		else if(str[i] == ')'){
			code = KEY_0;
			modifier = UPPER;
		}
		else if(str[i] == '/'){
			code = KEY_SLASH;
		}
		else if(str[i] == '?'){
			code = KEY_SLASH;
			modifier = UPPER;
		}
		else if(str[i] == ':'){
			code = HID_KEYBOARD_SC_SEMICOLON_AND_COLON;
			modifier = UPPER;
		}
		else if((str[i] == '\r')||(str[i] == '\n')){
			code = KEY_ENTER;
		}
		else if(str[i] == '.'){
			code = KEY_PERIOD;
		}
		else if(str[i] == ':'){
			code = HID_KEYBOARD_SC_SEMICOLON_AND_COLON;
			modifier = UPPER;
		}
		else if(str[i] == '!'){
			code = KEY_1;
			modifier = UPPER;
		}
		else if(str[i] == '0'){
			code = KEY_0;
		}
		else if((str[i] >= ASCII_1 )&&(str[i] <= ASCII_9)){
			code = str[i] - ASCII_1 + KEY_1; //translate ascii to hid number code 
		}
		else{
			code = toupper(str[i]);//make sure code is uppercase.
			code = code-ASCII_A+KEY_A;  //Convert the character (which is uppercase Ascii)  to an USB HID Keycode.
			modifier = UPPER;
		}
		USBSend(code,modifier);
		Delay_MS(STRING_SEND_DELAY);
	}
} 

/*Send a string literal over USB, using a string stored in program memory instead of data memory (this saves on data RAM)*/
void USBSendPROGString(const char*  ProgStr){
	strcpy_P(StringBuffer, (char*) ProgStr);
	USBSendString(StringBuffer);
}

/*Send a number between 0 and 255 over usb)*/
void USBSendNumber(uint8_t number){
	uint8_t ones ;
	uint8_t tens;
	
	ones = number%10;
	tens = ((number - ones)%100)/10;
	
	if (number >= 200){
		USBSend(KEY_2,LOWER);
	}
	else if (number >= 100){
		USBSend(KEY_1,LOWER);
	}
	
	if (tens != 0){
		USBSend(29+tens,LOWER);
	}
	else{
		USBSend(39,LOWER);
	}

	Delay_MS(100);
	
	if (ones!=0){
		USBSend(29+ones,LOWER);
	}
	else{
		USBSend(39,LOWER);
	}
	
	Delay_MS(100);

}

