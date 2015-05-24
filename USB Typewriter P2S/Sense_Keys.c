/*
 * Sense_Keys.c
 *
 * Created: 12/10/2014 10:30:42 AM
 *  Author: Jack
 */ 
#include "Keyboard.h"
#include "Sense_Keys.h"
#include "KeyCodes.h"

//extern USB_KeyboardReport_Data_t* KeyBuffer;

extern uint8_t KeyCodeLookUpTable[KEYCODE_LENGTH];
extern uint8_t FnKeyCodeLookUpTable[FN_KEYCODE_LENGTH];
extern uint8_t ShiftKeyCodeLookUpTable[SHIFT_KEYCODE_LENGTH];
extern uint8_t ReedSwitchLookUpTable[NUM_REED_SWITCHES];

//TIMING FOR SENSOR READOUT ROUTINES
#define CLK_DUTY 20 //% duty cycle of sensor clock positive pulse (in percent)
#define CLK_PER 40 //period of sensor clock in us
#define CLK_POS_PULSE (int)(CLK_PER*100)/CLK_DUTY // positive pulsewidth of sensor clk
#define CLK_NEG_PULSE CLK_PER-CLK_POS_PULSE //negative pulsewidth of sensor clk


uint8_t GetModifier(){
	uint8_t Modifier = 0;
	if(is_low(REED_4)) {bit_set(Modifier,HID_KEYBOARD_MODIFIER_LEFTSHIFT);}
	if(is_low(CTRL_KEY)) {bit_set(Modifier,HID_KEYBOARD_MODIFIER_LEFTCTRL);}
	if(is_low(ALT_KEY)) {bit_set(Modifier,HID_KEYBOARD_MODIFIER_LEFTALT);}
	if(is_low(CMD_KEY)) {bit_set(Modifier,HID_KEYBOARD_MODIFIER_LEFTGUI);}
	
	return Modifier;
}

uint8_t GetKeySimple(){
	unsigned long long ThisSensorReadout; //create 64-bit "long long" binary variable and set it all to 0s.
	uint8_t Key;
	
	Key=0;//by default, there is no key to send, unless one is detected later.
	ThisSensorReadout = ReadSensor();
	if(ThisSensorReadout){
		Key = (uint8_t) __builtin_clzll(ThisSensorReadout);
	}
	else{
		Key = 0;
	}
	return  Key;
	
}
	
	
uint8_t GetKey(){
		unsigned long long ThisSensorReadout; //create 64-bit "long long" binary variable and set it all to 0s.
		uint8_t DetectedKey; //the key we detect this round
		static uint8_t PreviousKey;
		static uint8_t ActiveKey; //the last key that was confirmed and detected and reported as a successful keypress.
		static uint8_t DoubleTapCounter;
		static uint8_t KeyHoldCounter;
		static uint8_t KeyReleaseCounter;
		
		Key=0;//by default, there is no key detected.
		
		ThisSensorReadout = ReadSensor();
		
		if(Active_Key){
			ThisSensorReadout = (ThisSensorReadout & LONGLONGBIT(Active_Key)); //if a key was detected last time, mask all others -- only look at that key this time -- prevents confusion from multiple keys.
		}
		
		else if(Previous_Key){
			DoubleTapCounter++;
			if (DoubleTapCounter >= DoubleTapTime){
				DoubleTapCounter = 0;
				PreviousKey = 0;
			}		
			else{
				ThisSensorReadout = (ThisSensorReadout & ~LONGLONGBIT(Previous_Key)); //whatever the previous active key pressed was, ignore it.
			}
		}
		
		Detected_Key = (uint8_t) __builtin_clzll(ThisSensorReadout); //get the position of the first "one" in the sparse key detection array
			
		if(Detected_Key){//if there is a detected key this time,
			 KeyHoldCounter++;  
			 KeyReleaseCounter=0;		 
		}
		else {
			KeyReleaseCounter++; 
			KeyHoldCounter = 0;  
		}
		
		if (KeyHoldCounter >= KeyHoldTime){
			KeyHoldCounter = KeyHoldTime; // can't get higher than keyholdtime
			ActiveKey = Key; //the current key is the new active key
		}
		else if (KeyReleaseCounter >= KeyReleaseTime){
			KeyReleaseCounter = KeyReleaseTime; //
			PreviousKey = ActiveKey; //save the current active key as the previous key pressed.
			ActiveKey = 0; //return 0 for the active key -- meaning, no keys are pressed, or the current active key was just released.
		}

	
		

}

