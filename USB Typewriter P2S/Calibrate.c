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
    USBSendString("TM");
	USBSend(KEY_0,UPPER);
	USBSend(KEY_ENTER,0);
	USBSendString("FIRMWARE VER");
	USBSend(KEY_SPACE,UPPER);
	USBSend(KEY_5,LOWER);
	USBSend(KEY_PERIOD,LOWER);
	USBSend(KEY_0,LOWER);
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
	
	for (char HIDkey = KEY_A; HIDkey <= KEY_Z; HIDkey ++){
		
		USBSend(HIDkey, UPPER);
		USBSend(KEY_SPACE,LOWER);// used to be a colon
		KeyPressed = WaitForKeystroke();
		Modifier = GetModifier();
		
		TeachHIDKey(HIDKey, KeyPressed, LOWER); //all alphanumeric keys programmed as a/A pairs -- no special shift keys allowed.
	
		ASCIIKey = HIDkey-KEY_A+ASCII_A;
		TeachASCIIKey(ASCIIKey, KeyPressed, UPPER);
		TeachASCIIKey(ASCIIKey+0x20, KeyPressed, LOWER);
		
		USBSend(KEY_ENTER,LOWER);
	}
		
   //Save calibration to eeprom:
	 eeprom_write_block (KeyCodeLookUpTable, (void *) KEYCODE_ADDR, KEYCODE_ARRAY_LENGTH);
	 eeprom_write_block (FnKeyCodeLookUpTable, (void *) FN_KEYCODE_ADDR, KEYCODE_ARRAY_LENGTH);
	 eeprom_write_block (ShiftKeyCodeLookUpTable, (void *) SHIFT_KEYCODE_ADDR, KEYCODE_ARRAY_LENGTH);
	 eeprom_write_block (ASCIILookUpTable,(void *) ASCII_ADDR, KEYCODE_ARRAY_LENGTH);
	 eeprom_write_block (ASCIIShiftLookUpTable,(void *) ASCII_SHIFT_ADDR, KEYCODE_ARRAY_LENGTH);
	 eeprom_write_block (ReedSwitchLookUpTable, (void *) REED_SWITCH_ADDR, NUM_REED_SWITCHES);
	 
	 
	Init_Mode(); //go into regular mode (whatever that is -- usb, bluetooth, etc)

}

int WaitForKeypress(){
	int KeyPressed = 0;
	
	Delay_MS(500);//implement 500 MS delay before detecting a key.  (prevents rapid re-detection of keys over and over.
	
	while(KeyPressed == 0){ //keep getting a key until there is a key to get.
		KeyPressed = GetKeySimple();
	}
	return KeyPressed;
}
	
	
void TeachHIDKey(char teachkey, int keypressed, char Modifier){
	if (Modifier == UPPER){
		ShiftKeyCodeLookUpTable[keypressed] = teachkey;
		USBSendString("SHIFT");
		USBSend(KEY_EQ,UPPER); //send a + sign
		USBSendNumber(keypressed);
	}
	else if (Modifier == HID_KEYBOARD_MODIFIER_LEFTALT){ //if FN is being held down,
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
}

void TeachASCIIKey(char teachkey, int keypressed, char Modifier){
	if (Modifier == UPPER){
		ASCIIShiftLookUpTable[keypressed] = teachkey;
	}
	else{
		ASCIILookUpTable[keypressed] = teachkey;
	}
}


