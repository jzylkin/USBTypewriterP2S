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
 *  Header file for Keyboard.c.
 */

#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

	/* Includes: */
		#include <avr/io.h>
		#include <avr/wdt.h>
		#include <avr/power.h>
		#include <avr/interrupt.h>
		#include <stdbool.h>
		#include <string.h>
		#include <util/delay.h>

		#include "Descriptors.h"

		#include <LUFA/Drivers/Board/Joystick.h>
		#include <LUFA/Drivers/Board/LEDs.h>
		#include <LUFA/Drivers/Board/Buttons.h>
		#include <LUFA/Drivers/USB/USB.h>
		#include <LUFA/Platform/Platform.h>
		
		
		#include "Lib/SCSI.h"

		#include "Lib/SDCardManager.h"
		
				#include "Lib/FATFs/ff.h"
				#include "Config/AppConfig.h"
				

		
		/***Jack's additional includes****/
		#include "IO_Macros.h"
		#include "Config_IO.h"
		#include "Config_Interrupts.h"
		#include "Init_Mode.h"
		#include "Sense_Keys.h"
		#include "Send.h"
		#include "Bluetooth.h"
		#include "uart.h"
		
		/**Jack'ss Macros */
		#define DELAY_1US asm("nop;nop;nop;nop;nop;nop;nop;nop;")
		#define DELAY_5US DELAY_1US;DELAY_1US;DELAY_1US;DELAY_1US;DELAY_1US
		#define DELAY_10US DELAY_5US;DELAY_5US
		
		#define SHIFT_REGISTER_PINS 8*6  //There are 6 shift registers, each with 8 inputs.
		
		#define KEYCODE_LENGTH 64
		#define FN_KEYCODE_LENGTH 64
		#define SHIFT_KEYCODE_LENGTH 64
		#define NUM_REED_SWITCHES 4
		
		#define KEYCODE_ADDR 0 //starting address of the table that stores the keycodes for each of 64 contacts
		#define FN_KEYCODE_ADDR (KEYCODE_ADDR + KEYCODE_LENGTH) //table starting  address for all contacts when fn key is held down
		#define SHIFT_KEYCODE_ADDR (FN_KEYCODE_ADDR + FN_KEYCODE_LENGTH) //table for all contacts when shift is held down (if implemented)
		#define REED_SWITCH_ADDR (SHIFT_KEYCODE_ADDR + SHIFT_KEYCODE_LENGTH)  //table for all reed switch functions.
		
		/** Indicates if the disk is write protected or not. */
		#define DISK_READ_ONLY           false
		
		#define ASCII_A 65
		#define HID_A 4
	/* Function Prototypes: */
		void SetupHardware(void);

		void EVENT_USB_Device_Connect(void);
		void EVENT_USB_Device_Disconnect(void);
		void EVENT_USB_Device_ConfigurationChanged(void);
		void EVENT_USB_Device_ControlRequest(void);
		void EVENT_USB_Device_StartOfFrame(void);

		bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
		                                         uint8_t* const ReportID,
		                                         const uint8_t ReportType,
		                                         void* ReportData,
		                                         uint16_t* const ReportSize);
		void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
		                                          const uint8_t ReportID,
		                                          const uint8_t ReportType,
		                                          const void* ReportData,
		                                          const uint16_t ReportSize);
												  

		bool CALLBACK_MS_Device_SCSICommandReceived(USB_ClassInfo_MS_Device_t* const MSInterfaceInfo);
												 
		uint32_t get_num_of_sectors();
		FRESULT OpenLogFile(void);
		void CloseLogFile(void);

#endif




