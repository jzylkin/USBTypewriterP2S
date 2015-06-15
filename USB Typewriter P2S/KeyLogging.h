/*
 * KeyLogging.h
 *
 * Created: 5/31/2015 12:10:04 PM
 *  Author: Jack
 */ 


#ifndef KEYLOGGING_H_
#define KEYLOGGING_H_

void LogKeystrokes();
		uint32_t get_num_of_sectors();
		FRESULT OpenLogFile(void);
		void CloseLogFile(void);
bool WriteToLogFile();
bool MountFilesystem();
void TestSDHardware();
#endif /* KEYLOGGING_H_ */