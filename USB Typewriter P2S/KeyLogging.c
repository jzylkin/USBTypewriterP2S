/*
 * KeyLogging.c
 *
 * Created: 5/31/2015 12:09:45 PM
 *  Author: Jack
 */ 

#include "Keyboard.h"
#include "Sense_Keys.h"
#include "KeyCodes.h"
#include "globals.h"

char FileName[] = "PAGE0001.TXT";

/** FAT Fs structure to hold the internal state of the FAT driver for the Dataflash contents. */
static FATFS DiskFATState;

/** FAT Fs structure to hold a FAT file handle for the log data write destination. */
static FIL LogFile;

void LogKeystrokes(){
	FRESULT diskstatus;
	FRESULT filestatus;
	FILINFO fileinfo;
	uint16_t filenum;
	uint8_t code = 0;
	uint8_t modifier;
	uint8_t key;
	
	SD_Buffer[0] = '\0'; // A simple way to erase the SD_Buffer string -- first character is now the end of the string;

	diskstatus = MountFilesystem();
	
	if (diskstatus != FR_OK){
		Typewriter_Mode = PANIC_MODE;
		return;
	}
	else{
		GlowGreenLED(VERY_SLOW,SOLID);
	}
	
	filenum = eeprom_read_word((uint16_t *)FILENUM_ADDR); //filenum is the last used filenum, plus 1;
	
	if (filenum>9999){
		filenum = 0;
	} //filenum can only be 4 digits long.

	do{ //increment filenum until a file name is found that does not already exist ("no file" error)
		filenum++; //increment file number
		sprintf(FileName,"PAGE%04d.TXT",filenum); //filename can only be 8 characters long (not including .TXT).
		filestatus = f_stat(FileName, &fileinfo);
	}while(filestatus == FR_OK); //at the end of this loop, FileName is unique
	
	if (filestatus != FR_NO_FILE){ //if the error was not "no file" something went wrong
		Typewriter_Mode = PANIC_MODE; // go into panic mode
		return;
	}
	
	while(!code){ // wait for a key to be pressed before actually opening the file -- this stops lots of empty files from being created
		key = GetKey();
		modifier = GetModifier();
		code = GetASCIIKeyCode(key,modifier);
	}
	
	AddToSDBuffer(code); //save this first key pressed to the buffer.  there will be more, and those will be handled in the main loop

	if (OpenLogFile()!=FR_OK){ //open the new log file.
		Typewriter_Mode = PANIC_MODE; // go into panic mode
		return;
	}
	
	eeprom_write_word((uint16_t *)FILENUM_ADDR,filenum); // save the new filenumber for next time
	
	myTimeoutCounter = 0; //reset timeout

	while(myTimeoutCounter < (SD_TIMEOUT)){ //keep listening for keys and adding them to buffer. Clear buffer occassionally.
			key = GetKey();
			modifier = GetModifier();
			
			code = GetASCIIKeyCode(key, modifier);
			
			if(code){
				if ((code == 's') && Ignore_Flag) code = 0; //if user is holding down S on startup, don't add this to file.
				Ignore_Flag = 0;
				AddToSDBuffer(code); //this adds the character to the sd write buffer.
				myTimeoutCounter = 0; //reset timeout every time a key is pressed.
			}
			if((code == '\r')||(code == '.')||(code == ',')||(code == '!')||(code == '?')||(code == ':')||(code == '\"')){
				GlowGreenLED(MEDIUM, GLOWING);//glow a green led to indicate write in progress.
				WriteToLogFile(); //save your work every time enter key is pressed.
			}
			Delay_MS(SENSE_DELAY);
			
			if ((myTimeoutCounter > SD_SAVE_TIME) && (SD_Buffer[0] != '\0')){
				set_low(GREEN_LED);
				Delay_MS(3000);
				WriteToLogFile();
				set_high(GREEN_LED);
			}
	}
	
			GlowGreenLED(MEDIUM, GLOWING);//glow a green led to indicate write in progress.
			WriteToLogFile(); //save your work then "sleep" -- stop recording keystrokes
			CloseLogFile(); // close log file so a new one can be opened later.
}

