/*
 * globals.h
 *
 * Created: 12/27/2014 2:50:17 PM
 *  Author: Jack
 */ 


#ifndef GLOBALS_H_
#define GLOBALS_H_

volatile extern int TMR1_Count;
volatile extern USB_KeyboardReport_Data_t* KeyBuffer;
extern uint8_t KeyCodeLookUpTable[KEYCODE_ARRAY_LENGTH];
extern uint8_t FnKeyCodeLookUpTable[KEYCODE_ARRAY_LENGTH];
extern uint8_t ShiftKeyCodeLookUpTable[KEYCODE_ARRAY_LENGTH];
extern uint8_t ReedSwitchLookUpTable[NUM_REED_SWITCHES];
extern uint8_t ASCIILookUpTable[KEYCODE_ARRAY_LENGTH];
extern uint8_t ASCIIShiftLookUpTable[KEYCODE_ARRAY_LENGTH];
extern uint8_t KeyBufferMod;

extern uint8_t DoubleTapTime;
extern uint8_t KeyReleaseTime;
extern uint8_t KeyHoldTime;





#endif /* GLOBALS_H_ */