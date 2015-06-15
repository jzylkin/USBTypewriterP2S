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

char FileName[] = "PAGE 0001";

/** FAT Fs structure to hold the internal state of the FAT driver for the Dataflash contents. */
static FATFS DiskFATState;

/** FAT Fs structure to hold a FAT file handle for the log data write destination. */
static FIL LogFile;

static bool FileIsOpen;

void LogKeystrokes(){
	FRESULT diskstatus;
	FRESULT filestatus;
	FILINFO fileinfo;
	int filenum;
	
	diskstatus = MountFilesystem();
	
	if (diskstatus != FR_OK){
		Typewriter_Mode = PANIC_MODE;
		return;
	}
	
	filenum = eeprom_read_word((uint16_t *)FILENUM_ADDR); //filenum is the last used filenum, plus 1;
	
	do{ //increment filenum until a file name is found that does not already exist 
		filenum++; //increment file number
		sprintf(FileName,"PAGE %04d",filenum);
		filestatus = f_stat(FileName, &fileinfo);
	}while(filestatus == FR_OK);
	
	if (filestatus == FR_NO_FILE){
		eeprom_write_word((uint16_t *)FILENUM_ADDR,filenum); // save the new filenumber for next time
		OpenLogFile();
	}
	else{ //an error occurred
		FileIsOpen = false;
		Typewriter_Mode = PANIC_MODE; // go into panic mode 
		return;
	}
	

	
}

/** Opens the log file on the Dataflash's FAT formatted partition according to the current date */
FRESULT OpenLogFile(void)
{
	FRESULT diskstatus;
	
	if (USB_DeviceState == DEVICE_STATE_Configured){
		return FR_LOCKED; //the disk is locked if the USB is engaged.  This prevents collision with filesystem read/writes
	}
	
		SD_Buffer[0] = '\0'; // A simple way to erase the SD_Buffer string -- first character is now the end of the string;

		diskstatus = f_open(&LogFile, FileName, FA_OPEN_ALWAYS | FA_WRITE);
		f_lseek(&LogFile, LogFile.fsize);
		FileIsOpen = true;
	
	
	return diskstatus;
}

/** Closes the open data log file on the Dataflash's FAT formatted partition */
void CloseLogFile(void)
{
	/* Sync any data waiting to be written, unmount the storage device */
	f_sync(&LogFile);
	f_close(&LogFile);
	FileIsOpen = false;
}

bool MountFilesystem(){
	bool diskstatus;
	diskstatus = f_mount(&DiskFATState,"",1);
	return diskstatus;
}

uint32_t get_num_of_sectors(){
	uint32_t tot_sect = (DiskFATState.n_fatent - 2) * DiskFATState.csize;
	return 	tot_sect;
}

bool WriteToLogFile(){
	UINT BytesWritten;

	uint8_t result;
	
	BytesWritten = strlen((char*)SD_Buffer);
//	BytesWritten = sprintf(SD_Buffer, "TESTINGTESTING/r/n");//debug message

	f_lseek(&LogFile, LogFile.fsize);
	result = f_write(&LogFile, (uint8_t *) SD_Buffer, BytesWritten, &BytesWritten);
	f_sync(&LogFile);
	
	
	
	return result;
	
}

void AddCharToWriteSD_Buffer(char character){
	UINT index;
	static char prevcharacter;
	
	index = strlen((char*)SD_Buffer); //index is moved to the end of the string saved in the SD_Buffer.
	if (index >= SD_BUFFER_LENGTH-10){
		return; //take no action if SD_Buffer is nearly full.  this could over-write other variables and cause a mess.
	}
	
	if (character == '\r'){//if the character is the return character, insert a space instead, and save to file.
		if(prevcharacter != '\r'){
			SD_Buffer[index] = ' ';
			SD_Buffer[index+1] = '\0';
			WriteToLogFile(); //save your work every time enter key is pressed.
		}
		else if (SD_Buffer[index-1] == ' '){//if a space was inserted last time in place of \r\n, user has pressed return twice.
			SD_Buffer[index-1] = '\r'; //so put the missing \r\n in now
			SD_Buffer[index] = '\n';
			SD_Buffer[index+1] = '\r';
			SD_Buffer[index+2] = '\n';
			SD_Buffer[index+3] = '\0';
		}
		else { //but if the last character entered was not a space, then call a spade a spade.
			SD_Buffer[index] = '\r';
			SD_Buffer[index+1] = '\n';
			SD_Buffer[index+2] = '\0';
		}
	}
	else if (character == '\b'){
		SD_Buffer[index-1] = '\0'; //turn the previous character into an "end of string" character 
	}
	else{
		SD_Buffer[index] = character;
		SD_Buffer[index+1] = '\0';
	}
}

