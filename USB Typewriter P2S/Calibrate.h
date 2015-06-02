/*
 * Calibrate.h
 *
 * Created: 12/27/2014 3:07:46 PM
 *  Author: Jack
 */ 


#ifndef CALIBRATE_H_
#define CALIBRATE_H_

void Calibrate();
void TeachHIDKey(char teachkey, int keypressed, char Modifier);
int WaitForKeypress();
void TeachASCIIKey(char teachkey, int keypressed, char Modifier);



#endif /* CALIBRATE_H_ */