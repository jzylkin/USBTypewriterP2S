/*
 * Sense_Keys.c
 *
 * Created: 12/10/2014 10:30:42 AM
 *  Author: Jack
 */ 
#include "Keyboard.h"
#include "Sense_Keys.h"
#include "KeyCodes.h"
#include "globals.h"
//extern USB_KeyboardReport_Data_t* KeyBuffer;

const uint8_t REED_BITS[] = {62,61,60,59}; //these are the bits of the sensor array that represent the reed switches


uint8_t GetModifier(){
	uint8_t Modifier = 0;
	bool ShiftIsPressed = false;
	
	switch (Shift_Reed){
	case 0: ShiftIsPressed = false; break;// if shift reed is 0, no shift key has been programed;
	case 1: ShiftIsPressed = (is_high(REED_1) == Reed1Polarity); break; //test if reed_1 is at the active level (high/low) indicated by reed1polarity
	case 2: ShiftIsPressed = (is_high(REED_2) == Reed2Polarity); break;
	case 3: ShiftIsPressed = (is_high(REED_3) == Reed3Polarity); break;
	case 4: ShiftIsPressed = (is_high(REED_4) == Reed4Polarity); break;
	default: ShiftIsPressed = false;
	}
	
	if(ShiftIsPressed) {Modifier |= HID_KEYBOARD_MODIFIER_LEFTSHIFT;}
	if(is_low(CTRL_KEY)) {Modifier |= HID_KEYBOARD_MODIFIER_LEFTCTRL;}
	if(is_low(ALT_KEY)) {Modifier |= HID_KEYBOARD_MODIFIER_LEFTALT;}
	if(is_low(CMD_KEY)) {Modifier |= HID_KEYBOARD_MODIFIER_LEFTGUI;}
	
	return Modifier;
}

uint8_t GetKeySimple(){
	unsigned long long SensorReadout; //create 64-bit "long long" binary variable and set it all to 0s.
	uint8_t Key;
	
	Key=0;//by default, there is no key to send, unless one is detected later.
	
	SensorReadout = ReadSensor();
	SensorReadout &= ~LONGLONGBIT(HALL_SENSOR_BIT); //discard the hall effect bit of the array -- it is not an actual key, so don't report it as one.

	if(SensorReadout){
		Key = (uint8_t) __builtin_clzll(SensorReadout); //this function finds the first nonzero bit in the bitfield SensorReadout (by counting leading zeros)
	}
	else{
		Key = 0;
	}
	return  Key;
	
}
	
