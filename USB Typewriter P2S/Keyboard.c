/*
	  USB TYPEWRITER CODE, 
	  Copyright (C) Jack Zylkin, 2015
	  www.usbtypewriter.com
	  jack [at] usbtypewriter.com


Utilizes the LUFA library written by Dean Camera.
             LUFA Library
     Copyright (C) Dean Camera, 2014.


Permission to use this software is granted under the terms of the Creative Commons Non-Commercial
Attribution Share-Alike license.  You may use this code or its derivatives in your own product, provided that you
do not use it for commercial purposes, and you attribute its origins to Jack Zylkin and USB Typewriter */

/** \file
 *
 *  Main source file for the USB Tyewriter. This file contains the main tasks of
 *  the keyboard and is responsible for the initial application hardware configuration.
 */

#include "Keyboard.h"
#include "Calibrate.h"
#include "KeyCodes.h"



//Globl variables:
volatile int8_t Typewriter_Mode;

volatile USB_KeyboardReport_Data_t* KeyBuffer; // Cleared each time a key is sent.
volatile uint16_t TMR1_Count;

uint8_t KeyCodeLookUpTable[KEYCODE_ARRAY_LENGTH];
uint8_t FnKeyCodeLookUpTable[KEYCODE_ARRAY_LENGTH];
uint8_t ShiftKeyCodeLookUpTable[KEYCODE_ARRAY_LENGTH];
uint8_t ASCIILookUpTable[KEYCODE_ARRAY_LENGTH];
uint8_t ASCIIShiftLookUpTable[KEYCODE_ARRAY_LENGTH];

uint8_t Shift_Reed;
uint8_t UseHallSensor; 
uint8_t HallSensorPolarity;

uint8_t DoubleTapTime;
uint8_t KeyReleaseTime;
uint8_t KeyHoldTime;
uint8_t ReedHoldTime;

uint8_t KeyBufferMod;

bool Reed1Polarity; //reed switches are active low by default
bool Reed2Polarity; //reed switches are active low by default
bool Reed3Polarity; //reed switches are active low by default
bool Reed4Polarity; //reed switches are active low by default

uint8_t Reeds_Are_Independent;

uint8_t Ignore_Flag; //flag to tell processor to ignore the next character typed by user.

uint8_t UseDummyLoad; //dummy load is a 100 ohm resistor (or so) that makes the device draw more current from supply -- useful for power supplies with load minimum requirements.

volatile uint16_t TimeoutCounter;

char StringBuffer[60];//global buffer to store strings that are being forwarded from program memory to regular data memory

volatile uint8_t Scheduled_Task;

/** Buffer to hold the previously generated Keyboard HID report, for comparison purposes inside the HID class driver. */
static uint8_t PrevKeyboardHIDReportBuffer[sizeof(USB_KeyboardReport_Data_t)];

//general purpose buffer to hold information to read/write to sd card
volatile uint8_t SD_Buffer[SD_BUFFER_LENGTH]; //global buffer for general purpose use by multiple functions.