unsigned long long ReadSensor(){
		unsigned long long Readout = 0;	
	
		set_high(SENSE_CLK);
		Delay_MS(1); //Sensor board has an LP filter and an inverter on the _LOAD signal.  Wait X ms for the low-pass filter on the _LOAD signal to fire low. 
		set_low(SENSE_CLK);
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
		
		return Readout;
}

uint8_t GetKeyCode(uint8_t key, uint8_t modifier){
	uint8_t code;
	if ((modifier & HID_KEYBOARD_MODIFIER_LEFTALT) && FnKeyCodeLookUpTable[key]){
			code = FnKeyCodeLookUpTable[key];
	}
	else if ((modifier & HID_KEYBOARD_MODIFIER_LEFTSHIFT) && ShiftKeyCodeLookUpTable[key]){
		code= ShiftKeyCodeLookUpTable[key];
	}
	else {
		code = KeyCodeLookUpTable[key];
	}
	return code;
}
	

void LoadKeyCodeTables(){
	 eeprom_read_block (( void *) KeyCodeLookUpTable, (void *) KEYCODE_ADDR, KEYCODE_LENGTH);
	 eeprom_read_block (( void *) FnKeyCodeLookUpTable, (void *) FN_KEYCODE_ADDR, FN_KEYCODE_LENGTH);
	 eeprom_read_block (( void *) ShiftKeyCodeLookUpTable, (void *) SHIFT_KEYCODE_ADDR, SHIFT_KEYCODE_LENGTH);
	 eeprom_read_block (( void *) ReedSwitchLookUpTable, (void *) REED_SWITCH_ADDR, NUM_REED_SWITCHES);
	 }

void LoadEepromParameters(){
	 KeyReleaseTime = eeprom_read_byte(RELEASE_TIME_ADDR);
	 KeyHoldTime = eeprom_read_byte(HOLD_TIME_ADDR);
	 DoubleTapTime = eeprom_read_byte(DOUBLE_TAP_ADDR);
	 
}

void ClearKeyCodeTables(){
	memset (&KeyCodeLookUpTable[0] , 0, KEYCODE_ARRAY_LENGTH);
	memset (&FnKeyCodeLookUpTable[0] , 0, KEYCODE_ARRAY_LENGTH);
	memset (&ShiftKeyCodeLookUpTable[0], 0, KEYCODE_ARRAY_LENGTH);
	memset (&AsciiLookupTable[0],0,KEYCODE_ARRAY_LENGTH)
	memset (&ReedSwitchLookUpTable[0], 0, NUM_REED_SWITCHES);
	
}

void InitializeEeprom(){
	if (eeprom_read_byte(EEP_CHECKSUM_ADDR) != EEP_CHECKSUM){
		for(int i=EEP_BANK0; i++; i<=EEP_BANK0_END){
			  eeprom_update_byte(i,0); // clear contents of eeprom to 0
		}
		eeprom_update_byte(DOUBLE_TAP_ADDR, DEFAULT_DOUBLE_TAP_TIME);
		eeprom_update_byte(HOLD_TIME_ADDR, DEFAULT_HOLD_TIME);
		eeprom_update_byte(RELEASE_TIME_ADDR, DEFAULT_RELEASE_TIME);		
		eeprom_write_byte(EEP_CHECKSUM_ADDR, EEP_CHECKSUM);//write the checksum to the eeprom to indicate that eeprom has been properly initialized.
	}
	
}


