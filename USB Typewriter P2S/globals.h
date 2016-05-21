/*
 * globals.h
 *
 * Created: 12/27/2014 2:50:17 PM
 *  Author: Jack
 */ 


#ifndef GLOBALS_H_
#define GLOBALS_H_

volatile extern uint16_t TMR1_Count;
volatile extern USB_KeyboardReport_Data_t* KeyBuffer;
extern uint8_t KeyCodeLookUpTable[KEYCODE_ARRAY_LENGTH];
extern uint8_t FnKeyCodeLookUpTable[KEYCODE_ARRAY_LENGTH];
extern uint8_t ShiftKeyCodeLookUpTable[KEYCODE_ARRAY_LENGTH];
extern uint8_t ASCIILookUpTable[KEYCODE_ARRAY_LENGTH];
extern uint8_t ASCIIShiftLookUpTable[KEYCODE_ARRAY_LENGTH];
volatile extern uint8_t KeyBufferMod;

extern uint8_t DoubleTapTime;
extern uint8_t KeyReleaseTime;
extern uint8_t KeyHoldTime;
extern uint8_t ReedHoldTime;

extern uint8_t Shift_Reed;
extern uint8_t UseHallSensor; 
extern uint8_t HallSensorPolarity;
extern bool Hold_Alt_Down;
//extern uint8_t BluetoothConfigured;

extern uint8_t EnablePinCode;

extern bool Reed1Polarity;
extern bool Reed2Polarity;
extern bool Reed3Polarity;
extern bool Reed4Polarity;

extern volatile uint8_t Typewriter_Mode;
extern volatile uint8_t SD_Buffer[SD_BUFFER_LENGTH];

extern char StringBuffer[60];

extern uint8_t Reeds_Are_Independent;

extern uint8_t Ignore_Flag;

extern uint8_t UseDummyLoad;

volatile extern uint16_t myTimeoutCounter;

#endif /* GLOBALS_H_ */