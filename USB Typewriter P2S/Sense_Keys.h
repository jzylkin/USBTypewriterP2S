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

uint8_t GetKeyCode(uint8_t key, uint8_t modifier);

void LoadKeyCodeTables();

void ClearKeyCodeTables();


#endif /* SENSE_KEYS_H_ */