/** LUFA Mass Storage Class driver interface configuration and state information. This structure is
 *  passed to all Mass Storage Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_MS_Device_t Disk_MS_Interface =
	{
		.Config =
			{
				.InterfaceNumber           = INTERFACE_ID_MassStorage,
				.DataINEndpoint            =
					{
						.Address           = MASS_STORAGE_IN_EPADDR,
						.Size              = MASS_STORAGE_IO_EPSIZE,
						.Banks             = 1,
					},
				.DataOUTEndpoint            =
					{
						.Address           = MASS_STORAGE_OUT_EPADDR,
						.Size              = MASS_STORAGE_IO_EPSIZE,
						.Banks             = 1,
					},
				.TotalLUNs                 = 1,
			},
	};


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
	InitializeEeprom();//sets all EEPROM entries to zero if the checksum is incorrect
	LoadEepromParameters();
	LoadKeyCodeTables();
	SDCardManager_Init(); 
	Init_Mode();
	USB_Init();
	GlobalInterruptEnable();
//	Delay_MS(INIT_DELAY);
	
	while(1){
		switch (Typewriter_Mode){
			case USB_LIGHT_MODE:
			case USB_COMBO_MODE:				
			MountFilesystem();//mount the filesystem so that we have info on it
			while(1){
				key = GetKey();
				modifier = GetModifier(); 
				code = GetHIDKeyCode(key, modifier);
				
				if(code){//if the code is valid, send it
						if ((code == KEY_U)&&Ignore_Flag) code = 0; //if user is holding down U on startup, don't add this U to file.
						Ignore_Flag = 0;
						USBSend(code,modifier);
				}
				Delay_MS(SENSE_DELAY);//perform this loop every X ms.
				HID_Device_USBTask(&Keyboard_HID_Interface);
				Task_Manager(); //do the required usb upkeep tasks, then update the list of scheduled tasks.
				
			}
			break;
			case TEST_MODE:
				USB_Disable(); //USB not needed for testing
				
				parity = (uint8_t)is_low(REED_1) + (uint8_t)is_low(REED_2)+ (uint8_t)is_low(REED_3) + (uint8_t)is_low(REED_4) + (uint8_t)getHallState();
				
				if (parity & 1){
					set_high(LED1);
					set_low(LED2);
				}
				else{
					set_low(LED1);
					set_high(LED2);
				}
			break;
			case HARDWARE_TEST:				
				USB_Disable(); //make sure no host is connected before accessing SD card.
				TestSDHardware();
			case SD_MODE:
				if(UseDummyLoad){set_low(DUMMY_LOAD);configure_as_output(DUMMY_LOAD);}
				USB_Disable(); //make sure no host is connected before accessing SD card.
				LogKeystrokes();
			break;
			case CAL_MODE:
				Calibrate();
				Typewriter_Mode = USB_LIGHT_MODE; //after calibrating, go to usb light mode.
			break;
			case QUICK_CAL_MODE:
				QuickCalibrate();
				Typewriter_Mode = USB_LIGHT_MODE;//after calibrating, go to usb light mode.
			break;
			case BLUETOOTH_MODE:
				if(UseDummyLoad){set_low(DUMMY_LOAD);configure_as_output(DUMMY_LOAD);}
				uart_init(UART_BAUD_SELECT(9600,F_CPU));//initialize the uart with a baud rate of x bps
				Bluetooth_Init();
				while(1){
					while(is_low(BT_CONNECTED)){;}//wait for connection to happen
					key = GetKey();
					modifier = GetModifier();
					code = GetHIDKeyCode(key, modifier);
					if(code){
						Bluetooth_Send(code,modifier);
					}
					Delay_MS(SENSE_DELAY);//perform this loop every X ms.
				}
			break;
			case PANIC_MODE:
				USB_Disable();
				while(1){
					set_high(LED2);
					set_low(LED1);
					Delay_MS(200);
					set_high(LED1);
					set_low(LED2);
					Delay_MS(200);
				}		
			break;
			case SENSITIVITY_MODE:
				Adjust_Sensitivity();
				Typewriter_Mode = USB_LIGHT_MODE;
			break;
			default:
				Typewriter_Mode = PANIC_MODE;
			break;
		};
			
	}//infinite loop
}

ISR (TIMER1_COMPA_vect){ //called each time timer1 counts up to the OCR1A register (every 10 ms, I think)
	TMR1_Count ++;
	TimeoutCounter ++;

		if ((Typewriter_Mode != USB_COMBO_MODE ) && (Typewriter_Mode != USB_LIGHT_MODE)){ //if we are not in the usual usb mode, usb tasks are taken care of by interrupts.  Otherwise, they are addressed in main flow.
			MS_Device_USBTask(&Disk_MS_Interface);
			HID_Device_USBTask(&Keyboard_HID_Interface);
			USB_USBTask();	
		}
		else { //schedule the aforementioned tasks to be handled by the main program.
			Scheduled_Task = MSHIDTASK;
		}
		
		return;
}


void Task_Manager(){ //very basic task manager for tasks scheduled by interrupts.
	if(Scheduled_Task == MSHIDTASK){
		MS_Device_USBTask(&Disk_MS_Interface);
		HID_Device_USBTask(&Keyboard_HID_Interface);
		USB_USBTask();
	}
	else if(Scheduled_Task == HIDTASK){
		HID_Device_USBTask(&Keyboard_HID_Interface);
		USB_USBTask();
	}
	else if(Scheduled_Task == MSTASK){
		MS_Device_USBTask(&Disk_MS_Interface);
		USB_USBTask();
	}
	
	Scheduled_Task = NOTASK;

}


bool WaitForEnumeration(){
	TMR1_Count = 0;
	while(USB_DeviceState != DEVICE_STATE_Configured && TMR1_Count <= 5000){;}//wait for configuration to complete.  timeout after 5 seconds.
	if(USB_DeviceState != DEVICE_STATE_Configured){ //if the device does not enumerate after the timeout period, return false.
		return false;
	}
	else{
		return true;
	}
}


/** Configures the board hardware and chip peripherals for functionality. */
void SetupHardware()
{

	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();
	
	/* Disable JTAG on PortF -- enables Port F pins to function normally.  Datasheet requires this pin to be written repeatedly in order for it to work. 
	("The application software must write this bit to the desired value twice within four cycles to change its value."*/
	MCUCR |= (1 << JTD); 
	MCUCR |= (1 << JTD); 
	MCUCR |= (1 << JTD); 
	MCUCR |= (1 << JTD); 

	/* Disable clock division */
	clock_prescale_set(clock_div_1);
	Config_Interrupts();
	
	/* Hardware Initialization */
	Config_IO();
	Delay_MS(50); //DELAY 50ms after setting IO.
	
	Reed1Polarity = eeprom_read_byte((uint8_t *)REED_1_POLARITY_ADDR);
	Reed2Polarity= eeprom_read_byte((uint8_t *)REED_2_POLARITY_ADDR);
	Reed3Polarity = eeprom_read_byte((uint8_t *)REED_3_POLARITY_ADDR);
	Reed4Polarity = eeprom_read_byte((uint8_t *)REED_4_POLARITY_ADDR);
	
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{


}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
	if(SDCardManager_CheckSDCardOperation()){ //if there is an SD Card present, flip into sd mode if the computer is shut off.
		Typewriter_Mode = SD_MODE;
	}

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
	ConfigSuccess &= MS_Device_ConfigureEndpoints(&Disk_MS_Interface);
	
	USB_Device_EnableSOFEvents();

}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	MS_Device_ProcessControlRequest(&Disk_MS_Interface);
	HID_Device_ProcessControlRequest(&Keyboard_HID_Interface);
}