uint8_t GetKey(){
		unsigned long long SensorReadout; //create 64-bit "long long" binary variable and set it all to 0s.
		uint8_t DetectedKey; //the key we detect this round
		
		static uint8_t PreviousKey;
		static uint8_t ActiveKey; //the last key that was confirmed and detected and reported as a successful keypress.
		static uint8_t DoubleTapCounter;
		static uint8_t KeyHoldCounter;
		static uint8_t KeyReleaseCounter;
		static bool ActiveReeds[4];//array showing all currently active reeds.
		static uint8_t ReedDebounce[4]; //array tracking the debounce times of all four reed switches.  
		
		bool OKtoSendKey = false; //non static bool that gives the go-ahead to send a key over the usb/bluetooth bus.  Indicates that the key has been pressed first time.
		bool OKtoSendReed = false; //go ahead to send a reed switch
		
		uint8_t ReedToSend = 0;
	
/*READ INPUT FROM SENSOR STRIP*/	
		SensorReadout = ReadSensor();
		SensorReadout &= ~LONGLONGBIT(HALL_SENSOR_BIT); //after detecting it, discard the hall effect bit of the array -- it is not an actual key, so don't report it as one.

/*READ AND DEBOUNCE REED SWITCH INPUTS*/

		/*Detect which reeds have been pressed, and if they have been sent to the host already or if they still need to be*/
		/*Note: In this code, the term "Active" means that a key is being held down.  If a key is already active, it will not be sent if detected)*/
		for (uint8_t i=0; i<4; i++){
			int reednumber = i+1;
			int j;
			
			if (Reeds_Are_Independent) j = i; //
			else j = 0; //setting j = 0 means "If any reed is active, all reeds are considered active also (and are therefore won't be sent when pressed)
			
			if (SensorReadout & LONGLONGBIT(REED_BITS[i])){ //if the reed is detected,
				if (ReedDebounce[i] < ReedHoldTime){//increment the debounce entry if not already maxed out.
					ReedDebounce[i] ++;
				} 
				else if (!ActiveReeds[j]){ //if the debounce has reached KeyHoldTime, but the reed isn't already listed as active
						ActiveReeds[j] = true; //then list it as active.
						ReedToSend = reednumber; //code 1,2,3, or 4 indicates which reed has been pressed
						OKtoSendReed = true;//and tell the routine to send it
				}
			} 
			else if (Reeds_Are_Independent){//if the reed is not detected, and we are tracking them separately.
				if(ReedDebounce[i] == 0) ActiveReeds[j] = false; //then if the debounce counter has run down, consider the reed to be released.
				else ReedDebounce[i]--; //if not already zero, decrement the counter
			}
			else if (!Reeds_Are_Independent){ //if reeds are not considered independent, all reeds must be released before another can fire.
				if((ReedDebounce[0] == 0)&&(ReedDebounce[1] == 0)&&(ReedDebounce[2]== 0)&&(ReedDebounce[3]==0)){ //so wait for all reeds to be released
					ActiveReeds[j] = false; // only if all reeds are released do we allow a new reed to be pressed.
				}
				else if (ReedDebounce[i]) {ReedDebounce[i]--;} //decrement debounce counter for this reed.
			}
		}
		

		

/*APPLY VARIOUS MASKS TO SIMPLIFY SENSOR READOUT*/	
		SensorReadout = SensorReadout & KEY_SENSOR_MASK;//discard the reed switch bits and the sensor bit -- look only at the key sensor contacts.
		if (SensorReadout == KEY_SENSOR_MASK){
			SensorReadout = 0;// if masked sensor readout is all ones, sensor is probably not plugged in -- discard.
		}
		else if(ActiveKey){
			SensorReadout = (SensorReadout & KEYMASK(ActiveKey)); //if a key was detected last time, mask all others -- only look at that key this time -- prevents confusion from multiple keys.
		}
		else if(PreviousKey){ //if no key was detected, but a key was recently detected (maybe it was just released, or maybe it bounced off)
			DoubleTapCounter++;
			if (DoubleTapCounter >= DoubleTapTime){ //once the double tap timer has expired, reset everything and stop ignoring previous key.
				DoubleTapCounter = 0;
				PreviousKey = 0; 
			}		
			else{ //if timer has not expired yet, ignore the previous key pressed.
				SensorReadout = (SensorReadout & ~KEYMASK(PreviousKey)); //whatever the previous active key pressed was, ignore it.
			}
		}
		
/*DETERMINE WHICH CONTACT, IF ANY, HAS DETECTED A KEY THIS ROUND*/	
		if(SensorReadout){ //if sensor readout is not all zeros
			DetectedKey = (uint8_t) __builtin_clzll(SensorReadout); //get the position of the first "one" in the sparse key detection array 
		}
		else{
			DetectedKey = 0;
		}
		
/*DEBOUNCE KEY READING*/
		if(DetectedKey){//if there is a detected key this time,
			 KeyHoldCounter++;  
			 KeyReleaseCounter=0;		 
		}
		else {
			KeyReleaseCounter++; 
			KeyHoldCounter = 0;  
		}
		
		if (KeyHoldCounter >= KeyHoldTime){
			KeyHoldCounter = KeyHoldTime; // can't get higher than keyholdtime
			if (DetectedKey != ActiveKey){ // if this is a new active key (just pressed) then set the active key to the new value, and give go-ahead to send it.
				ActiveKey = DetectedKey; //the current key is the new active key
				OKtoSendKey = true;
			}
		}
		else if (KeyReleaseCounter >= KeyReleaseTime){
			KeyReleaseCounter = KeyReleaseTime; //
			PreviousKey = ActiveKey; //save the current active key as the previous key pressed.
			ActiveKey = 0; //return 0 for the active key -- meaning, no keys are pressed, or the current active key was just released.
		}
		
/*SEND RESULTS, IF ANY, BACK TO MAIN ROUTINE*/
		if(OKtoSendReed){//if there is a reed switch that needs sending, report it
			return ReedToSend;
		}
		else if(OKtoSendKey){ //otherwise, if there is a key to send, report it
			return ActiveKey; 
		}
		else{
			return 0; //if no keys or reeds need sending, return 0
		}

}

