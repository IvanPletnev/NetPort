/*
 * converter.h
 *
 *  Created on: 7 ����. 2019 �.
 *      Author: Ivan
 */
#ifndef CONVERTER_H_
#define CONVERTER_H_
#include "cmsis_os.h"
#include "lwip/inet.h"

#include "eeprom.h"
#include "tcp_server.h"
#include "tcp_client.h"
#include "udp_server.h"

#define NO_OF_PORTS	2
#define BUFFER_SIZE	1000

typedef enum _portMode {
	RS485_MODE,
	RS485_4W_MODE,
	RS422_MODE
}portModeType;

typedef enum _networkMode {
	TCP_SERVER_MODE,
	TCP_CLIENT_MODE,
	UDP_SERVER_MODE
}networkModeType;


typedef struct _UART_RX_Data {
	uint16_t size;
	uint8_t payload[BUFFER_SIZE];
}UART_RX_Data;

extern osMessageQId UART_RX_Q_1Handle;
extern osMessageQId UART_RX_Q_2Handle;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern UART_RX_Data uartRXData1;
extern UART_RX_Data uartRXData2;

extern char jsonBuffer[800];
void deviceInit(void);
void setTerminator (uint8_t portNo, uint8_t termEn);
void setComPortMode(uint8_t port, uint8_t ifMode);
void setTxMode (uint8_t portNo);
void setRxMode (uint8_t portNo);
void USER_UART_IRQHandler(UART_HandleTypeDef *huart);
void USER_UART_IDLECallback(UART_HandleTypeDef *huart);
void deviceInitDefaults (void);
void putParamsToJson (void);

typedef struct _serverParams {
	uint8_t number;
	UART_HandleTypeDef *uart;
	const char *sendTaskName;
	const char *receiveTaskName;
	osThreadId *sendTaskHandle;
	osThreadId *receiveTaskHandle;
	osMessageQId *uartRxQueue;
	struct sockaddr_in *remotehost;
	in_addr_t s_address;
	int currentFd;
	uint8_t connCounter;
	uint8_t srvRcvBuf[BUFFER_SIZE];
	UART_RX_Data *uartRxData;
}serverParams;

typedef struct _portSettings {
	uint8_t portAlias[64];
	portModeType portMode;
	networkModeType networkMode;
	uint8_t termEnable;
	uint32_t baudRate;
	uint8_t remoteIpAddress[4];
	uint16_t localTcpPort;
	uint16_t localUdpPort;
	uint16_t remoteTcpPort;
	uint16_t remoteUdpPort;
	uint8_t nagle;
}portSettingsType;

typedef struct _deviceSettings{
	uint8_t deviceAlias[64];
	uint8_t dhcpEnable;
	uint8_t ipAddress[4];
	uint8_t subnetMask[4];
	uint8_t gateway[4];
	uint8_t userName[16];
	uint8_t password[16];
	uint8_t webInterfaceEn;
	portSettingsType portSettings[NO_OF_PORTS];
}deviceSettingsType;

extern serverParams srvParams[NO_OF_PORTS];
extern deviceSettingsType defaultSettings;
extern deviceSettingsType eepromSettings;

extern uint32_t port1UartReceived;
extern uint32_t port2UartReceived;

extern uint32_t port1UartTransmitted;
extern uint32_t port2UartTransmitted;

#endif /* CONVERTER_H_ */
