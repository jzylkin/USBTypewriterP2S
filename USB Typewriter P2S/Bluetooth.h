/*
 * Bluetooth.h
 *
 * Created: 2/12/2015 11:10:06 PM
 *  Author: Jack
 */ 


#ifndef BLUETOOTH_H_
#define BLUETOOTH_H_

bool Bluetooth_Enter_CMD_Mode();
void Bluetooth_Reset();
void Bluetooth_Enter_Proxy_Mode();
void Bluetooth_Exit_Proxy_Mode();
bool Bluetooth_Disconnect();
void BT_Wake();
void BT_Sleep();
bool Bluetooth_Connect();

void Bluetooth_Exit_CMD_Mode();
bool Bluetooth_Configure();
bool BluetoothInquire();
void Bluetooth_Init();
void Bluetooth_Send(uint8_t key, uint8_t modifier);
uint8_t Get_Bluetooth_State();
#define INACTIVE 0
#define INITIALIZED 1

#if MODULE_NAME==EHONG
bool Bluetooth_Send_CMD(char* command, bool verbose);
bool Bluetooth_Send_PROGMEM_CMD(const char* progcommand, bool verbose);
bool Get_Response(bool verbose);
//bluetooth states for the bt_state variable
#define BLUETOOTH_RESPONSE_DELAY 100 //delay for a little bit to make sure bluetooth has time to respond after each command --100ms recommended by makeymakey.
#define BLUETOOTH_RESET_DELAY 500 //500ms recommended by datasheet

#elif MODULE_NAME==RN42
bool Get_Response();
bool Bluetooth_Send_CMD(char* command);
bool Bluetooth_Enter_CMD_Mode();
void Bluetooth_Exit_CMD_Mode();
#define BLUETOOTH_RESPONSE_DELAY 100 //delay for a little bit to make sure bluetooth has time to respond after each command --100ms recommended by makeymakey.
#define BLUETOOTH_RESET_DELAY 700 //500ms recommended by datasheet
#endif //modulename

#endif /* BLUETOOTH_H_ */