/*
 * Bluetooth.c
 *
 * Created: 2/12/2015 11:09:37 PM
 *  Author: Jack
 */ 
#include "Keyboard.h"
#include "Bluetooth.h"
#include <util/delay.h>

void Bluetooth_Send(uint8_t key, uint8_t modifier){
/*	0	0x0c	Length of packet (12)
	1	0x00	Forward HID Report.
	2	0xa1	HID Input Report Header
	3	0x01	Keyboard Report ID
	4	0x00	Modifier Keys (none pressed)
	5	0x00	Reserved
	6	0x04	Keycode 1 ('A' key pressed on USA keyboard)
	7	0x00	Keycode 2 (not pressed)
	8	0x00	Keycode 3 (not pressed)
	9	0x00	Keycode 4 (not pressed)
	10	0x00	Keycode 5 (not pressed)
	11	0x00	Keycode 6 (not pressed)*/
		
	uart_putc(12);
    uart_putc(0x00);
	uart_putc(0xa1);
	uart_putc(0x01);
	uart_putc(0x00);//mod
	uart_putc(0x00);
	uart_putc(key);
	uart_putc(0x00);
	uart_putc(0x00);
	uart_putc(0x00);
	uart_putc(0x00);
	uart_putc(0x00);
	
	//clear the keystroke
	
	uart_putc(12);
	uart_putc(0x00);
	uart_putc(0xa1);
	uart_putc(0x01);
	uart_putc(0x00);//mod
	uart_putc(0x00);
	uart_putc(0x00);
	uart_putc(0x00);
	uart_putc(0x00);
	uart_putc(0x00);
	uart_putc(0x00);
	uart_putc(0x00);
	
}