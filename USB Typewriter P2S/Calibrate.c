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

const uint8_t HIDNumbers[] PROGMEM = {KEY_1,KEY_2,KEY_3,KEY_4,KEY_5,KEY_6,KEY_7,KEY_8,KEY_9,KEY_0}; //usb codes for 0-9
const uint8_t ASCIINumbers[] PROGMEM = {'1','2','3','4','5','6','7','8','9','0'}; // the ascii codes for 0-9
const uint8_t ASCIINumSymbols[] PROGMEM = {'!','\"','#','$','%','_','&','\'','(',')'};// ascii codes for weird characters above 0-9 on  typewriters.
const uint8_t hid_arrows[] PROGMEM = {HID_KEYBOARD_SC_LEFT_ARROW,HID_KEYBOARD_SC_RIGHT_ARROW,HID_KEYBOARD_SC_UP_ARROW,HID_KEYBOARD_SC_DOWN_ARROW};
const char str_left[] PROGMEM = "LEFT";
const char str_right[] PROGMEM = "RIGHT";
const char str_up[] PROGMEM = "UP";
const char str_down[] PROGMEM = "DOWN";

const char * const arrow_table[] = {str_left, str_right, str_up, str_down};
	

//messages to user are stored in program memory space, to conserve data memory (there is much more program memory than data memory)
	const char Str_USB_Typewriter[]		PROGMEM = "USB TYPEWRITER (TM)\r";
	const char Str_Firmware_Ver[]		PROGMEM = FIRMWARE_VERSION_AND_MODULE;
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
	const char Str_SD_Only[]			PROGMEM = " (DURING SD CARD MODE):\t";
	const char Str_USB_Only[]			PROGMEM = " (DURING USB/BT MODE):   \t";
	const char Str_Dummy_Load[]			PROGMEM = "DUMMY LOAD ACTIVATED\r";
	const char Str_No_Dummy_Load[]		PROGMEM = "DUMMY LOAD DEACTIVATED\r";
	const char Str_Pin_Code_Enabled[]	PROGMEM = "BLUETOOTH PIN ENABLED\r";
	const char Str_Pin_Code_Disabled[]	PROGMEM = "BLUETOOTH PIN DISABLED\r";
	const char Str_Quick_Calibrate[]	PROGMEM = "QUICK CALIBRATION MODE...\r";
	const char Str_Spacebar[]			PROGMEM = "SPACEBAR";
	const char Str_Enter[]				PROGMEM = "ENTER\t";
	const char Str_Tab[]				PROGMEM = "TAB\t";
	const char Str_Esc[]				PROGMEM = "ESC\t";
	const char Str_Second_Enter[]		PROGMEM = "SECOND ENTER";
	const char Str_Post[]				PROGMEM = "SEND\t";
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
	const char Str_Manual_Calibration[] PROGMEM = "MANUAL CALIBRATION MODE...\r";
	const char Str_U_For_USB[]   PROGMEM = "Press U if changes should apply to USB/BT mode.\r";
	const char Str_S_For_SD[]	 PROGMEM = "Press S if changes should apply to SD mode.\r";
	const char Str_How_To_Scroll[]	PROGMEM = "Press SPACEBAR to scroll through characters.\r";
	const char Str_How_To_Scroll_Back[]  PROGMEM = "Press CTRL+SPACEBAR to scroll backwards.\r";
	const char Str_How_To_Exit[] PROGMEM = "Press CMD+SPACEBAR to save changes and exit.\r";
	const char Str_Assign[]	PROGMEM = " CHARACTER SELECTED. PRESS A KEY TO ASSIGN... ";
	const char Str_Shift_Plus[] PROGMEM = "SHIFT+";
	const char Str_Reed[] PROGMEM ="REED";
	const char Str_Header[] PROGMEM = "\rCHAR:\tCONTACT#:\r";
	const char Str_BT_Not_Found[] PROGMEM = "NO BLUETOOTH MODULE DETECTED...\r";

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
	USBSend(KEY_ENTER,LOWER);
	
	
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
		default: USBSendString("NONE\r"); break;
	}
	
	USBSendPROGString(Str_Calibrating);
	
	/*Measure the reed switch polarities*/
		Reed1Polarity = is_low(REED_1); //if reed_1 is low at start of calibration, then the polarity of reed 1 is active high
		Reed2Polarity= is_low(REED_2);
		Reed3Polarity = is_low(REED_3);
		Reed4Polarity = is_low(REED_4);
		
		if(!Bluetooth_Test()){ //if there is no bluetooth module, tell the user.
			USBSendPROGString(Str_BT_Not_Found);
			Delay_MS(2000);
		}
		
		if (is_low(S2)){//hold down to activate the dummy load
				ToggleDummyLoad();
		}

		DetectHallSensor();
	

	
	USBSendPROGString(Str_Type_The_Following);
	USBSendPROGString(Str_Header);
	
