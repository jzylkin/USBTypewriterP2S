/*
             USB Typewriter
     Copyright (C) Jack Zylkin, 2016.

		www.usbtypewriter.com
*/

/*
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

		#define		EHONG 1
		#define		RN42 2

		//document the firmware version and bluetooth module version on the following two lines: choices for module_name are "RN42" or "EHONG"
		
		#define		FIRMWARE_VERSION	"FIRMWARE VER 5.5.0" //bugfix of iphone
		#define		MODULE_NAME			EHONG //the module name can be either EHONG or RN42

		
//		#define		BT_DEBUG  //define BT_DEBUG to relay bluetooth module UART conversations over usb.
	
		#if				MODULE_NAME==RN42
		#define			MODULE_STR ".RN"
		#elif			MODULE_NAME==EHONG
		#define			MODULE_STR ".EH"
		#else 
		#error "INVALID MODULE NAME!"
		#endif
		
		#define		FIRMWARE_VERSION_AND_MODULE		FIRMWARE_VERSION MODULE_STR 

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
		#include "Sense_Keys.h"
		#include "Send.h"
		#include "Bluetooth.h"
		#include "uart.h"
		#include "KeyLogging.h"
		
		/**Jack'ss Macros */
		#define DELAY_1US asm("nop;nop;nop;nop;nop;nop;nop;nop;")
		#define DELAY_5US DELAY_1US;DELAY_1US;DELAY_1US;DELAY_1US;DELAY_1US
		#define DELAY_10US DELAY_5US;DELAY_5US
		
		#define SHIFT_REGISTER_PINS 8*6  //There are 6 shift registers, each with 8 inputs.
		
		#define KEYCODE_ARRAY_LENGTH 64 // must be less than or equal to 64 (0x40)
		
		#define NUM_REED_SWITCHES 4
		
		
		#define EEP_BANK0 0 //bank 0 starts at 0
		#define KEYCODE_ADDR 0 //starting address of the table that stores the keycodes for each of 64 contacts
		#define FN_KEYCODE_ADDR 0x40 //table starting  address for all contacts when fn key is held down
		#define SHIFT_KEYCODE_ADDR 0x80 //table for all contacts when shift is held down (if implemented)
		#define ASCII_ADDR 0xC0 //table to store ascii characters for saving to SD card
		#define ASCII_SHIFT_ADDR 0x100 // table for shifted ascii characters for saving to SD card
		#define EEP_BANK0_END 0x150 // eeprom BANK0 ends at this address
		
		#define EEP_BANK2 0x200
		#define DOUBLE_TAP_ADDR 0x200
		#define HOLD_TIME_ADDR 0x201
		#define RELEASE_TIME_ADDR 0x202
		#define USE_HALL_SENSOR_ADDR 0x203
		#define HALL_SENSOR_POLARITY_ADDR 0x204
		#define SHIFT_REED_ADDR 0x205
		#define REED_1_POLARITY_ADDR 0x206
		#define REED_2_POLARITY_ADDR 0x207
		#define REED_3_POLARITY_ADDR 0x208
		#define REED_4_POLARITY_ADDR 0x209
		#define FILENUM_ADDR 0x20A
		#define FILENUM_ADDR_2 0x20B
		#define REED_HOLD_TIME_ADDR 0x20C
		#define DEFAULT_MODE_ADDR 0x20D
		#define REEDS_INDEPENDENT_ADDR 0x20E
		#define DUMMY_LOAD_ADDR 0x20F
		#define ENABLE_PIN_CODE_ADDR 0x210
		#define EEP_BANK2_END 0x211
		
		#define EEP_CHECKSUM_ADDR 0x3FF
		#define EEP_CHECKSUM 71 //if eeprom doesn't have this random code in the checksum address, it has not been initialized yet. 
		

		#define SENSE_DELAY 1// wait X ms between reading the sensor.
		#define DEFAULT_HOLD_TIME 6
		#define DEFAULT_RELEASE_TIME 3
		#define DEFAULT_DOUBLE_TAP_TIME 5
		#define DEFAULT_REED_HOLD_TIME 3
		#define REEDS_ARE_INDEPENDENT_BY_DEFAULT 0
		
		//TIMING FOR SENSOR READOUT ROUTINES
		#define CLK_POS_PULSE 10
		#define CLK_NEG_PULSE 40

		//DEFINING THE 64-bit SENSOR BITFIELD
		#define KEY_SENSOR_MASK 0x0000FFFFFFFFFFFF  //This mask represents the lowest 48 bits of the bitfield  -- these are the bits used for detecting keys on the sensor.
		#define REED_1_BIT 63
		#define REED_2_BIT 62
		#define REED_3_BIT 61
		#define REED_4_BIT 60
		#define HALL_SENSOR_BIT 44
		
		#define HALL_ACTIVE 2
		#define HALL_NOT_ACTIVE 1
		#define HALL_NOT_PRESENT 0
		
		#define NO_MODE 0
		#define USB_COMBO_MODE 1
		#define SD_MODE 2
		#define SENSITIVITY_MODE 3
		#define TEST_MODE 4
		#define CAL_MODE 5
		#define QUICK_CAL_MODE 6
		#define FINETUNING_MODE 7
		#define DEEP_SLEEP_MODE 8
		#define BLUETOOTH_MODE 9
		#define PANIC_MODE 10
		#define HARDWARE_TEST 11
		#define INITIALIZING 12
		#define USB_LIGHT_MODE 13
		#define MANUAL_CAL_MODE 14
		
		#define SD_BUFFER_LENGTH 512
		
		#define USB_SEND_TIMEOUT 100 //wait 50ms if usb send buffer is full before discarding a character.
		#define USB_SEND_DELAY 20// wait 30ms after sending each key to usb.
		#define CALIBRATION_DELAY 500// wait 500ms between programming keys.
		
		
		#define INIT_DELAY 500// wait X ms after initializing.
		
		#define ENUMERATION_TIMEOUT 5000 //how long to wait for enumeration to happen.  this parameter might not be used.
		
		#define SD_TIMEOUT_S  (uint16_t)1800 //how many seconds to wait without any input before closing the file (half an hour?)
		#define SD_TIMEOUT (uint16_t)SD_TIMEOUT_S*100
		
		#define SD_SAVE_TIME_S (uint16_t)30 //how many seconds before saving work if no keys are pressed.
		#define SD_SAVE_TIME (uint16_t)SD_SAVE_TIME_S*100
		
		#define BT_SLEEP_TIMEOUT_S  (uint16_t)6000 //how many seconds to wait without any input before sleeping bluetooth (10 min)
		#define BT_SLEEP_TIMEOUT (uint16_t)BT_SLEEP_TIMEOUT_S*100
		
		#define STRING_SEND_DELAY 50
		
		/** Indicates if the disk is write protected or not. */
		#define DISK_READ_ONLY           false
		
		#define ASCII_A 0x41 // upper case A in ascii
		#define ASCII_a 0x61 //lower case A in ascii
		
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
												 

		
	void Init_Mode(void);
	void Task_Manager(void);
		#define NOTASK 0
		#define MSHIDTASK 1
		#define MSTASK 2
		#define HIDTASK 3

	#define FORCE_UPPER 0x80 //a flag that forces a keycode to be upper case.

bool WaitForEnumeration();

		

#endif




