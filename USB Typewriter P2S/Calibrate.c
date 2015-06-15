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

const uint8_t HIDNumbers[] = {KEY_0,KEY_1,KEY_2,KEY_3,KEY_4,KEY_5,KEY_6,KEY_7,KEY_8,KEY_9}; //usb codes for 0-9
const uint8_t ASCIINumbers[] = {'0','1','2','3','4','5','6','7','8','9'}; // the ascii codes for 0-9
const uint8_t ASCIINumSymbols[] = {')','!','\"','#','$','%','_','*','('};// ascii codes for weird characters above 0-9 on  typewriters.

void Calibrate(){
//	ClearKeyCodeTables();
	uint8_t ASCIIKey;
	uint8_t HIDKey;
	uint8_t Modifier;
	uint8_t KeyPressed;
	
	bool HallSensorTest1;
	bool HallSensorTest2;

	while(USB_DeviceState != DEVICE_STATE_Configured){;}//wait for configuration to complete
	Delay_MS(1000);//wait 1 second.
	
//--------SEND PROMPT TO USER -----------
    USBSendString("USB TYPEWRITER ");
	USBSend(KEY_9,UPPER);
    USBSendString("TM");
	USBSend(KEY_0,UPPER);
	USBSendString("\r");
	USBSendString("FIRMWARE VER 5.0\r");
	USBSendString("CALIBRATING...\r");
	
	/*Measure the reed switch polarities*/
		Reed1Polarity = is_low(REED_1); //if reed_1 is low at start of calibration, then the polarity of reed 1 is active high
		Reed2Polarity= is_low(REED_2);
		Reed3Polarity = is_low(REED_3);
		Reed4Polarity = is_low(REED_4);
	
	if (is_low(CMD_KEY)){ //ALT is a hidden way to activate the hall effect sensor
		HallSensorTest1 = getHallState(); //sample the hall effect sensor bit
		USBSendString("FIRMLY PRESS ANY KEY TO CALIBRATE HALL EFFECT SENSOR...\r");
		Delay_MS(2000);
		HallSensorTest2 = getHallState(); //sample it again
		if (HallSensorTest1 == HallSensorTest2){ //if it has not changed, hall effect sensor is not present.
			USBSendString("NO HALL EFFECT SENSOR DETECTED!\r");
			UseHallSensor = false;
		}
		else{ // if it has changed, the "active" polarity of the hall effect sensor is HallSensorTest2
			USBSendString("OK!\r");
			UseHallSensor = true;
			HallSensorPolarity = HallSensorTest2;
		}
		
		/*Save the hall effect parameters to eeprom right away:*/
		eeprom_update_byte ((uint8_t *)USE_HALL_SENSOR_ADDR, UseHallSensor);
		eeprom_update_byte ((uint8_t *)HALL_SENSOR_POLARITY_ADDR, HallSensorPolarity);
	}
	
	
	USBSendString("TYPE THE FOLLOWING KEYS...\r");
	
//--------TEACH LETTER KEYS----------
	for (HIDKey = KEY_A; HIDKey <= KEY_Z; HIDKey ++){
		USBSend(HIDKey, UPPER);
		USBSend(KEY_SPACE,LOWER);// used to be a colon
		KeyPressed = WaitForKeypress();
		Modifier = GetModifier();
		
		Modifier &= ~(HID_KEYBOARD_MODIFIER_LEFTSHIFT); //Lower-case version of the modifier indicator (overrid user's shift key)
					
		TeachHIDKey(HIDKey, KeyPressed, Modifier); //all alphanumeric keys programmed as a/A pairs -- no special shift keys allowed.
		ASCIIKey = HIDKey-KEY_A+ASCII_A; //calculated corresponding sd card Ascii key
		
		if (!(Modifier & FN_MODIFIER)) { //only bother to program sd card letters if the fn key is not being pressed (sd card doesn't use fn key)
			TeachASCIIKey(ASCIIKey, KeyPressed, UPPER);  //program this key into memory as an upper case key
			TeachASCIIKey(ASCIIKey+0x20, KeyPressed, LOWER); // and as a lower case key
		}
		
		USBSend(KEY_ENTER,LOWER);
	}

//--------TEACH NUMBER KEYS---------
	for (int i = 0; i <= 9; i ++){
			USBSend(HIDNumbers[i], LOWER);
			USBSend(KEY_SPACE,LOWER);// used to be a colon
			KeyPressed = WaitForKeypress();
			Modifier = GetModifier();
			
			Modifier &= ~(HID_KEYBOARD_MODIFIER_LEFTSHIFT); //numbers are always lower-case (override user's shift key)
			
			TeachHIDKey(HIDNumbers[i], KeyPressed, Modifier); //teach the hid keycode array about this key -- must be lowercase.
			
			if ( !(Modifier & FN_MODIFIER)) { //if the fn key is not being pressed (sd card doesn't use fn key)
				TeachASCIIKey(ASCIINumbers[i], KeyPressed, LOWER); // and the ascii array about this key
				TeachASCIIKey(ASCIINumSymbols[i],KeyPressed, UPPER); // and the symbols above the numbers on most typewriters, for asii (sd card) use only.
			}
			
			USBSend(KEY_ENTER,LOWER);
	}
	
	//------TEACH SPACE BAR --------
	USBSendString("SPACEBAR");
	USBSend(KEY_SPACE,LOWER);
	KeyPressed = WaitForKeypress();
	TeachHIDKey(KEY_SPACE,KeyPressed,LOWER); // hid backspace
	TeachASCIIKey(' ',KeyPressed,LOWER); // ascii space key
	TeachASCIIKey(' ',KeyPressed,UPPER); // shift space is still space
	USBSend(KEY_ENTER,LOWER);
	
	//--------TEACH SHIFT KEY-----------
	USBSendString("SHIFT");
	USBSend(KEY_SPACE,LOWER);// used to be a colon
	KeyPressed = WaitForKeypress();
	if(KeyPressed)
	if(KeyPressed <= 8){ //if keypressed is 1, 2, 3, or 4, it represents a reed switch being held down.
		Shift_Reed = KeyPressed;
	}
	
//----------TEACH SYMBOL KEYS----------------	
	for (HIDKey = KEY_DASH; HIDKey <= KEY_SLASH; HIDKey ++){
			if (HIDKey != HID_KEYBOARD_SC_NON_US_HASHMARK_AND_TILDE){ //don't bother to program the non-us hash key, which is a weird and confusing key.
			USBSend(HIDKey, LOWER);
			USBSend(KEY_SPACE,LOWER);// used to be a colon
			KeyPressed = WaitForKeypress();
			Modifier = GetModifier();
			USBSendNumber(Modifier);
			
			TeachHIDKey(HIDKey, KeyPressed, Modifier); //program these characters, including the modifer used when programming them.
			
			//some of these keys (but not all) are also used for the SD card mode:			
			
			if ( !(Modifier & FN_MODIFIER)) { //if the fn key is not being pressed (sd card doesn't use fn key)
				if(HIDKey == HID_KEYBOARD_SC_SEMICOLON_AND_COLON){TeachASCIIKey(';', KeyPressed, LOWER);TeachASCIIKey(':', KeyPressed, UPPER);}
				if(HIDKey == HID_KEYBOARD_SC_EQUAL_AND_PLUS){TeachASCIIKey('=', KeyPressed, LOWER);TeachASCIIKey('+', KeyPressed, UPPER);}
				if(HIDKey == HID_KEYBOARD_SC_GRAVE_ACCENT_AND_TILDE){TeachASCIIKey('`', KeyPressed, LOWER);TeachASCIIKey('~', KeyPressed, UPPER);}
				if(HIDKey == KEY_DASH){TeachASCIIKey('-', KeyPressed, LOWER); TeachASCIIKey('*', KeyPressed, UPPER);}
				if(HIDKey == KEY_SLASH){TeachASCIIKey('/', KeyPressed, LOWER);TeachASCIIKey('?', KeyPressed, UPPER);}
				if(HIDKey == KEY_COMMA){TeachASCIIKey(',', KeyPressed, LOWER); TeachASCIIKey('?', KeyPressed, UPPER);}
				if(HIDKey == KEY_PERIOD){TeachASCIIKey('.', KeyPressed, LOWER); TeachASCIIKey('.', KeyPressed, UPPER);}
			}
				
			USBSend(KEY_ENTER,LOWER);
		}
	}
	

	
	//-------TEACH BACKSPACE KEY ---------
	USBSendString("BACKSPACE");
	USBSend(KEY_SPACE,LOWER);
	KeyPressed = WaitForKeypress();
	TeachHIDKey(KEY_BACKSPACE,KeyPressed,LOWER); // hid backspace is always lower case
	TeachASCIIKey(8,KeyPressed,LOWER); // 8 is ascii backspace
	TeachASCIIKey(8,KeyPressed,UPPER); // upper case backspace is still backspace
	USBSend(KEY_ENTER,LOWER);
	
	//------TEACH TAB KEY ---------
	USBSendString("TAB");
	USBSend(KEY_SPACE,LOWER);
	KeyPressed = WaitForKeypress();
	TeachHIDKey(KEY_TAB,KeyPressed,LOWER); // hid tab is always lower case.
	
	if ( !(Modifier & FN_MODIFIER)) { //if the fn key is not being pressed (sd card doesn't use fn key)
		TeachASCIIKey('\t',KeyPressed,LOWER); // tab key
		TeachASCIIKey('t',KeyPressed,UPPER); //tab key + shift is still tab
	}
	//sd card ascii mode does not support tab0019
	; 
	USBSend(KEY_ENTER,LOWER);
	
	//------TEACH ESC KEY ----------
	USBSendString("ESC");
	USBSend(KEY_SPACE,LOWER);
	KeyPressed = WaitForKeypress();
	TeachHIDKey(KEY_ESC,KeyPressed,LOWER); // hid backspace
	//no ascii character stored for this key.
	USBSend(KEY_ENTER,LOWER);
	
	//--------TEACH ENTER KEY --------
	USBSendString("ENTER");
	USBSend(KEY_SPACE,LOWER);// used to be a colon
	KeyPressed = WaitForKeypress();
	//Enter key cannot use modifiers
	TeachHIDKey(KEY_ENTER, KeyPressed, LOWER); //teach the hid keycode array about this key.
	TeachASCIIKey('\r',KeyPressed, LOWER);//return carriage for ascii users
	TeachASCIIKey('\r',KeyPressed,UPPER); // shift enter is still enter
	USBSend(KEY_ENTER,LOWER);
	
	//------TEACH SPACE BAR --------
	USBSendString("SPACEBAR");
	USBSend(KEY_SPACE,LOWER);
	KeyPressed = WaitForKeypress();
	TeachHIDKey(KEY_SPACE,KeyPressed,LOWER); // hid backspace
	TeachASCIIKey(' ',KeyPressed,LOWER); // ascii space key
	TeachASCIIKey(' ',KeyPressed,UPPER); // shift space is still space
	USBSend(KEY_ENTER,LOWER);
		
   //Save calibration to eeprom:
	 eeprom_write_block (KeyCodeLookUpTable, (void *) KEYCODE_ADDR, KEYCODE_ARRAY_LENGTH);
	 eeprom_write_block (FnKeyCodeLookUpTable, (void *) FN_KEYCODE_ADDR, KEYCODE_ARRAY_LENGTH);
	 eeprom_write_block (ShiftKeyCodeLookUpTable, (void *) SHIFT_KEYCODE_ADDR, KEYCODE_ARRAY_LENGTH);
	 eeprom_write_block (ASCIILookUpTable,(void *) ASCII_ADDR, KEYCODE_ARRAY_LENGTH);
	 eeprom_write_block (ASCIIShiftLookUpTable,(void *) ASCII_SHIFT_ADDR, KEYCODE_ARRAY_LENGTH);
	 
	 eeprom_update_byte ((uint8_t *)SHIFT_REED_ADDR, Shift_Reed); 	 
	 
	 eeprom_update_byte ((uint8_t *)REED_1_POLARITY_ADDR,Reed1Polarity);
	 eeprom_update_byte ((uint8_t *)REED_2_POLARITY_ADDR,Reed2Polarity);
	 eeprom_update_byte ((uint8_t *)REED_3_POLARITY_ADDR,Reed3Polarity);
	 eeprom_update_byte ((uint8_t *)REED_4_POLARITY_ADDR,Reed4Polarity);
	
	Typewriter_Mode = USB_MODE;
}

int WaitForKeypress(){
	int KeyPressed = 0;
	
	Delay_MS(100);//implement 500 MS delay before detecting a key.  (prevents rapid re-detection of keys over and over.
	
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


