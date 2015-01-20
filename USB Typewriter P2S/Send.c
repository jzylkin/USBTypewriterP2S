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
	while (KeyBuffer->KeyCode[0] && TMR1_Count < 20) {} //if buffer is full, wait.  If timeout expires, stop waiting.
	KeyBuffer->KeyCode[0] = code;
	KeyBufferMod = mod;
	Delay_MS(100);
}



void USBSendString(const char *str){
	int length;
	int code;
	length = strlen(str);
	for (int i=0; i<length; i++){
		if(str[i] == ' '){
			code = KEY_SPACE;
		}
		else{
			code = toupper(str[i]);//make sure code is uppercase.
			code = code-ASCII_A+HID_A;  //Convert the character (which is uppercase Ascii)  to an USB HID Keycode.
		}
		USBSend(code,HID_KEYBOARD_MODIFIER_LEFTSHIFT);
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