/** Event handler for the USB device Start Of Frame event. */
void EVENT_USB_Device_StartOfFrame(void)
{
	HID_Device_MillisecondElapsed(&Keyboard_HID_Interface);
}

/** Mass Storage class driver callback function the reception of SCSI commands from the host, which must be processed.
 *
 *  \param[in] MSInterfaceInfo  Pointer to the Mass Storage class interface configuration structure being referenced
 */
bool CALLBACK_MS_Device_SCSICommandReceived(USB_ClassInfo_MS_Device_t* const MSInterfaceInfo)
{
	bool CommandSuccess;

	CommandSuccess = SCSI_DecodeSCSICommand(MSInterfaceInfo);

	return CommandSuccess;
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

//	memcpy((void*)KeyboardReport->KeyCode, (void*)KeyBuffer->KeyCode, 6); //copy the keybuffer into the keyboard report being sent to host.
	KeyboardReport->KeyCode[0] = KeyBuffer->KeyCode[0];

	if (KeyboardReport->KeyCode[0]){ //if there is a key waiting to be sent, then use the modifier that goes with that key.
		KeyboardReport->Modifier = KeyBufferMod;
	}
	else {
		KeyboardReport->Modifier = 0; //otherwise, clear the modifiers so the host doesn't think we are holding down shift or alt or whatever for no reason.
	}
	
//	memset((void*)KeyBuffer->KeyCode,0,6);  //clear keybuffer to clear room for the next key.  This indicates to other routines that the USB buffer is available for sending.
	KeyBuffer->KeyCode[0] = 0;
	KeyBufferMod= 0;

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

	uint8_t* LEDReport = (uint8_t*)ReportData;

	if (*LEDReport & HID_KEYBOARD_LED_NUMLOCK) //if numlock is somehow active,
	  KeyBuffer->KeyCode[0] = HID_KEYBOARD_LED_NUMLOCK; //press numlock key to deactivate it.

	else if (*LEDReport & HID_KEYBOARD_LED_CAPSLOCK) //if capslock is somehow active,
	  KeyBuffer ->KeyCode[0] = HID_KEYBOARD_LED_CAPSLOCK; //press capslock key to deactivate it.

	else if (*LEDReport & HID_KEYBOARD_LED_SCROLLLOCK) //if scrolllock is somehow active,
	  KeyBuffer ->KeyCode[0] = HID_KEYBOARD_LED_SCROLLLOCK; //press scrolllock key to deactivate it.

}


