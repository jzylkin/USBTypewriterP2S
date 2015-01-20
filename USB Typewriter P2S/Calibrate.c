/*
 * Calibrate.c
 *
 * Created: 12/27/2014 1:26:25 PM
 *  Author: Jack
 */ 
#include "Keyboard.h"
#include "Send.h"
#include "KeyCodes.h"
#include "Calibrate.h"
#include "globals.h"




void Calibrate(){
//	ClearKeyCodeTables();
	while(USB_DeviceState != DEVICE_STATE_Configured){;}//wait for configuration to complete
	Delay_MS(1000);//wait 1 second.
    USBSendString("USB TYPEWRITER");
	USBSend(KEY_9,UPPER);
	USBSend(KEY_T,UPPER);
	USBSend(KEY_M,UPPER);
	USBSend(KEY_0,UPPER);
	USBSend(KEY_ENTER,0);
	USBSendString("VER");
	USBSend(KEY_SPACE,UPPER);
	USBSend(KEY_4,LOWER);
	USBSend(KEY_PERIOD,LOWER);
	USBSend(KEY_2,LOWER);
	USBSend(KEY_ENTER,LOWER);
	USBSendString("CALIBRATING");
	USBSend(KEY_PERIOD,LOWER);
	USBSend(KEY_PERIOD,LOWER);
	USBSend(KEY_PERIOD,LOWER);
	USBSend(KEY_ENTER,LOWER);
	USBSendString("TYPE THE FOLLOWING KEYS");
	USBSend(KEY_PERIOD,LOWER);
	USBSend(KEY_PERIOD,LOWER);
	USBSend(KEY_PERIOD,LOWER);
	USBSend(KEY_ENTER,LOWER);
	
	for (char learnChar = KEY_A; learnChar <= KEY_Z; learnChar ++){
		USBSend(learnChar, UPPER);
		USBSend(KEY_SPACE,LOWER);// used to be a colon
		GetTeachKey(learnChar);
		USBSend(KEY_ENTER,LOWER);
	}
	
	Init_Mode(); //go into regular mode (whatever that is -- usb, bluetooth, etc)
	
	//CODE TO SAVE ARRAYS INTO EEPROM MUST GO HERE!!!
	//
	//
	//
}

void GetTeachKey(char teachkey){
	int keypressed = 0;
	while(keypressed == 0){ //keep getting a key until there is a key to get.
		keypressed = GetKeySimple();
	}
	if (is_low(S3)){
		ShiftKeyCodeLookUpTable[keypressed] = teachkey;
		USBSendString("SHIFT");
		USBSend(KEY_EQ,UPPER); //send a + sign
		USBSendNumber(keypressed);
	}
	else if (is_low(S2)){ //if Alt is being held down,
		FnKeyCodeLookUpTable[keypressed] = teachkey;
		//send "FN+number"
		USBSendString("FN");
		USBSend(KEY_EQ,UPPER); //send a + sign
		USBSendNumber(keypressed);
	}
	else{
		KeyCodeLookUpTable[keypressed] = teachkey;
		USBSendNumber(keypressed);
	}
	
	USBSend(KEY_ENTER,0);
	
	Delay_MS(500);//implement 500 MS delay
}


