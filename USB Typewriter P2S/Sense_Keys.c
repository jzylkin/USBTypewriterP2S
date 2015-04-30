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

uint8_t GetModifier(){
	uint8_t Modifier = 0;
	if(is_low(REED_4)) {bit_set(Modifier,HID_KEYBOARD_MODIFIER_LEFTSHIFT);}
	if(is_low(S1)) {bit_set(Modifier,HID_KEYBOARD_MODIFIER_LEFTCTRL);}
	if(is_low(S2)) {bit_set(Modifier,HID_KEYBOARD_MODIFIER_LEFTALT);}
	if(is_low(S2)) {bit_set(Modifier,HID_KEYBOARD_MODIFIER_LEFTGUI);}
	
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
	#define READOUT_FILTER_DEPTH 2 // only 2 entries in filter array of long longs
	#define RELEASED_KEY_TIME 5 //delay 5 cycles before recognizing that a key has been released.
	#define NO_BINARY_FILTERING
	unsigned long long ThisSensorReadout; //create 64-bit "long long" binary variable and set it all to 0s.
	unsigned long long SensorReadoutAvg;
	static uint8_t ReleaseCount; // timer that ticks down when the active key is released.
	static uint8_t ActiveKey; //the key that has most recently been pressed.
	uint8_t Key;//the return value
	
	ThisSensorReadout = ReadSensor();
	Key=0;//by default, there is no key to send, unless one is detected later.

	#ifdef USE_BINARY_FILTERING
	static unsigned long long *SensorReadouts[READOUT_FILTER_DEPTH];//initialize an array of readouts 
	SensorReadoutAvg= ThisSensorReadout & *SensorReadouts[0] & *SensorReadouts[1];// this should be a for loop -- bitwise ANDing the last few readouts will debounce the sensor;
	memmove(SensorReadouts+1, SensorReadouts, (READOUT_FILTER_DEPTH-1)*sizeof(unsigned long long*)); //shift the filter values over 1
	*SensorReadouts[0] = ThisSensorReadout; //push new sensor readout onto the filter in the spot that just opened up.
	#else 
	SensorReadoutAvg= ThisSensorReadout;
	#endif

	bool ACTIVE_KEY_PERSISTS = bit_get(SensorReadoutAvg,ActiveKey);

	if(! ACTIVE_KEY_PERSISTS){ //if current active key is not in the list, 
		if(ReleaseCount){ReleaseCount--;}//then decrement the releasecount (if not already zero);
	}
	else{//if the active key is in the list
		ReleaseCount = RELEASED_KEY_TIME; //then reset the release count, reaffirming the keypress
	}
		
	if (ReleaseCount== 0){//if the current active key has been released and the release debounce counter has expired.
		if (SensorReadoutAvg){//and if there are any other keys that have been pressed
			ActiveKey = __builtin_clzll(SensorReadoutAvg);//count the leading zeros from lsb (finds the first 1 in the binary number). This is the new active key
			ReleaseCount = RELEASED_KEY_TIME; //reset the key release timer
			Key = ActiveKey; //return the new active key. ONLY return a key when it is first pressed.  Otherwise, return zero.
		}
		else{
			ActiveKey = 0; //otherwise, there is no active key
			ReleaseCount = 0; //clear everything
			Key = 0; //No Key to send;
		}
	}
	
	return Key;//return a key number if it has just been pressed, otherwise return zero;
	
}

unsigned long long ReadSensor(){
		unsigned long long Readout = 0;	
	
		set_low(SENSE_CLK);
		
//		set_high(SENSE_CLR);//To ramp up the power on the external board slowly, we use sense_clr
//		configure_as_output(SENSE_CLR);
//		_delay_us(30); //wait for the sensor board to charge up.

		configure_as_input(SENSE_CLR);//stop grounding-out the sensor's power supply -- this output works as an open collector!!! NOT PUSH PULL!
		set_low(SENSE_POWER); //begin supplying power to sensor;
		
		_delay_us(200);//wait some number of uS for the sensor board to initialize (_PL signal on sensor board goes high about 100uS after powering up)
		
		for (int i=0;i<SHIFT_REGISTER_PINS;i++){   //loop through every bit in readout. i=0 is the first contact (actually the 8th one on the board)
			if (is_low(SENSE_SER)) { 
				longlongbit_set(Readout,i);// if the readout for one of the sensor pins comes back low, that key has been pressed -- store it as a 1 in the readout.
			}	
			set_high(SENSE_CLK);
			_delay_us(1);
			set_low(SENSE_CLK);
			_delay_us(1);
		}
		
		set_high(SENSE_POWER);//turn off sensor power
		_delay_us(50); //wait for power switch to fully turn off
		
		set_low(SENSE_CLR);// Sensor's VCC must go back to 0V, because the shift registers' _PL pins are tied to VCC, _PL must be low on each power up.
		configure_as_output(SENSE_CLR);
		Delay_MS(10);  //a delay of some length (not long) is ABSOLUTELY required after each scan.  Otherwise, the sensor does not have time to reset itself.

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
	 
	 for (int i = 0; i<sizeof(KeyCodeLookUpTable); i++){ //change 0xff (default uninitialized eeprom value) to zero
		 if (KeyCodeLookUpTable[i]== 0xff){KeyCodeLookUpTable[i] = 0;}
	 }
	 for (int i = 0; i<sizeof(FnKeyCodeLookUpTable); i++){
		 if (FnKeyCodeLookUpTable[i]== 0xff){FnKeyCodeLookUpTable[i] = 0;}
	 }
	 for (int i = 0; i<sizeof(ShiftKeyCodeLookUpTable); i++){
		 if (ShiftKeyCodeLookUpTable[i]== 0xff){ShiftKeyCodeLookUpTable[i] = 0;}
	 }
	 for (int i = 0; i<sizeof(ReedSwitchLookUpTable); i++){
		 if (ReedSwitchLookUpTable[i]== 0xff){ReedSwitchLookUpTable[i] = 0;}
	 }
}

void ClearKeyCodeTables(){
	memset (&KeyCodeLookUpTable[0] , 0, KEYCODE_LENGTH);
	memset (&FnKeyCodeLookUpTable[0] , 0, FN_KEYCODE_LENGTH);
	memset (&ShiftKeyCodeLookUpTable[0], 0, SHIFT_KEYCODE_LENGTH);
	memset (&ReedSwitchLookUpTable[0], 0, NUM_REED_SWITCHES);
	
	for (int i = 0; i<sizeof(KeyCodeLookUpTable); i++){ //change 0xff (default uninitialized eeprom value) to zero
		if (KeyCodeLookUpTable[i]== 0xff){KeyCodeLookUpTable[i] = 0;}
	}
	for (int i = 0; i<sizeof(FnKeyCodeLookUpTable); i++){
		if (FnKeyCodeLookUpTable[i]== 0xff){FnKeyCodeLookUpTable[i] = 0;}
	}
	for (int i = 0; i<sizeof(ShiftKeyCodeLookUpTable); i++){
		if (ShiftKeyCodeLookUpTable[i]== 0xff){ShiftKeyCodeLookUpTable[i] = 0;}
	}
	for (int i = 0; i<sizeof(ReedSwitchLookUpTable); i++){
		if (ReedSwitchLookUpTable[i]== 0xff){ReedSwitchLookUpTable[i] = 0;}
	}
}


