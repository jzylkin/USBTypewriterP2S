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

void USBSend(uint8_t code,uint8_t mod){	
	
	TMR1_Count = 0;
	while (KeyBuffer->KeyCode[0] && TMR1_Count < 20) {Do_HID_Task();} //if buffer is full, wait.  If timeout expires, stop waiting.
	KeyBuffer->KeyCode[0] = code;
	KeyBufferMod = mod;
	Do_HID_Task();
	Delay_MS(100);
	Do_HID_Task();
}


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
		else if(str[i] == '?'){
			code = KEY_SLASH;
			modifier = UPPER;
		}
		else if((str[i] == '\r')||(str[i] == '\n')){
			code = KEY_ENTER;
		}
		else if(str[i] == '.'){
			code = KEY_PERIOD;
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
	}
} 

void USBSendNumber(uint8_t number){
	uint8_t ones ;
	uint8_t tens;
	
	ones = number%10;
	tens = ((number - ones)%100)/10;
	
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

