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

const uint8_t HIDNumbers[] = {KEY_1,KEY_2,KEY_3,KEY_4,KEY_5,KEY_6,KEY_7,KEY_8,KEY_9,KEY_0}; //usb codes for 0-9
const uint8_t ASCIINumbers[] = {'1','2','3','4','5','6','7','8','9','0'}; // the ascii codes for 0-9
const uint8_t ASCIINumSymbols[] = {'!','\"','#','$','%','_','&','\'','(',')'};// ascii codes for weird characters above 0-9 on  typewriters.
	

//messages to user are stored in program memory space, to conserve data memory (there is much more program memory than data memory)
	const char Str_USB_Typewriter[]		PROGMEM = "USB TYPEWRITER (TM)\r";
	const char Str_Firmware_Ver[]		PROGMEM = "FIRMWARE VER 5.0.2\r";
	const char Str_Typewriter_Mode[]	PROGMEM = "DEFAULT SETTING: ";
	const char Str_BT_Mode[]			PROGMEM = "BLUETOOTH KEYBOARD MODE\r";
	const char Str_Light_Mode[]			PROGMEM = "LIGHT MODE (SD CARD READER DISABLED)\r";
	const char Str_Combo_Mode[]			PROGMEM = "USB KEYBOARD/CARD READER MODE\r";
	const char Str_SD_Mode[]			PROGMEM = "SD CARD STORAGE MODE\r";
	const char Str_SD_Detected[]		PROGMEM = "SD CARD DETECTED\r";
	const char Str_SD_Not_Detected[]	PROGMEM = "SD CARD NOT DETECTED\r";
	const char Str_Calibrating[]		PROGMEM = "CALIBRATING...\r";
	const char Str_Type_The_Following[] PROGMEM = "TYPE THE FOLLOWING KEYS (PRESS SPACE TO SKIP)...\r";
	const char Str_Shift_Error[]		PROGMEM = "ERROR...SHIFT MUST BE A REED SWITCH.\r";
	const char Str_SD_Only[]			PROGMEM = " (SD MODE)";
	const char Str_USB_Only[]			PROGMEM = " (USB MODE)";
	const char Str_Dummy_Load[]			PROGMEM = "DUMMY LOAD ACTIVATED\r";
	const char Str_No_Dummy_Load[]		PROGMEM = "DUMMY LOAD DEACTIVATED\r";
	const char Str_Quick_Calibrate[]	PROGMEM = "QUICK CALIBRATION MODE...\r";
	const char Str_Spacebar[]			PROGMEM = "SPACEBAR";
	const char Str_Enter[]				PROGMEM = "ENTER";
	const char Str_Backspace[]			PROGMEM = "BACKSPACE";
	const char Str_Calibrate_Hall[]		PROGMEM = "HOLD DOWN ANY KEY TO CALIBRATE HALL EFFECT SENSOR...\r";
	const char Str_No_Hall[]			PROGMEM = "NO HALL EFFECT SENSOR DETECTED. (NOT A PROBLEM)\r";
	const char Str_Adj_Sensitivity[]	PROGMEM = "ADJUSTING KEY SENSITIVITY.\r";
	const char Str_Press_CMD[]			PROGMEM = "PRESS CMD KEY TO CONTINUE...\r";
	const char Str_Set_Reaction_Time[]	PROGMEM = "PRESS CTRL AND ALT TO SET KEY REACTION TIME...\r";
	const char Str_Set_Release_Time[]	PROGMEM = "\rNOW SET KEY RELEASE TIME...\r";
	const char Str_Set_Double_Time[]	PROGMEM = "\rNOW SET DELAY BETWEEN DOUBLE KEY PRESSES...\r";
	const char Str_Set_Reed_Time[]		PROGMEM = "\rNOW SET REED REACTION TIME...\r";
	const char Str_Spacebar_Blocks_Enter[] PROGMEM = "\rIGNORE ENTER KEY WHEN SPACEBAR IS HELD?\r";
	const char Str_Settings_Saved[]		PROGMEM = "SETTINGS SAVED!\r";
	