void Init_Mode(){
	uint8_t key;
	uint8_t code;
	uint8_t Default_Mode;
	
	Default_Mode = eeprom_read_byte((uint8_t*)DEFAULT_MODE_ADDR);
	
	key = GetKeySimple(); //read the key that is being held during startup (if any)
	code = GetASCIIKeyCode(key,UPPER);
	
	if (is_low(S1)&&is_low(S2)&&is_low(S3)){ //reset device to known state
			Typewriter_Mode = USB_COMBO_MODE;
			Default_Mode = USB_COMBO_MODE;
			RestoreFactoryDefaults();			
	}
	else if(is_low(S2)&&is_low(S3)){ //debug bluetooth and sd mode
			Typewriter_Mode = BLUETOOTH_MODE;
	}
	else if(is_low(S1)&&is_low(S2)){
			Typewriter_Mode = SD_MODE;
	}
	else if(is_low(S1)&&is_low(S3)){//quick calibration mode
			Typewriter_Mode = QUICK_CAL_MODE;
	}
	else if (is_low(S1)){ //hold down S1 during initialization to calibrate
			Typewriter_Mode = CAL_MODE;
	}
	else if(is_low(S2)){
		Typewriter_Mode = SENSITIVITY_MODE;
	}
	else if (is_low(S3)){ //hold down S3 to enter LED indication mode to test reed switches.
		Typewriter_Mode = TEST_MODE;
	}
	else if(code == 'U'){ //if the letter U is being held by user
		Typewriter_Mode = USB_COMBO_MODE;
		Default_Mode = USB_COMBO_MODE;	
		Ignore_Flag = 1; //tell sensor routine to ignore this U.
	}
	else if(code == 'S'){//if the letter S is being held by the user
		if(SDCardManager_CheckSDCardOperation()){ //if an sd card is present and working, put typewriter into sd mode
			Typewriter_Mode = SD_MODE;
			Default_Mode = SD_MODE;
			Ignore_Flag = 1; //tell sensor routine to ignore this S.
		}
		else{
			Typewriter_Mode = PANIC_MODE; //otherwise, panic to indicate malfunction ... don't change default mode.
			Default_Mode = SD_MODE; //even so, sd mode next time you plug in.
		}
	}
	else if(code == 'B'){ //if the letter B is being held by the user
		if(Bluetooth_Configure()){
			Typewriter_Mode = BLUETOOTH_MODE;
			Default_Mode = BLUETOOTH_MODE;
		}
		else{ //if something goes wrong during configuration...
			Typewriter_Mode = PANIC_MODE; //don't change default mode
		}
	}
	else if(code == 'L'){
			Typewriter_Mode = USB_LIGHT_MODE;
			Default_Mode = USB_LIGHT_MODE;
	}

	else{
		Typewriter_Mode = Default_Mode; //otherwise just go into the last mode selected by user.
	}
	
	eeprom_update_byte((uint8_t*)DEFAULT_MODE_ADDR,Default_Mode);  //Save the new default mode (if changed)
}

