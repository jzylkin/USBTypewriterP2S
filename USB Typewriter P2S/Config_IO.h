/*
 * Config_IO.h
 *
 * Created: 12/9/2014 9:41:22 AM
 *  Author: Jack
 */ 


#ifndef CONFIG_IO_H_
#define CONFIG_IO_H_

void Config_IO();
void GlowGreenLED(uint8_t speed, uint8_t mode);
#define VERY_SLOW 0
#define SLOW 1
#define MEDIUM 2
#define FAST 3

#define GLOWING 0
#define SOLID 1

#define START 0b10100100
#define STOP 0b10010100


#endif /* CONFIG_IO_H_ */