void Calibrate(){
	uint8_t ASCIIKey;
	uint8_t HIDKey;
	uint8_t Modifier;
	uint8_t KeyPressed;
	
	uint8_t Default_Mode;

	ClearKeyCodeTables();//clear keycode tables (in memory)

	while(USB_DeviceState != DEVICE_STATE_Configured){;}//wait for configuration to complete
	Delay_MS(1000);//wait 1 second.
	
//--------SEND PROMPT TO USER -----------

    USBSendPROGString(Str_USB_Typewriter);
	USBSendPROGString(Str_Firmware_Ver);
	
	
	Default_Mode = eeprom_read_byte((uint8_t*)DEFAULT_MODE_ADDR);
	USBSendPROGString(Str_Typewriter_Mode);
	switch(Default_Mode){
		case USB_COMBO_MODE: 
			USBSendPROGString(Str_Combo_Mode); 
		break;
		case BLUETOOTH_MODE: 
			USBSendPROGString(Str_BT_Mode); 
		break;
		case SD_MODE: 
			USBSendPROGString(Str_SD_Mode);
		break;
		case USB_LIGHT_MODE:
			USBSendPROGString(Str_Light_Mode);
		break;
		default: USBSendString("Unknown"); break;
	}
	
	USBSendPROGString(Str_Calibrating);
	
	/*Measure the reed switch polarities*/
		Reed1Polarity = is_low(REED_1); //if reed_1 is low at start of calibration, then the polarity of reed 1 is active high
		Reed2Polarity= is_low(REED_2);
		Reed3Polarity = is_low(REED_3);
		Reed4Polarity = is_low(REED_4);
	
	if (is_low(CMD_KEY)){ //holding this key down after the initial message is a hidden way to activate the hall effect sensor
		DetectHallSensor();
	}
	
	if (is_low(S2)){//hold down to activate the dummy load
		if (UseDummyLoad) {
			UseDummyLoad = 0; 
			USBSendPROGString(Str_No_Dummy_Load);
		}
		else {
		UseDummyLoad = 1; 
		USBSendPROGString(Str_Dummy_Load);
		}
		eeprom_update_byte((uint8_t*)DUMMY_LOAD_ADDR, UseDummyLoad);
	}
	
	USBSendPROGString(Str_Type_The_Following);
	
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
				FnKeyCodeLookUpTable[KeyPressed] = (KEY_F1+i);			}
			
			USBSend(KEY_ENTER,LOWER);
	}
	
//----TEACH F1, F11, F12
	USBSendString("F1");
	USBSend(KEY_SPACE,LOWER);
	KeyPressed = WaitForKeypress();
	Modifier = GetModifier();
	TeachHIDKey(KEY_F1, KeyPressed, Modifier); 
	USBSend(KEY_ENTER,LOWER);
	
	USBSendString("F11");
	USBSend(KEY_SPACE,LOWER);
	KeyPressed = WaitForKeypress();
	Modifier = GetModifier();
	TeachHIDKey(KEY_F11, KeyPressed, Modifier);
	USBSend(KEY_ENTER,LOWER);
	
	USBSendString("F12");
	USBSend(KEY_SPACE,LOWER);
	KeyPressed = WaitForKeypress();
	Modifier = GetModifier();
	TeachHIDKey(KEY_F12, KeyPressed, Modifier);
	USBSend(KEY_ENTER,LOWER);	
	
//--------TEACH SHIFT KEY-----------
	USBSendString("SHIFT");
	USBSend(KEY_SPACE,LOWER);// used to be a colon
	KeyPressed = WaitForKeypress();
	if((KeyPressed)&&(KeyPressed <= 8)){ //if keypressed is 1, 2, 3, or 4, it represents a reed switch being held down.
		Shift_Reed = KeyPressed;
		USBSendNumber(Shift_Reed);
	}
	else{
		USBSendPROGString(Str_Shift_Error);
	}
	USBSend(KEY_ENTER,LOWER);
	
