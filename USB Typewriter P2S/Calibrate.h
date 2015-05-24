/*
 * Calibrate.h
 *
 * Created: 12/27/2014 3:07:46 PM
 *  Author: Jack
 */ 


#ifndef CALIBRATE_H_
#define CALIBRATE_H_

void Calibrate();
void TeachHIDKey(char hidkey, int keypressed);
int WaitForKeypress();
void TeachASCIIKey(char asciikey, int keypressed);
void TeachASCIIShiftKey(char asciishiftkey, int keypressed);



#endif /* CALIBRATE_H_ */