//--------TEACH LETTER KEYS----------
	for (HIDKey = KEY_A; HIDKey <= KEY_Z; HIDKey ++){
		USBSend(HIDKey, UPPER);
		USBSend(KEY_TAB,LOWER);// used to be a colon
		KeyPressed = WaitForKeypress();
		Modifier = GetModifier();
		
		Modifier &= ~(HID_KEYBOARD_MODIFIER_LEFTSHIFT); //Lower-case version of the modifier indicator (overrid user's shift key)
					
		TeachHIDKey(HIDKey, KeyPressed, Modifier); //all alphanumeric keys programmed as a/A pairs -- no special shift keys allowed.
		ASCIIKey = HIDKey-KEY_A+ASCII_A; //calculated corresponding sd card Ascii key
		
		if(HIDKey  == HID_KEYBOARD_SC_L){ //if the key is l, make sure fn+l=1
			FnKeyCodeLookUpTable[KeyPressed] = KEY_1;
		}
		
		if (!(Modifier & FN_MODIFIER)) { //only bother to program sd card letters if the fn key is not being pressed (sd card doesn't use fn key)
			TeachASCIIKey(ASCIIKey, KeyPressed, UPPER);  //program this key into memory as an upper case key
			TeachASCIIKey(ASCIIKey+0x20, KeyPressed, LOWER); // and as a lower case key
		}
		
		USBSend(KEY_ENTER,LOWER);
	}

//--------TEACH NUMBER KEYS---------
	for (int i = 0; i <= 9; i ++){
			USBSend(pgm_read_byte(&HIDNumbers[i]), LOWER); //numbers are saved in program space.
			USBSend(KEY_TAB,LOWER);// used to be a colon
			KeyPressed = WaitForKeypress();
			Modifier = GetModifier();
			
			Modifier &= ~(HID_KEYBOARD_MODIFIER_LEFTSHIFT); //numbers are always lower-case (override user's shift key)
			
			TeachHIDKey(pgm_read_byte(&HIDNumbers[i]), KeyPressed, Modifier); //teach the hid keycode array about this key -- must be lowercase.
			
			if ( !(Modifier & FN_MODIFIER)) { //if the fn key is not being pressed (sd card doesn't use fn key)
				TeachASCIIKey(pgm_read_byte(&ASCIINumbers[i]), KeyPressed, LOWER); // and the ascii array about this key
				TeachASCIIKey(pgm_read_byte(&ASCIINumSymbols[i]),KeyPressed, UPPER); // and the symbols above the numbers on most typewriters, for asii (sd card) use only.
				FnKeyCodeLookUpTable[KeyPressed] = (KEY_F1+i);			}
			
			USBSend(KEY_ENTER,LOWER);
	}
	