//----------TEACH SYMBOL KEYS------------	
	for (HIDKey = KEY_DASH; HIDKey <= KEY_SLASH; HIDKey ++){
			if (HIDKey != HID_KEYBOARD_SC_NON_US_HASHMARK_AND_TILDE){ //don't bother to program the non-us hash key, which is a weird and confusing key.
			USBSend(HIDKey, LOWER);
			USBSend(KEY_SPACE,LOWER);// used to be a colon
			KeyPressed = WaitForKeypress();
			Modifier = GetModifier();
			
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
	
//-------TEACH VARIOUS UPPER CASE SYMBOLS ---------
	//@ for sd
	USBSend(KEY_2,UPPER);
//	USBSendPROGString(Str_SD_Only);
	USBSend(KEY_SPACE,LOWER);
	KeyPressed = WaitForKeypress();
	Modifier = GetModifier();

	TeachASCIIKey('@',KeyPressed,Modifier);
	USBSendNumber(KeyPressed);
	USBSend(KEY_ENTER,LOWER);
	
/*	//@ for usb
	USBSend(KEY_2,UPPER);
	USBSendPROGString(Str_USB_Only);
	KeyPressed = WaitForKeypress();
	Modifier = GetModifier();

	TeachHIDKey(KEY_2|FORCE_UPPER,KeyPressed,Modifier);
	USBSend(KEY_ENTER,LOWER);*/
	
	//?
	USBSend(KEY_SLASH,UPPER);
//	USBSendPROGString(Str_SD_Only);
	USBSend(KEY_SPACE,LOWER);
	KeyPressed = WaitForKeypress();
	Modifier = GetModifier();
	TeachASCIIKey('?',KeyPressed,Modifier);
	USBSendNumber(KeyPressed);
	USBSend(KEY_ENTER,LOWER);
	
//for USB
/*
	USBSend(KEY_SLASH,UPPER);
	USBSendPROGString(Str_USB_Only);
	KeyPressed = WaitForKeypress();
	Modifier = GetModifier();

	TeachHIDKey(KEY_SLASH|FORCE_UPPER,KeyPressed,Modifier);
	USBSend(KEY_ENTER,LOWER);*/
	
	//!
	USBSend(KEY_1|FORCE_UPPER,UPPER);
	USBSend(KEY_SPACE,LOWER);
	KeyPressed = WaitForKeypress();
	Modifier = GetModifier();

	if(!(Modifier&FN_MODIFIER&UPPER)){ //don't bother dealing with complicated combinations of FN and Shift to produce an !
		TeachHIDKey(KEY_1|FORCE_UPPER,KeyPressed,Modifier);
		TeachASCIIKey('!',KeyPressed,Modifier);
	}
	USBSend(KEY_ENTER,LOWER);
	
//------TEACH REED SWITCHES--------//
	CalibrateReeds();
	
	SaveCalibration();
	
	USBSendPROGString(Str_Settings_Saved);
	
}

void QuickCalibrate(){
	uint8_t KeyPressed; //
	
	
	while(USB_DeviceState != DEVICE_STATE_Configured){;}//wait for configuration to complete
	Delay_MS(1000);//wait 1 second.
	
	USBSendPROGString(Str_Quick_Calibrate);
	
	/*Measure the reed switch polarities*/
	Reed1Polarity = is_low(REED_1); //if reed_1 is low at start of calibration, then the polarity of reed 1 is active high
	Reed2Polarity= is_low(REED_2);
	Reed3Polarity = is_low(REED_3);
	Reed4Polarity = is_low(REED_4);
	
	
	
	//Find out if user wants to use the hall effect sensor (probably not).
	DetectHallSensor();

	USBSendPROGString(Str_Type_The_Following);

	//--------TEACH SHIFT KEY-----------
	USBSendString("SHIFT");
	Shift_Reed = 0; //reset the shift reed to 0 (undefined) so that WaitForKeypress() doesn't ignore any of the reeds
	USBSend(KEY_SPACE,LOWER);// used to be a colon
	KeyPressed = WaitForKeypress();
	if((KeyPressed)&&(KeyPressed <= 8)){ //if keypressed is 1, 2, 3, or 4, it represents a reed switch being held down.
		Shift_Reed = KeyPressed;
		USBSendNumber(Shift_Reed);
	}
	else{
		USBSendPROGString(Str_Shift_Error);
	}
	USBSend(KEY_ENTER,LOWER);
	
	CalibrateReeds();
	
	SaveCalibration();
	
	USBSendPROGString(Str_Settings_Saved);
	
}

void SaveCalibration(){
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
}

void CalibrateReeds(){	
	uint8_t Modifier;
	uint8_t KeyPressed;
	
//-------TEACH BACKSPACE KEY ---------
	USBSendPROGString(Str_Backspace);
	USBSend(KEY_SPACE,LOWER);
	KeyPressed = WaitForKeypress();
	Modifier = GetModifier();
	
	TeachHIDKey(KEY_BACKSPACE,KeyPressed,Modifier); 
	if (!(Modifier & FN_MODIFIER)){//sd card does not use fn modifier.
		TeachASCIIKey(8,KeyPressed,LOWER); // 8 is ascii backspace
	}
	USBSend(KEY_ENTER,LOWER);
	
//------TEACH ESC KEY ----------
	USBSendString("ESC");
	USBSend(KEY_SPACE,LOWER);
	KeyPressed = WaitForKeypress();
	Modifier = GetModifier();
	TeachHIDKey(KEY_ESC,KeyPressed,Modifier); 
	//no ascii character stored for this key.
	USBSend(KEY_ENTER,LOWER);
	
	//------TEACH TAB KEY ---------
	USBSendString("TAB");
	USBSend(KEY_SPACE,LOWER);
	KeyPressed = WaitForKeypress();
	Modifier = GetModifier();
	TeachHIDKey(KEY_TAB,KeyPressed,Modifier);
	
	if ( !(Modifier & FN_MODIFIER)) { //if the fn key is not being pressed (sd card doesn't use fn key)
		TeachASCIIKey('\t',KeyPressed,LOWER); // tab key
		TeachASCIIKey('t',KeyPressed,UPPER); //tab key + shift is still tab
	}

	USBSend(KEY_ENTER,LOWER);
	
//--------TEACH ENTER KEY --------
	USBSendPROGString(Str_Enter);
	USBSend(KEY_SPACE,LOWER);// used to be a colon
	KeyPressed = WaitForKeypress();
	
	//Enter key cannot use modifiers
	TeachHIDKey(KEY_ENTER, KeyPressed, LOWER); //teach the hid keycode array about this key.
	TeachASCIIKey('\r',KeyPressed, LOWER);//return carriage for ascii users
	USBSend(KEY_ENTER,LOWER);
	
//------TEACH SPACE BAR ---- must be the last thing programmed (because of "Press space to skip" instruction)
	USBSendPROGString(Str_Spacebar);
	USBSend(KEY_SPACE,LOWER);
	KeyPressed = WaitForKeypress();
	
	TeachHIDKey(KEY_SPACE,KeyPressed,LOWER); //space bar is independent of modifier.
	TeachASCIIKey(' ',KeyPressed,LOWER); // ascii space key
	USBSend(KEY_ENTER,LOWER);
	
}

bool DetectHallSensor(){
	bool HallSensorTest1;
	bool HallSensorTest2;
	
			HallSensorTest1 = getHallState(); //sample the hall effect sensor bit
			USBSendPROGString(Str_Calibrate_Hall);
			Delay_MS(4000);
			HallSensorTest2 = getHallState(); //sample it again
			if (HallSensorTest1 == HallSensorTest2){ //if it has not changed, hall effect sensor is not present.
				USBSendPROGString(Str_No_Hall);
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
			
			return UseHallSensor;
}

int WaitForKeypress(){
	int KeyPressed = 0;
	
	Delay_MS(200);//implement 500 MS delay before detecting a key.  (prevents rapid re-detection of keys over and over.
	
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
	Delay_MS(CALIBRATION_DELAY);//delay between programming keys.
}

void TeachASCIIKey(char teachkey, int keypressed, char Modifier){
	if (Modifier == UPPER){
		ASCIIShiftLookUpTable[keypressed] = teachkey;
	}
	else if (Modifier & FN_MODIFIER){}//do nothing -- sd cards do not use fn key.
	else{
		ASCIILookUpTable[keypressed] = teachkey;
	}
}

void Adjust_Sensitivity(){
	KeyHoldTime = eeprom_read_byte((uint8_t*)HOLD_TIME_ADDR);
	KeyReleaseTime = eeprom_read_byte((uint8_t*)RELEASE_TIME_ADDR);
	DoubleTapTime= eeprom_read_byte((uint8_t*)DOUBLE_TAP_ADDR);
	
	while(USB_DeviceState != DEVICE_STATE_Configured){;}//wait for configuration to complete
	Delay_MS(1000);//wait 1 second.
	
	USBSendPROGString(Str_Adj_Sensitivity);
	USBSendPROGString(Str_Press_CMD);
	while(is_high(S3)){;}
	
	USBSendPROGString(Str_Set_Reaction_Time);
	USBSendNumber(KeyHoldTime);
	USBSend(KEY_ENTER,LOWER);
	while(is_high(S3)){
		if(is_low(S1)){KeyHoldTime++;USBSendNumber(KeyHoldTime);USBSend(KEY_ENTER,LOWER);}
		if(is_low(S2)){KeyHoldTime--;USBSendNumber(KeyHoldTime);USBSend(KEY_ENTER,LOWER);}
	}
	
	USBSendPROGString(Str_Set_Release_Time);
	USBSendNumber(KeyReleaseTime);
	USBSend(KEY_ENTER,LOWER);
	while(is_high(S3)){
		if(is_low(S1)){KeyReleaseTime++;USBSendNumber(KeyReleaseTime);USBSend(KEY_ENTER,LOWER);}
		if(is_low(S2)){KeyReleaseTime--;USBSendNumber(KeyReleaseTime);USBSend(KEY_ENTER,LOWER);}
	}
	
	USBSendPROGString(Str_Set_Double_Time);
	USBSendNumber(DoubleTapTime);
	USBSend(KEY_ENTER,LOWER);
	while(is_high(S3)){
		if(is_low(S1)){DoubleTapTime++;USBSendNumber(DoubleTapTime);USBSend(KEY_ENTER,LOWER);}
		if(is_low(S2)){DoubleTapTime--;USBSendNumber(DoubleTapTime);USBSend(KEY_ENTER,LOWER);}
	}
	
	USBSendPROGString(Str_Set_Reed_Time);
	USBSendNumber(ReedHoldTime);
	USBSend(KEY_ENTER,LOWER);
	while(is_high(S3)){
		if(is_low(S1)){ReedHoldTime++;USBSendNumber(ReedHoldTime);USBSend(KEY_ENTER,LOWER);}
		if(is_low(S2)){ReedHoldTime--;USBSendNumber(ReedHoldTime);USBSend(KEY_ENTER,LOWER);}
	}	
	
	USBSendPROGString(Str_Spacebar_Blocks_Enter);
	Reeds_Are_Independent ? USBSendString("NO\r") : USBSendString("YES\r");
	while(is_high(S3)){
		if(is_low(S1)){Reeds_Are_Independent = false; USBSendString("YES\r");}
		if(is_low(S2)){Reeds_Are_Independent = true; USBSendString("NO\r");}
		while(is_low(S1)||is_low(S2)){;}//wait until one of the switches is released before looping.
	}
	
	eeprom_update_byte((uint8_t*)DOUBLE_TAP_ADDR,DoubleTapTime);
	eeprom_update_byte((uint8_t*)RELEASE_TIME_ADDR,KeyReleaseTime);
	eeprom_update_byte((uint8_t*)HOLD_TIME_ADDR,KeyHoldTime);
	eeprom_update_byte((uint8_t*)REED_HOLD_TIME_ADDR,ReedHoldTime);
	eeprom_update_byte((uint8_t*)REEDS_INDEPENDENT_ADDR,Reeds_Are_Independent);
	
	USBSendPROGString(Str_Settings_Saved);
}


