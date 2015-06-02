/*
             LUFA Library
     Copyright (C) Dean Camera, 2009.
              
  dean [at] fourwalledcubicle [dot] com
      www.fourwalledcubicle.com
*/

/*
  Copyright 2009  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, and distribute this software
  and its documentation for any purpose and without fee is hereby
  granted, provided that the above copyright notice appear in all
  copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaim all warranties with regard to this
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
 *  Functions to manage the physical dataflash media, including reading and writing of
 *  blocks of data. These functions are called by the SCSI layer when data must be stored
 *  or retrieved to/from the physical storage media. If a different media is used (such
 *  as a SD card or EEPROM), functions similar to these will need to be generated.
 */

#define  INCLUDE_FROM_SDCARDMANAGER_C
#include "SDCardManager.h"
#include "../Lib/FatFS/diskio.h"
#include "../IO_Macros.h"
#include "../globals.h"

static bool SDCard_Present= false;

void SDCardManager_Init(void)
{

	if(disk_initialize(0)==FR_OK){ //if the disk initializes correctly
		SDCard_Present = true;
	}
	else{
		SDCard_Present = false; //tell other functions that the SD Card is missing/malfunctioned
	}
}


uintptr_t SDCardManager_WriteBlockHandler(uint8_t* buffer, uint16_t offset)
{

	/* Check if the endpoint is currently empty */
	if (!(Endpoint_IsReadWriteAllowed()))
	{
		/* Clear the current endpoint bank */
		Endpoint_ClearOUT();
		
		/* Wait until the host has sent another packet */
		if (Endpoint_WaitUntilReady())
		  return 0;
	}
	
	/* Write one 16-byte chunk of data to the dataflash */
	buffer[0+offset] = Endpoint_Read_8();
	buffer[1+offset] = Endpoint_Read_8();
	buffer[2+offset] = Endpoint_Read_8();
	buffer[3+offset] = Endpoint_Read_8();
	buffer[4+offset] = Endpoint_Read_8();
	buffer[5+offset] = Endpoint_Read_8();
	buffer[6+offset] = Endpoint_Read_8();
	buffer[7+offset] = Endpoint_Read_8();
	buffer[8+offset] = Endpoint_Read_8();
	buffer[9+offset] = Endpoint_Read_8();
	buffer[10+offset] = Endpoint_Read_8();
	buffer[11+offset] = Endpoint_Read_8();
	buffer[12+offset] = Endpoint_Read_8();
	buffer[13+offset] = Endpoint_Read_8();
	buffer[14+offset] = Endpoint_Read_8();
	buffer[15+offset] = Endpoint_Read_8();
	
	return 16;
}

/** Writes blocks (OS blocks, not Dataflash pages) to the storage medium, the board dataflash IC(s), from
 *  the pre-selected data OUT endpoint. This routine reads in OS sized blocks from the endpoint and writes
 *  them to the dataflash in Dataflash page sized blocks.
 *
 *  \param[in] BlockAddress  Data block starting address for the write sequence
 *  \param[in] TotalBlocks   Number of blocks of data to write
 */
void SDCardManager_WriteBlocks(USB_ClassInfo_MS_Device_t* const MSInterfaceInfo, uint32_t BlockAddress, uint16_t TotalBlocks)
{
	uint16_t  BytesWritten;

	/* Wait until endpoint is ready before continuing */
	if (Endpoint_WaitUntilReady())
	  return;
	
	while (TotalBlocks)
	{
		//Reset BytesWritten every time you finish writing a block, Dummy!
		BytesWritten = 0;
		
		while((BytesWritten<512)){
			BytesWritten += SDCardManager_WriteBlockHandler((uint8_t*)SD_Buffer, BytesWritten);
			if (USB_DeviceState != DEVICE_STATE_Configured){return;}//if the device is not configured, exit out of this
			if (MSInterfaceInfo->State.IsMassStoreReset){return;}
		}
		
		disk_write (0, (uint8_t*)SD_Buffer, BlockAddress, 1);//write to disk 0, from Buffer array, into BlockAddress, Write only 1 sector (block);
	
		/* Decrement the blocks remaining counter and reset the sub block counter */
		BlockAddress++;
		TotalBlocks--;			
		
	}

	/* If the endpoint is empty, clear it ready for the next packet from the host */
	if (!(Endpoint_IsReadWriteAllowed()))
	  Endpoint_ClearOUT();
}

