/*
 * Sense_Keys.h
 *
 * Created: 12/10/2014 10:27:47 AM
 *  Author: Jack
 */ 


#ifndef SENSE_KEYS_H_
#define SENSE_KEYS_H_

uint8_t GetModifier();
uint8_t GetKeySimple();

uint8_t GetKey();

unsigned long long ReadSensor();

void LoadKeyCodeTables();

void ClearKeyCodeTables();

uint8_t GetASCIIKeyCode(uint8_t key, uint8_t modifier);

uint8_t GetHIDKeyCode(uint8_t key, uint8_t* modifier); //modifier is passed by reference so it can be edited if necessary.

bool getHallState();

void InitializeEeprom();
void LoadEepromParameters();
void RestoreFactoryDefaults();
#endif /* SENSE_KEYS_H_ */