/** Opens the log file on the Dataflash's FAT formatted partition according to the current date */
FRESULT OpenLogFile(void)
{
	FRESULT diskstatus;
	
//	if (USB_DeviceState == DEVICE_STATE_Configured){
//		return FR_LOCKED; //the disk is locked if the USB is engaged.  This prevents collision with filesystem read/writes
//	}
	
		diskstatus = f_open(&LogFile, FileName, FA_OPEN_ALWAYS | FA_WRITE);
		f_sync(&LogFile);
		f_close(&LogFile);
		diskstatus = f_open(&LogFile, FileName, FA_OPEN_ALWAYS | FA_WRITE);
		
		f_lseek(&LogFile, LogFile.fsize);
	
	return diskstatus;
}

/** Closes the open data log file on the Dataflash's FAT formatted partition */
void CloseLogFile(void)
{
	/* Sync any data waiting to be written, unmount the storage device */
	f_sync(&LogFile);
	f_close(&LogFile);
}

bool MountFilesystem(){
	bool diskstatus;
	diskstatus = f_mount(&DiskFATState,"",1);
	return diskstatus;
}

uint32_t get_num_of_sectors(){
	static uint32_t tot_sect;
	if(!tot_sect){ //if we have not yet read a valid value into totsect
		tot_sect = (DiskFATState.n_fatent - 2) * DiskFATState.csize;
	}
	return 	tot_sect;
}

bool WriteToLogFile(){
	UINT BytesWritten;
	uint8_t result;
	
	
	BytesWritten = strlen((char*)SD_Buffer);
//	BytesWritten = sprintf(SD_Buffer, "TESTINGTESTING/r/n");//debug 
	f_lseek(&LogFile, LogFile.fsize);
	result = f_write(&LogFile, (void *) SD_Buffer, BytesWritten, &BytesWritten);
	SD_Buffer[0] = '\0'; //a simple way to clear the buffer (equivalent to saving an empty string into the buffer)
	f_sync(&LogFile);
	
	return result;
	
}

void TestSDHardware(){
		FRESULT diskstatus;
		
		diskstatus = MountFilesystem();
		
		if (diskstatus != FR_OK){
			Typewriter_Mode = PANIC_MODE;
			return;
		}
		
		strcpy(FileName, "SDHW.TXT");
		while(1){
		OpenLogFile();
		strcpy((CHAR*)SD_Buffer,"testphrase\n");
		WriteToLogFile();
		CloseLogFile();
		Delay_MS(100);
		}
		
}

void AddToSDBuffer(char character){
	UINT index;
	static char prevcharacter;
	
	index = strlen((char*)SD_Buffer); //index is moved to the end of the string saved in the SD_Buffer.
	if (index >= SD_BUFFER_LENGTH-10){
		return; //take no action if SD_Buffer is nearly full.  this could over-write other variables and cause a mess.
	}
	
	if (character == '\r'){ //special treatment for return character
		if(prevcharacter != '\r'){ //if this is first time \r is pressed, insert a space instead, and save to file.
			SD_Buffer[index] = ' ';
			SD_Buffer[index+1] = '\0';
		}
		else if (SD_Buffer[index-1] == ' '){//if a space was inserted last time in place of \r\n, user has pressed return twice.
			SD_Buffer[index-1] = '\r'; //so put the missing \r\n in now
			SD_Buffer[index] = '\n';
			SD_Buffer[index+1] = '\r';
			SD_Buffer[index+2] = '\n';
			SD_Buffer[index+3] = '\0';
		}
		else { //but if the last character entered was not recorded as a space (\r has already been pressed several times), then call a spade a spade.
			SD_Buffer[index] = '\r';
			SD_Buffer[index+1] = '\n';
			SD_Buffer[index+2] = '\0';
		}
	}
	else if (character == '\b'){ //for a backspace character,
		SD_Buffer[index-1] = '\0'; //turn the previous character into an "end of string" character 
	}
	else{ //the most common scenario -- put a character at the end of the buffer, then follow with a \0;
		SD_Buffer[index] = character;
		SD_Buffer[index+1] = '\0';
	}
	
	prevcharacter = character; // save the character just pressed.
}

