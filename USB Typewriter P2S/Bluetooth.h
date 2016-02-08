/*
 * Bluetooth.h
 *
 * Created: 2/12/2015 11:10:06 PM
 *  Author: Jack
 */ 


#ifndef BLUETOOTH_H_
#define BLUETOOTH_H_
void Bluetooth_Send(uint8_t key, uint8_t modifier);
void Bluetooth_Init();
bool Bluetooth_Send_CMD(char* command, bool verbose);
bool Bluetooth_Enter_CMD_Mode();
void Bluetooth_Reset();
bool Bluetooth_Disconnect();
bool Bluetooth_Connect();
bool Get_Response(bool verbose);
void Bluetooth_Exit_CMD_Mode();
bool Bluetooth_Configure();
bool BluetoothInquire();
#define BLUETOOTH_RESPONSE_DELAY 100 //delay for a little bit to make sure bluetooth has time to respond after each command --100ms recommended by makeymakey.
#define BLUETOOTH_RESET_DELAY 500 //500ms recommended by datasheet	

#endif /* BLUETOOTH_H_ */