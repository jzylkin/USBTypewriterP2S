/*
             LUFA Library
     Copyright (C) Dean Camera, 2014.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2014  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/** \file
 *
 *  Main source file for the Keyboard demo. This file contains the main tasks of
 *  the demo and is responsible for the initial application hardware configuration.
 */

#include "Keyboard.h"
#include "Calibrate.h"

volatile int Typewriter_Mode;

volatile USB_KeyboardReport_Data_t* KeyBuffer; //GLOBAL A single-key buffer holding the next key to send. Cleared each time a key is sent.
volatile int TMR1_Count;

uint8_t KeyCodeLookUpTable[KEYCODE_LENGTH];
uint8_t FnKeyCodeLookUpTable[FN_KEYCODE_LENGTH];
uint8_t ShiftKeyCodeLookUpTable[SHIFT_KEYCODE_LENGTH];
uint8_t ReedSwitchLookUpTable[NUM_REED_SWITCHES];

/** Buffer to hold the previously generated Keyboard HID report, for comparison purposes inside the HID class driver. */
static uint8_t PrevKeyboardHIDReportBuffer[sizeof(USB_KeyboardReport_Data_t)];

/** LUFA HID Class driver interface configuration and state information. This structure is
 *  passed to all HID Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_HID_Device_t Keyboard_HID_Interface =
	{
		.Config =
			{
				.InterfaceNumber              = INTERFACE_ID_Keyboard,
				.ReportINEndpoint             =
					{
						.Address              = KEYBOARD_EPADDR,
						.Size                 = KEYBOARD_EPSIZE,
						.Banks                = 1,
					},
				.PrevReportINBuffer           = PrevKeyboardHIDReportBuffer,
				.PrevReportINBufferSize       = sizeof(PrevKeyboardHIDReportBuffer),
			},
	};


/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{	
	uint8_t key;
	uint8_t code;
	uint8_t modifier;
	uint8_t parity;
	Typewriter_Mode = INITIALIZING;
	SetupHardware();
	GlobalInterruptEnable();
	while(1){
		switch (Typewriter_Mode){
			LoadKeyCodeTables();
			while(1){
				if(KeyBuffer->KeyCode[0] == 0){ // If there the key buffer is not full,  get a new key.
					key = GetKey();
					modifier = GetModifier(); 
					code = GetKeyCode(key, modifier);
					if (code) { //if there is a new key to send, then send it
						USBSend(code,modifier);
					}
				}
			}
			case TEST_MODE:
				parity = (uint8_t)is_high(REED_1) + (uint8_t)is_high(REED_2)+ (uint8_t)is_high(REED_3) + (uint8_t)is_high(REED_4);
				if (parity & 1){
					set_low(LED2);
					set_high(LED1);
				}
				else{
					set_low(LED1);
					set_high(LED2);
				}
			case CAL_MODE:
				Calibrate();
			break;
		};
			
		}//infinite loop
}

ISR (TIMER1_COMPA_vect){ //called each time timer1 counts up to the OCR1A register (every couple ms)
	TMR1_Count ++;
	HID_Device_USBTask(&Keyboard_HID_Interface); //These are the VITAL usb functions that must be called every so often (Dean Camera recommends every 30ms, no more that 200ms)
	USB_USBTask(); //these functions respond to host queries, and they load the usb reports by calling the CALLBACK_HID_Device_CreateHIDReport() function
}



/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware()
{

	
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);
	Config_Interrupts();
	
	/* Hardware Initialization */
	Config_IO();
	Init_Mode();
	
	if(Typewriter_Mode == SD_MODE){
		//Initialize SD Card
	}
	else if(Typewriter_Mode == BT_MODE){
		//Initialize Bluetooth Module
	}
	else{
		//Initialize USB HID mode
	USB_Init(); //
	
	}
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{

}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{

}

void EVENT_USB_Device_Suspend(void){
	
}

void EVENT_USB_Device_WakeUp(void){
	
}


/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	ConfigSuccess &= HID_Device_ConfigureEndpoints(&Keyboard_HID_Interface);

	USB_Device_EnableSOFEvents();

}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	HID_Device_ProcessControlRequest(&Keyboard_HID_Interface);
}

/** Event handler for the USB device Start Of Frame event. */
void EVENT_USB_Device_StartOfFrame(void)
{
	HID_Device_MillisecondElapsed(&Keyboard_HID_Interface);
}

/** HID class driver callback function for the creation of HID reports to the host.
 *
 *  \param[in]     HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in,out] ReportID    Report ID requested by the host if non-zero, otherwise callback should set to the generated report ID
 *  \param[in]     ReportType  Type of the report to create, either HID_REPORT_ITEM_In or HID_REPORT_ITEM_Feature
 *  \param[out]    ReportData  Pointer to a buffer where the created report should be stored
 *  \param[out]    ReportSize  Number of bytes written in the report (or zero if no report is to be sent)
 *
 *  \return Boolean \c true to force the sending of the report, \c false to let the library determine if it needs to be sent
 */
bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                         uint8_t* const ReportID,
                                         const uint8_t ReportType,
                                         void* ReportData,
                                         uint16_t* const ReportSize)
{
	USB_KeyboardReport_Data_t* KeyboardReport = (USB_KeyboardReport_Data_t*)ReportData;

	  KeyboardReport->KeyCode[0] = KeyBuffer->KeyCode[0];


	if (KeyboardReport->KeyCode[0]){
		KeyboardReport->Modifier = KeyBuffer->KeyCode[0];
	}
	else {
		KeyboardReport->Modifier = GetModifier();
	}
	
	KeyBuffer->KeyCode[0]= 0;  //remove key to send to clear room for the next key.  This indicates to other routines that the USB buffer is available for sending.
	KeyBuffer->Modifier = 0;

	*ReportSize = sizeof(USB_KeyboardReport_Data_t);
	return false;
}

/** HID class driver callback function for the processing of HID reports from the host.
 *
 *  \param[in] HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in] ReportID    Report ID of the received report from the host
 *  \param[in] ReportType  The type of report that the host has sent, either HID_REPORT_ITEM_Out or HID_REPORT_ITEM_Feature
 *  \param[in] ReportData  Pointer to a buffer where the received report has been stored
 *  \param[in] ReportSize  Size in bytes of the received HID report
 */
void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                          const uint8_t ReportID,
                                          const uint8_t ReportType,
                                          const void* ReportData,
                                          const uint16_t ReportSize)
{
	uint8_t  LEDMask   = LEDS_NO_LEDS;
	uint8_t* LEDReport = (uint8_t*)ReportData;

	if (*LEDReport & HID_KEYBOARD_LED_NUMLOCK)
	  LEDMask |= LEDS_LED1;

	if (*LEDReport & HID_KEYBOARD_LED_CAPSLOCK)
	  LEDMask |= LEDS_LED3;

	if (*LEDReport & HID_KEYBOARD_LED_SCROLLLOCK)
	  LEDMask |= LEDS_LED4;

	LEDs_SetAllLEDs(LEDMask);
}