/** Reads blocks (OS blocks, not Dataflash pages) from the storage medium, the board dataflash IC(s), into
 *  the pre-selected data IN endpoint. This routine reads in Dataflash page sized blocks from the Dataflash
 *  and writes them in OS sized blocks to the endpoint.
 *
 *  \param[in] BlockAddress  Data block starting address for the read sequence
 *  \param[in] TotalBlocks   Number of blocks of data to read
 */
void SDCardManager_ReadBlocks(USB_ClassInfo_MS_Device_t* const MSInterfaceInfo, uint32_t BlockAddress, uint16_t TotalBlocks)
{
	uint16_t BytesRead;
	
	/* Wait until endpoint is ready before continuing */
	if (Endpoint_WaitUntilReady())
	  return;
	
	while (TotalBlocks)
	{		
		//Reset tally of BytesRead every time a new block is accessed.  
		BytesRead = 0;
		
		/* Read a data block from the SD card */		
		disk_read (0, (uint8_t *) SD_Buffer, BlockAddress, 1);//  read disk 0,  into Buffer,  starting at block address,  read only 1 sector (block=sector)
		
		while(BytesRead<512){  //send the results to the usb endpoint buffer, 16 bytes at a time.
			BytesRead += SDCardManager_ReadBlockHandler((uint8_t*)SD_Buffer, BytesRead); // BytesRead increases 16 every time handler is called, if all goes well.
			if (MSInterfaceInfo->State.IsMassStoreReset){return;}
			if (USB_DeviceState != DEVICE_STATE_Configured){Typewriter_Mode = PANIC_MODE; return;}
		}
		
		/* Decrement the blocks remaining counter */
		BlockAddress++;
		TotalBlocks--;
		

		
	}
	
	/* If the endpoint is full, send its contents to the host */
	if (!(Endpoint_IsReadWriteAllowed()))
	  Endpoint_ClearIN();
}

/*Forwarding buffered data to the endpoint (and therefore on to the host) in 16 byte chunks. This function is called by ReadBlocks.  
 *  \param[in] BlockAddress  Data block starting address for the read sequence
 *  \param[in] TotalBlocks   Number of blocks of data to read
 *  \output -- number of bytes forwarded successfully.  Return 16 if they are forwarded successfully, 0 if host rejects them.
 */
uint8_t SDCardManager_ReadBlockHandler(uint8_t* buffer, uint16_t offset)
{

	/* Check if the endpoint is currently full */
	if (!(Endpoint_IsReadWriteAllowed()))
	{
		/* Clear the endpoint bank to send its contents to the host */
		Endpoint_ClearIN();
		
		/* Wait until the endpoint is ready for more data */
		if (Endpoint_WaitUntilReady())
		return 0;
	}
	
	Endpoint_Write_8(buffer[0+offset]);
	Endpoint_Write_8(buffer[1+offset]);
	Endpoint_Write_8(buffer[2+offset]);
	Endpoint_Write_8(buffer[3+offset]);
	Endpoint_Write_8(buffer[4+offset]);
	Endpoint_Write_8(buffer[5+offset]);
	Endpoint_Write_8(buffer[6+offset]);
	Endpoint_Write_8(buffer[7+offset]);
	Endpoint_Write_8(buffer[8+offset]);
	Endpoint_Write_8(buffer[9+offset]);
	Endpoint_Write_8(buffer[10+offset]);
	Endpoint_Write_8(buffer[11+offset]);
	Endpoint_Write_8(buffer[12+offset]);
	Endpoint_Write_8(buffer[13+offset]);
	Endpoint_Write_8(buffer[14+offset]);
	Endpoint_Write_8(buffer[15+offset]);
	
	return 16;
}

/** Performs a simple test on the attached Dataflash IC(s) to ensure that they are working.
 *
 *  \return Boolean true if all media chips are working, false otherwise
 */
bool SDCardManager_CheckSDCardOperation(void)
{	
	return SDCard_Present;
}

