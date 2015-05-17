/** \file
 *
 *  Header file for SDCardManager.c.
 */
 
#ifndef _SD_MANAGER_H
#define _SD_MANAGER_H

	/* Includes: */
		#include <avr/io.h>
		
		#include "../Keyboard.h"
		#include "../Descriptors.h"

		#include <LUFA/Common/Common.h>
		#include <LUFA/Drivers/USB/USB.h>

	/* Defines: */
		/** Block size of the device. This is kept at 512 to remain compatible with the OS despite the underlying
		 *  storage media (Dataflash) using a different native block size. Do not change this value.
		 */
		#define VIRTUAL_MEMORY_BLOCK_SIZE           512

	/* Function Prototypes: */
		void SDCardManager_Init(void);
		uint32_t SDCardManager_GetNbBlocks(void);
		void SDCardManager_WriteBlocks(USB_ClassInfo_MS_Device_t* const MSInterfaceInfo, const uint32_t BlockAddress, uint16_t TotalBlocks);
		void SDCardManager_ReadBlocks(USB_ClassInfo_MS_Device_t* const MSInterfaceInfo, uint32_t BlockAddress, uint16_t TotalBlocks);
		void SDCardManager_ResetDataflashProtections(void);
		bool SDCardManager_CheckSDCardOperation(void);
		uintptr_t SDCardManager_WriteBlockHandler(uint8_t* buffer, uint16_t offset);
		uint8_t SDCardManager_ReadBlockHandler(uint8_t* buffer, uint16_t offset);
		
#endif