//----TEACH F1, F11, F12
	USBSendString("F1");
	USBSend(KEY_TAB,LOWER);
	KeyPressed = WaitForKeypress();
	Modifier = GetModifier();
	TeachHIDKey(KEY_F1, KeyPressed, Modifier); 
	USBSend(KEY_ENTER,LOWER);
	
	USBSendString("F11");
	USBSend(KEY_TAB,LOWER);
	KeyPressed = WaitForKeypress();
	Modifier = GetModifier();
	TeachHIDKey(KEY_F11, KeyPressed, Modifier);
	USBSend(KEY_ENTER,LOWER);
	
	USBSendString("F12");
	USBSend(KEY_TAB,LOWER);
	KeyPressed = WaitForKeypress();
	Modifier = GetModifier();
	TeachHIDKey(KEY_F12, KeyPressed, Modifier);
	USBSend(KEY_ENTER,LOWER);	
	
//--------TEACH SHIFT KEY-----------
	USBSendString("SHIFT");
	USBSend(KEY_TAB,LOWER);// used to be a colon
	KeyPressed = WaitForKeypress();
	if((KeyPressed)&&(KeyPressed <= 8)){ //if keypressed is 1, 2, 3, or 4, it represents a reed switch being held down.
		Shift_Reed = KeyPressed;
		USBSendPROGString(Str_Reed);
		USBSendNumber(Shift_Reed);
	}
	else{
		USBSendPROGString(Str_Shift_Error);
	}
	Delay_MS(CALIBRATION_DELAY);
	USBSend(KEY_ENTER,LOWER);
	