unsigned long long ReadSensor(){
		unsigned long long Readout = 0;	
		bool HallReading;
		
		GlobalInterruptDisable();
		set_high(SENSE_CLK);
		Delay_MS(1); //Sensor board has an LP filter and an inverter on the _LOAD signal.  Wait X ms for the low-pass filter on the _LOAD signal to fire low. 
		set_low(SENSE_CLK);
		GlobalInterruptEnable();
		Delay_MS(1); //Discharge the LP filter, sending _LOAD high.  This also takes X ms
		
		for (int i=0;i<SHIFT_REGISTER_PINS;i++){   //loop through every bit in readout. i=0 is the first contact (actually the 8th one on the board)
			if (is_low(SENSE_SER)) { 
				longlongbit_set(Readout,i);// if the readout for one of the sensor pins comes back low, that key has been pressed -- store it as a 1 in the readout.
			}	
			
			GlobalInterruptDisable();//if sense clk stays high too long, it could falsely trigger _Load signal.
			set_high(SENSE_CLK);
			_delay_us(CLK_POS_PULSE); //delay for the required pulsewidth
			set_low(SENSE_CLK);
			GlobalInterruptEnable();
			_delay_us(CLK_NEG_PULSE); //delay for the required negative pulsewidth

		}
		
		/*The hall effect sensor on the end of the sensor board is only installed in certain cases -
		- its job is to tell if the entire crossbar has moved(active), or if it is at rest (therefore no keys should be detected)*/
		if(UseHallSensor){ //when the hall effect sensor is installed and activated
			HallReading = Readout & LONGLONGBIT(HALL_SENSOR_BIT); //one of the bits of the sensor readout gives the state of the hall sensor
			if(HallReading != HallSensorPolarity){
				Readout = 0; //then if the hall effect sensor is not triggered, readout of keys is invalid -- clear it. 
			}
		}

		//add the reed switches to the last 4 bits of the readout array -- if the reed switch is for the shift key, ignore it!
		if (Shift_Reed != 1 && is_high(REED_1)==Reed1Polarity){ Readout |= LONGLONGBIT(REED_BITS[0]);} //63rd bit of readout (or something like that) represents Reed1
		if (Shift_Reed != 2 && is_high(REED_2)==Reed2Polarity){ Readout |= LONGLONGBIT(REED_BITS[1]);} //62nd bit of readout (or something like that) represents Reed2
		if (Shift_Reed != 3 && is_high(REED_3)==Reed3Polarity){ Readout |= LONGLONGBIT(REED_BITS[2]);} //61st bit of readout (or something like that) represents Reed3
		if (Shift_Reed != 4 && is_high(REED_4)==Reed4Polarity){ Readout |= LONGLONGBIT(REED_BITS[3]);} //60th bit of readout (or something like that) represents Reed4
		
		return Readout;
}

uint8_t GetHIDKeyCode(uint8_t key, uint8_t modifier){ 
	uint8_t code;

	if ((modifier & HID_KEYBOARD_MODIFIER_LEFTALT) && FnKeyCodeLookUpTable[key]){ //if the FN key is held down, look up key in FN array.
		code = FnKeyCodeLookUpTable[key];
	}
	else if ((modifier & HID_KEYBOARD_MODIFIER_LEFTSHIFT) && ShiftKeyCodeLookUpTable[key]){
		code = ShiftKeyCodeLookUpTable[key];
	}
	else {
		code = KeyCodeLookUpTable[key]; //otherwise, look up the key in the regular array.
	}
	return code;
}

uint8_t GetASCIIKeyCode(uint8_t key, uint8_t modifier){
	uint8_t code;
	
	if ((modifier & HID_KEYBOARD_MODIFIER_LEFTSHIFT) && ASCIIShiftLookUpTable[key]){
		code = ASCIIShiftLookUpTable[key];
	}
	else {
		code = ASCIILookUpTable[key];
	}
	return code;
}

bool getHallState(){ //don't call this function from inside ReadSensor !  It will cause an infinite loop...
	bool hallstate;
	hallstate = (bool)(ReadSensor() & LONGLONGBIT(HALL_SENSOR_BIT));
	return hallstate;
}

	

