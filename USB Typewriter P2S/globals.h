/*
 * globals.h
 *
 * Created: 12/27/2014 2:50:17 PM
 *  Author: Jack
 */ 


#ifndef GLOBALS_H_
#define GLOBALS_H_

extern int TMR1_Count;
extern USB_KeyboardReport_Data_t* KeyBuffer;
extern uint8_t KeyCodeLookUpTable[KEYCODE_LENGTH];
extern uint8_t FnKeyCodeLookUpTable[FN_KEYCODE_LENGTH];
extern uint8_t ShiftKeyCodeLookUpTable[SHIFT_KEYCODE_LENGTH];
extern uint8_t ReedSwitchLookUpTable[NUM_REED_SWITCHES];


#endif /* GLOBALS_H_ */