//----------TEACH SYMBOL KEYS------------	
	for (HIDKey = KEY_DASH; HIDKey <= KEY_SLASH; HIDKey ++){
			if (HIDKey != HID_KEYBOARD_SC_NON_US_HASHMARK_AND_TILDE){ //don't bother to program the non-us hash key, which is a weird and confusing key.
			USBSend(HIDKey, LOWER);
			USBSend(KEY_TAB,LOWER);// used to be a colon
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
	
//=======TEACH ARROW KEYS========

	
for(uint8_t i=0;i<4;i++){
	USBSendPROGString(arrow_table[i]);
	USBSend(KEY_TAB,LOWER);// used to be a colon
	KeyPressed = WaitForKeypress();
	Modifier = GetModifier();
	TeachHIDKey(pgm_read_byte(&hid_arrows[i]),KeyPressed,Modifier);
	USBSend(KEY_ENTER,LOWER);
}


//-------TEACH VARIOUS UPPER CASE SYMBOLS ---------
	//!
	USBSend(KEY_1|FORCE_UPPER,UPPER);
	USBSend(KEY_TAB,LOWER);
	KeyPressed = WaitForKeypress();
	Modifier = GetModifier();

	if(!(Modifier&FN_MODIFIER&UPPER)){ //don't bother dealing with complicated combinations of FN and Shift to produce an !
		TeachHIDKey(KEY_1|FORCE_UPPER,KeyPressed,Modifier);
		TeachASCIIKey('!',KeyPressed,Modifier);
	}
	
	USBSend(KEY_ENTER,LOWER);


	//@ for sd
	USBSend(KEY_2,UPPER);
	USBSendPROGString(Str_SD_Only);
	KeyPressed = WaitForKeypress();
	Modifier = GetModifier();
	
		if(Modifier==HID_KEYBOARD_MODIFIER_LEFTSHIFT){
			USBSendString("SHIFT");
			USBSend(KEY_EQ,UPPER); //send a + sign
		}
	USBSendNumber(KeyPressed);
	USBSend(KEY_ENTER,LOWER);
	
	Delay_MS(CALIBRATION_DELAY);//delay between programming keys.
	
	//@ for usb
	USBSend(KEY_2,UPPER);
	USBSendPROGString(Str_USB_Only);
	KeyPressed = WaitForKeypress();
	Modifier = GetModifier();

	TeachHIDKey(KEY_2|FORCE_UPPER,KeyPressed,Modifier);
	USBSend(KEY_ENTER,LOWER);
	
	//?
	USBSend(KEY_SLASH,UPPER);
	USBSendPROGString(Str_SD_Only);
	KeyPressed = WaitForKeypress();
	Modifier = GetModifier();
	TeachASCIIKey('?',KeyPressed,Modifier);
	if(Modifier==HID_KEYBOARD_MODIFIER_LEFTSHIFT){
		USBSendString("SHIFT");
		USBSend(KEY_EQ,UPPER); //send a + sign
	}
	USBSendNumber(KeyPressed);
	USBSend(KEY_ENTER,LOWER);
	
	
	Delay_MS(CALIBRATION_DELAY);//delay between programming keys.
	
//for USB

	USBSend(KEY_SLASH,UPPER);
	USBSendPROGString(Str_USB_Only);
	KeyPressed = WaitForKeypress();
	Modifier = GetModifier();

	TeachHIDKey(KEY_SLASH|FORCE_UPPER,KeyPressed,Modifier);
	USBSend(KEY_ENTER,LOWER);
	

	
	
//------TEACH REED SWITCHES--------//
	CalibrateReeds();
	
	SaveCalibration();
	
	USBSendPROGString(Str_Settings_Saved);
	
}

void ToggleDummyLoad(){
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

void QuickCalibrate(){
	uint8_t KeyPressed; //

	while(USB_DeviceState != DEVICE_STATE_Configured){;}//wait for configuration to complete
	Delay_MS(1000);//wait 1 second.
	
	USBSendPROGString(Str_Quick_Calibrate);
			
	if(!Bluetooth_Test()){ //if there is no bluetooth module, tell the user.
		USBSendPROGString(Str_BT_Not_Found);
		Delay_MS(2000);
	}
	
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
	USBSend(KEY_TAB,LOWER);// used to be a colon
	KeyPressed = WaitForKeypress();
	if((KeyPressed)&&(KeyPressed <= 8)){ //if keypressed is 1, 2, 3, or 4, it represents a reed switch being held down.
		Shift_Reed = KeyPressed;
		USBSendPROGString(Str_Reed);
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

#define CODE_IS_SKIPPED (code == 129)||(code == 130)||(code==132)||(code==141)||(code==155)||(code==157)||(code==160)||(code==168)||(code == 175)||(code == 180)||(code== 173)||(code== 183)||(code== 184)

//increment the code to program during manual calibration mode. In USB mode, if code is Enter or space or other character in between, skip it.  
#define INCREMENT_CODE() {\
		code++;\
		if ((code == 0)||(code > codeend2)) {code = codestart1;}\
		else if((code > codeend1) && (code < codestart2)){code = codestart2;}\
		USBSend(KEY_ENTER,LOWER);\
		if (edit_mode == 'u'){\
			while (((code|FORCE_UPPER) >= (KEY_ENTER|FORCE_UPPER)) && ((code|FORCE_UPPER) <= (KEY_SPACE|FORCE_UPPER))){code++;}\
			USBSend(code,LOWER);\
		}\
		else {\
			if(code == 'A'){code = 'Z'+1;}\
			if(code == '0'){code = '9'+1;}\
			while(CODE_IS_SKIPPED){code ++;}\
			if(code == 146){code = 155;}\
			USBSendASCII(code);\
		}\
}

#define DECREMENT_CODE() {\
	if(code <= codestart1){code=codeend2;}\
	else if((code <= codestart2)&&(code>codeend1)){code = codeend1;}\
	else{code--;}\
	USBSend(KEY_ENTER,LOWER);\
	if (edit_mode == 'u'){\
		while (((code|FORCE_UPPER) >= (KEY_ENTER|FORCE_UPPER)) && ((code|FORCE_UPPER) <= (KEY_SPACE|FORCE_UPPER))){code--;}\
		USBSend(code,LOWER);\
	}\
	else {\
		if(code == 'Z'){code = 'A'-1;}\
		if(code == '9'){code = '0'-1;}\
		if(code == 154){code = 145;}\
		while(CODE_IS_SKIPPED){code --;}\
		USBSendASCII(code);\
	}\
}


void Calibrate_Manually(){
	

	uint8_t code;
	uint8_t codestart1;
	uint8_t codeend1;
	uint8_t codestart2;
	uint8_t codeend2;
	uint8_t keypressed;
	uint8_t modifier;
	char edit_mode = 's';
	
	while(USB_DeviceState != DEVICE_STATE_Configured){;}//wait for configuration to complete
	Delay_MS(1000);
		
	//tell user what is up
	USBSendPROGString(Str_Manual_Calibration);
	USBSendPROGString(Str_U_For_USB);
	USBSendPROGString(Str_S_For_SD);
	edit_mode = Get_User_Response();
	USBSendString("OK\r");
	USBSendPROGString(Str_How_To_Scroll);
	USBSendPROGString(Str_How_To_Scroll_Back);
	USBSendPROGString(Str_How_To_Exit);
	
	//set ranges of ascii/hid codes over which to calibrate.  there are two ranges.
	if(edit_mode == 'u'){
		codestart1 = 0x2D;
		codeend1 = 0x38;
		codestart2 = 0x1E|FORCE_UPPER;
		codeend2 = 0x38|FORCE_UPPER;
	}
	else{
		codestart1 = 0x21;
		codeend1 = 142;// see http://www.peterstagg.com/blog/wp-content/uploads/2012/05/letter-spagetti.png
		codestart2 = 153;
		codeend2 = 0xFF;
	}
	
	code = codestart1;
	if (edit_mode == 'u'){USBSend(code,LOWER);}
	else {USBSendASCII(code);}
	
	while(1){			
			keypressed = WaitForKeypress();
			modifier = GetModifier();
			if(KeyCodeLookUpTable[keypressed]!=KEY_SPACE){//if the key pressed is not spacebar, program the code -- otherwise skip to next line
				if (edit_mode == 'u'){
					TeachHIDKey(code,keypressed,modifier);
				}
				else if (edit_mode=='s'){
					TeachASCIIKey(code,keypressed,modifier);
					if(modifier & HID_KEYBOARD_MODIFIER_LEFTSHIFT){USBSendPROGString(Str_Shift_Plus);}
					USBSendNumber(keypressed);
					if((code >= 'a') && (code <='z')){
						TeachASCIIKey(code-'a'+'A',keypressed,modifier|HID_KEYBOARD_MODIFIER_LEFTSHIFT);//program upper case letters too
					}
				}
			INCREMENT_CODE();			
			Delay_MS(CALIBRATION_DELAY);
			}
			else{ //if key pressed is spacebar
				if (is_low(CTRL_KEY)){DECREMENT_CODE();}
				else if (is_low(CMD_KEY)){break;}
				else {INCREMENT_CODE();}
			}
	}
	
	SaveCalibration(); //save your work.
	USBSendPROGString(Str_Settings_Saved);
	
}


void SaveCalibration(){
		 USBSendString("SAVING...\r");
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
	USBSend(KEY_TAB,LOWER);
	KeyPressed = WaitForKeypress();
	Modifier = GetModifier();
	
	TeachHIDKey(KEY_BACKSPACE,KeyPressed,Modifier); 
	if (!(Modifier & FN_MODIFIER)){//sd card does not use fn modifier.
		TeachASCIIKey(8,KeyPressed,LOWER); // 8 is ascii backspace
	}
	USBSend(KEY_ENTER,LOWER);
	
//------TEACH ESC KEY ----------
	USBSendPROGString(Str_Esc);
	USBSend(KEY_TAB,LOWER);
	KeyPressed = WaitForKeypress();
	Modifier = GetModifier();
	TeachHIDKey(KEY_ESC,KeyPressed,Modifier); 
	//no ascii character stored for this key.
	USBSend(KEY_ENTER,LOWER);
	
	//------TEACH TAB KEY ---------
	USBSendPROGString(Str_Tab);
	USBSend(KEY_TAB,LOWER);
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
	USBSend(KEY_TAB,LOWER);// used to be a colon
	KeyPressed = WaitForKeypress();
	
	//Enter key cannot use modifiers
	TeachHIDKey(KEY_ENTER, KeyPressed, LOWER); //teach the hid keycode array about this key.
	TeachASCIIKey('\r',KeyPressed, LOWER);//return carriage for ascii users
	USBSend(KEY_ENTER,LOWER);
	
//-------TEACH SECONDARY ENTER KEY ----------
	
	USBSendPROGString(Str_Second_Enter);
	USBSend(KEY_TAB,LOWER);// used to be a colon
	KeyPressed = WaitForKeypress(KEY_EXECUTE);
	
	//Enter key cannot use modifiers
	TeachHIDKey(KEY_ENTER, KeyPressed, LOWER); //teach the hid keycode array about this key.
	USBSend(KEY_ENTER,LOWER);
	
// -------TEACH SEND KEY --------------
	USBSendPROGString(Str_Post);
	USBSend(KEY_TAB,LOWER);// used to be a colon
	KeyPressed = WaitForKeypress();
	TeachHIDKey(KEY_EXECUTE, KeyPressed, LOWER); //teach the hid keycode array about this key.
	USBSend(KEY_ENTER,LOWER);
	
//------TEACH SPACE BAR ---- must be the last thing programmed (because of "Press space to skip" instruction)
	USBSendPROGString(Str_Spacebar);
	USBSend(KEY_TAB,LOWER);
	KeyPressed = WaitForKeypress();
	
	TeachHIDKey(KEY_SPACE,KeyPressed,LOWER); //space bar is independent of modifier.
	TeachASCIIKey(' ',KeyPressed,LOWER); // ascii space key
	USBSend(KEY_ENTER,LOWER);
	
}

bool DetectHallSensor(){
	bool HallSensorTest1;
	bool HallSensorTest2;

	
			UseHallSensor = HALL_NOT_PRESENT;  //make sure the hall effect bit is not cleared as soon as it is read.
			HallSensorTest1 = getHallState(); //sample the hall effect sensor bit
			USBSendPROGString(Str_Calibrate_Hall);
			Delay_MS(4000);
			HallSensorTest2 = getHallState(); //sample it again
			if (HallSensorTest1 == HallSensorTest2){ //if it has not changed, hall effect sensor is not present.
				USBSendPROGString(Str_No_Hall);
				UseHallSensor = HALL_NOT_PRESENT;
			}
			else{ // if it has changed, the "active" polarity of the hall effect sensor is HallSensorTest2
				USBSendString("Hall Sensor Detected!\r");
				UseHallSensor = HALL_NOT_ACTIVE;
				HallSensorPolarity = HallSensorTest2;
				USBSendString("Press CTRL key now to activate Hall Sensor...");
				for (int i=1; i < 100; i++){
					if(is_low(CTRL_KEY)) UseHallSensor = HALL_ACTIVE; 
					Delay_MS(30);
				}
				if (UseHallSensor == HALL_ACTIVE) USBSendString("\rHall Sensor Activated!\r");
			}
			
			if(is_low(CMD_KEY)){
				UseHallSensor = HALL_NOT_ACTIVE; //In the event that the hall sensor is acting erratically or not at all, holding CMD can force it off.
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
		USBSendPROGString(Str_Shift_Plus);
	}
	else if (Modifier == HID_KEYBOARD_MODIFIER_LEFTALT){ //if FN is being held down,
		FnKeyCodeLookUpTable[keypressed] = teachkey;
		//send "FN+number"
		USBSendString("FN");
		USBSend(KEY_EQ,UPPER); //send a + sign
	}
	else{
		KeyCodeLookUpTable[keypressed] = teachkey;
	}
	
	if(keypressed <= NUM_REED_SWITCHES+1){USBSendPROGString(Str_Reed);}
	USBSendNumber(keypressed);
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

char Get_User_Response(){
	uint8_t code;
	while(1){
		code = WaitForKeypress();
		if ((ASCIILookUpTable[code]	== 'u')||(ASCIILookUpTable[code]	== 's')){
			break;
		}
	}
	return ASCIILookUpTable[code];
}