void LoadKeyCodeTables(){
	 eeprom_read_block (( void *) KeyCodeLookUpTable, (void *) KEYCODE_ADDR, KEYCODE_ARRAY_LENGTH);
	 eeprom_read_block (( void *) FnKeyCodeLookUpTable, (void *) FN_KEYCODE_ADDR, KEYCODE_ARRAY_LENGTH);
	 eeprom_read_block (( void *) ShiftKeyCodeLookUpTable, (void *) SHIFT_KEYCODE_ADDR, KEYCODE_ARRAY_LENGTH);
	 eeprom_read_block (( void *) ASCIILookUpTable, (void *) ASCII_ADDR, KEYCODE_ARRAY_LENGTH);
	 eeprom_read_block (( void *) ASCIIShiftLookUpTable, (void *) ASCII_SHIFT_ADDR, KEYCODE_ARRAY_LENGTH);

}

void LoadEepromParameters(){	
	 KeyReleaseTime = eeprom_read_byte((uint8_t *)RELEASE_TIME_ADDR);
	 KeyHoldTime = eeprom_read_byte((uint8_t *)HOLD_TIME_ADDR);
	 DoubleTapTime = eeprom_read_byte((uint8_t *)DOUBLE_TAP_ADDR);
	 ReedHoldTime = eeprom_read_byte((uint8_t *)REED_HOLD_TIME_ADDR);
	 UseHallSensor = eeprom_read_byte((uint8_t *)USE_HALL_SENSOR_ADDR);
	 HallSensorPolarity = eeprom_read_byte((uint8_t *)HALL_SENSOR_POLARITY_ADDR);
	 Shift_Reed = eeprom_read_byte((uint8_t *)SHIFT_REED_ADDR);
	 Reeds_Are_Independent = eeprom_read_byte((uint8_t *)REEDS_INDEPENDENT_ADDR);
}

void ClearKeyCodeTables(){
	memset (&KeyCodeLookUpTable[0] , 0, KEYCODE_ARRAY_LENGTH);
	memset (&FnKeyCodeLookUpTable[0] , 0, KEYCODE_ARRAY_LENGTH);
	memset (&ShiftKeyCodeLookUpTable[0], 0, KEYCODE_ARRAY_LENGTH);
	memset (&ASCIILookUpTable[0],0,KEYCODE_ARRAY_LENGTH);
	memset (&ASCIIShiftLookUpTable[0],0,KEYCODE_ARRAY_LENGTH);

	Shift_Reed = 0;
	UseHallSensor = 0;	
}

void InitializeEeprom(){
	int i;
	if (eeprom_read_byte((uint8_t*)EEP_CHECKSUM_ADDR) != EEP_CHECKSUM){
		
		for(i=EEP_BANK0; i<=EEP_BANK0_END; i++){
			  eeprom_update_byte((uint8_t*)i,0); // clear contents of eeprom to 0
		}
		
		for(i=EEP_BANK2; i<=EEP_BANK2_END; i++){
			  eeprom_update_byte((uint8_t*)i,0); // clear contents of eeprom to 0
		}
		
		RestoreFactoryDefaults();

		eeprom_write_byte((uint8_t*)EEP_CHECKSUM_ADDR, EEP_CHECKSUM);//write the checksum to the eeprom to indicate that eeprom has been properly initialized.
	}
}

void RestoreFactoryDefaults(){
			eeprom_update_byte((uint8_t*)DOUBLE_TAP_ADDR, DEFAULT_DOUBLE_TAP_TIME);
			eeprom_update_byte((uint8_t*)HOLD_TIME_ADDR, DEFAULT_HOLD_TIME);
			eeprom_update_byte((uint8_t*)RELEASE_TIME_ADDR, DEFAULT_RELEASE_TIME);
			eeprom_update_byte((uint8_t*)REED_HOLD_TIME_ADDR, DEFAULT_REED_HOLD_TIME);
			eeprom_update_byte((uint8_t*)REEDS_INDEPENDENT_ADDR,REEDS_ARE_INDEPENDENT_BY_DEFAULT);
			eeprom_update_byte((uint8_t*)DEFAULT_MODE_ADDR,USB_COMBO_MODE);
			eeprom_write_word((uint16_t *)FILENUM_ADDR,0);//reset sd card file number to zero.
			
			LoadEepromParameters(); //load new defaults into RAM
}



