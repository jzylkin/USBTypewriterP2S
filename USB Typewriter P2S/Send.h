/*
 * Send.h
 *
 * Created: 12/27/2014 1:41:19 PM
 *  Author: Jack
 */ 


#ifndef SEND_H_
#define SEND_H_


void USBSend(uint8_t code, uint8_t mod);
void USBSendString(char *str);
void USBSendNumber(uint8_t number);
void USBSendASCII(uint8_t number);
void USBSendPROGString(const char* ProgStr);
#endif /* SEND_H_ */