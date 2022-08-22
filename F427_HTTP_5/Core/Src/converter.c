/*
 * converter.c
 *
 *  Created on: 29 янв. 2021 г.
 *      Author: Ivan
 */

#include "main.h"
#include <string.h>
#include <stdlib.h>
#include "stdio.h"
#include "converter.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "jWrite.h"

uint8_t interfaceMode[NO_OF_PORTS];
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;

extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_usart2_tx;
extern DMA_HandleTypeDef hdma_usart3_rx;
extern DMA_HandleTypeDef hdma_usart3_tx;

extern osMessageQId UART_RX_Q_1Handle;
extern osMessageQId UART_RX_Q_2Handle;

UART_RX_Data uartRXData1;
UART_RX_Data uartRXData2;

uint16_t settingStructureSize = 0;

deviceSettingsType defaultSettings;
deviceSettingsType eepromSettings;

uint32_t port1UartReceived = 0;
uint32_t port2UartReceived = 0;

uint32_t port1UartTransmitted = 0;
uint32_t port2UartTransmitted = 0;

char jsonBuffer[800];
int	buflen = 800;

void deviceInit(void) {

	huart3.Init.BaudRate = eepromSettings.portSettings[0].baudRate;
	  if (HAL_UART_Init(&huart3) != HAL_OK)
	  {
	    Error_Handler();
	  }
	huart2.Init.BaudRate = eepromSettings.portSettings[1].baudRate;
	  if (HAL_UART_Init(&huart2) != HAL_OK)
	  {
	    Error_Handler();
	  }

	setComPortMode(0, eepromSettings.portSettings[0].portMode);
	setComPortMode(1, eepromSettings.portSettings[1].portMode);

	setTerminator(0, eepromSettings.portSettings[0].termEnable);
	setTerminator(1, eepromSettings.portSettings[1].termEnable);

	srvParams[0].number = 0;
	srvParams[0].uart = &huart3;
	srvParams[0].receiveTaskName = "rcvTask0";
	srvParams[0].sendTaskName = "sndTask0";
	srvParams[0].uartRxQueue = &UART_RX_Q_1Handle;
	srvParams[0].uartRxData = &uartRXData1;

	srvParams[1].number = 1;
	srvParams[1].uart = &huart2;
	srvParams[1].receiveTaskName = "rcvTask1";
	srvParams[1].sendTaskName = "sndTask1";
	srvParams[1].uartRxQueue = &UART_RX_Q_2Handle;
	srvParams[1].uartRxData = &uartRXData2;

	switch (eepromSettings.portSettings[0].networkMode){
	case TCP_SERVER_MODE:
		sys_thread_new("tcpserver0", tcp_server_socket_thread,
				(void*) &srvParams[0], 256, osPriorityNormal);
		break;
	case TCP_CLIENT_MODE:
		sys_thread_new("tcpClient0", tcp_client_socket_thread,
				(void*) &srvParams[0], 1024, osPriorityNormal);
		break;
	case UDP_SERVER_MODE:
		sys_thread_new("udpServer0", udp_server_socket_thread,
				(void*) &srvParams[0], 256, osPriorityNormal);
		break;
	}

	switch (eepromSettings.portSettings[1].networkMode){
	case TCP_SERVER_MODE:
		sys_thread_new("tcpserver1", tcp_server_socket_thread,
				(void*) &srvParams[1], 256, osPriorityNormal);
		break;
	case TCP_CLIENT_MODE:
		sys_thread_new("tcpClient1", tcp_client_socket_thread,
				(void*) &srvParams[1], 1024, osPriorityNormal);
		break;
	case UDP_SERVER_MODE:
		sys_thread_new("udpServer1", udp_server_socket_thread,
				(void*) &srvParams[1], 256, osPriorityNormal);
		break;
	}
}

void setComPortMode(uint8_t port, uint8_t ifMode) {
	switch (port) {
	case 0:
		switch (ifMode) {
		case RS485_MODE:
			HAL_GPIO_WritePin(IFMODE1_GPIO_Port, IFMODE1_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(RE1_GPIO_Port, RE1_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(DE1_GPIO_Port, DE1_Pin, GPIO_PIN_RESET);
			break;
		case RS485_4W_MODE:
			HAL_GPIO_WritePin(IFMODE1_GPIO_Port, IFMODE1_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(RE1_GPIO_Port, RE1_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(DE1_GPIO_Port, DE1_Pin, GPIO_PIN_SET);
			break;
		case RS422_MODE:
			HAL_GPIO_WritePin(IFMODE1_GPIO_Port, IFMODE1_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(RE1_GPIO_Port, RE1_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(DE1_GPIO_Port, DE1_Pin, GPIO_PIN_SET);
			break;
		}
		break;
	case 1:
		switch (ifMode) {
		case RS485_MODE:
			HAL_GPIO_WritePin(IFMODE2_GPIO_Port, IFMODE2_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(RE2_GPIO_Port, RE2_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(DE2_GPIO_Port, DE2_Pin, GPIO_PIN_RESET);
			break;
		case RS485_4W_MODE:
			HAL_GPIO_WritePin(IFMODE1_GPIO_Port, IFMODE2_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(RE1_GPIO_Port, RE1_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(DE1_GPIO_Port, DE1_Pin, GPIO_PIN_SET);
			break;
		case RS422_MODE:
			HAL_GPIO_WritePin(IFMODE2_GPIO_Port, IFMODE2_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(RE2_GPIO_Port, RE2_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(DE2_GPIO_Port, DE2_Pin, GPIO_PIN_SET);
			break;
		}
	}
}

void setTerminator (uint8_t portNo, uint8_t termEn){
	switch (portNo) {
	case 0:
		if (termEn){
			HAL_GPIO_WritePin(TERM1_GPIO_Port, TERM1_Pin, GPIO_PIN_SET);
		} else {
			HAL_GPIO_WritePin(TERM1_GPIO_Port, TERM1_Pin, GPIO_PIN_RESET);
		}
		break;
	case 1:
		if (termEn){
			HAL_GPIO_WritePin(TERM2_GPIO_Port, TERM2_Pin, GPIO_PIN_SET);
		} else {
			HAL_GPIO_WritePin(TERM2_GPIO_Port, TERM2_Pin, GPIO_PIN_RESET);
		}
		break;
	}
}

void setTxMode (uint8_t portNo){
	if (eepromSettings.portSettings[portNo].portMode == RS422_MODE || eepromSettings.portSettings[portNo].portMode == RS485_4W_MODE) return;
	switch (portNo){
	case 0:
		HAL_GPIO_WritePin(RE1_GPIO_Port, RE1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(DE1_GPIO_Port, DE1_Pin, GPIO_PIN_SET);
		break;
	case 1:
		HAL_GPIO_WritePin(RE2_GPIO_Port, RE2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(DE2_GPIO_Port, DE2_Pin, GPIO_PIN_SET);
	}
}

void setRxMode (uint8_t portNo){
	if (eepromSettings.portSettings[portNo].portMode == RS422_MODE || eepromSettings.portSettings[portNo].portMode == RS485_4W_MODE) return;
	switch (portNo){
	case 0:
		HAL_GPIO_WritePin(RE1_GPIO_Port, RE1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(DE1_GPIO_Port, DE1_Pin, GPIO_PIN_RESET);
		break;
	case 1:
		HAL_GPIO_WritePin(RE2_GPIO_Port, RE2_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(DE2_GPIO_Port, DE2_Pin, GPIO_PIN_RESET);
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  /* Prevent unused argument(s) compilation warning */
	if (huart == &huart3){
		setRxMode(0);
	}
	if (huart == &huart2){
		setRxMode(1);
	}
  /* NOTE: This function should not be modified, when the callback is needed,
           the HAL_UART_RxCpltCallback could be implemented in the user file
   */
}

void USER_UART_IRQHandler(UART_HandleTypeDef *huart)
{
	if( __HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE)!= RESET)   //Judging whether it is idle interruption
	{
		__HAL_UART_CLEAR_IDLEFLAG(huart);                     //Clear idle interrupt sign (otherwise it will continue to enter interrupt)
		USER_UART_IDLECallback(huart);                          //Call interrupt handler
	}
}


void USER_UART_IDLECallback(UART_HandleTypeDef *huart) {

	HAL_UART_DMAStop(huart);

	if (huart->Instance == USART3){
		uartRXData1.size = BUFFER_SIZE - __HAL_DMA_GET_COUNTER(&hdma_usart3_rx);
		port1UartReceived += uartRXData1.size;
		HAL_UART_Receive_DMA(huart, uartRXData1.payload, BUFFER_SIZE);
		if(osMessagePut(UART_RX_Q_1Handle,(uint32_t)&uartRXData1,0)==osOK){
		}
	}
	if (huart->Instance == USART2){
		uartRXData2.size = BUFFER_SIZE - __HAL_DMA_GET_COUNTER(&hdma_usart2_rx);
		port2UartReceived += uartRXData2.size;
		HAL_UART_Receive_DMA(huart, uartRXData2.payload, BUFFER_SIZE);
	  if(osMessagePut(UART_RX_Q_2Handle,(uint32_t)&uartRXData2,0)==osOK){
	  }
	}
}

//char* mallocByString(const char *str) {
//    char* p = (char*) malloc(strlen(str) + 1);
//    strcpy(p, str);
//    return p;
//}

void deviceInitDefaults (void){

	deviceSettingsType * pDevSettings = &defaultSettings;
	strcpy ((char *)defaultSettings.deviceAlias, "NetPort 2102");
	strcpy ((char *)defaultSettings.userName, "admin");
	strcpy ((char *)defaultSettings.password, "admin");
	strcpy ((char *)defaultSettings.portSettings[0].portAlias, "Port 1");
	strcpy ((char *)defaultSettings.portSettings[1].portAlias, "Port 2");

	pDevSettings->dhcpEnable = 0;
	pDevSettings->ipAddress[0] = 192;
	pDevSettings->ipAddress[1] = 168;
	pDevSettings->ipAddress[2] = 1;
	pDevSettings->ipAddress[3] = 5;
	pDevSettings->subnetMask[0] = 255;
	pDevSettings->subnetMask[1] = 255;
	pDevSettings->subnetMask[2] = 255;
	pDevSettings->subnetMask[3] = 0;
	pDevSettings->gateway[0] = 192;
	pDevSettings->gateway[1] = 168;
	pDevSettings->gateway[2] = 1;
	pDevSettings->gateway[3] = 1;
	pDevSettings->webInterfaceEn = 1;
	pDevSettings->portSettings[0].portMode = RS485_MODE;
	pDevSettings->portSettings[0].baudRate = 57600;
	pDevSettings->portSettings[0].networkMode = TCP_SERVER_MODE;
	pDevSettings->portSettings[0].termEnable = 0;
	pDevSettings->portSettings[0].localTcpPort = 61300;
	pDevSettings->portSettings[0].localUdpPort = 64303;
	pDevSettings->portSettings[0].remoteTcpPort = 58125;
	pDevSettings->portSettings[0].remoteUdpPort = 51000;
	pDevSettings->portSettings[0].remoteIpAddress[0] = 192;
	pDevSettings->portSettings[0].remoteIpAddress[1] = 168;
	pDevSettings->portSettings[0].remoteIpAddress[2] = 1;
	pDevSettings->portSettings[0].remoteIpAddress[3] = 250;
	pDevSettings->portSettings[0].nagle = 0;
	pDevSettings->portSettings[1].portMode = RS485_MODE;
	pDevSettings->portSettings[1].baudRate = 57600;
	pDevSettings->portSettings[1].networkMode = UDP_SERVER_MODE;
	pDevSettings->portSettings[1].termEnable = 0;
	pDevSettings->portSettings[1].localTcpPort = 61320;
	pDevSettings->portSettings[1].localUdpPort = 64000;
	pDevSettings->portSettings[1].remoteTcpPort = 58025;
	pDevSettings->portSettings[1].remoteUdpPort = 50770;
	pDevSettings->portSettings[1].remoteIpAddress[0] = 192;
	pDevSettings->portSettings[1].remoteIpAddress[1] = 168;
	pDevSettings->portSettings[1].remoteIpAddress[2] = 1;
	pDevSettings->portSettings[1].remoteIpAddress[3] = 250;
	pDevSettings->portSettings[1].nagle = 0;

//	printf ("Default settings struct size = %u\r\n\r\n", sizeof (defaultSettings));
	eepromWrite((uint8_t*)&defaultSettings, sizeof (defaultSettings));

}

void putParamsToJson (void){
	int err;

	jwOpen(jsonBuffer, buflen, JW_OBJECT, JW_COMPACT);
	jwObj_string("devAlias", (char*) eepromSettings.deviceAlias);
	jwObj_int("dhcp", (unsigned int)eepromSettings.dhcpEnable);
	jwObj_array("ipAddr");
		jwArr_int(eepromSettings.ipAddress[0]);
		jwArr_int(eepromSettings.ipAddress[1]);
		jwArr_int(eepromSettings.ipAddress[2]);
		jwArr_int(eepromSettings.ipAddress[3]);
	jwEnd();
	jwObj_array("mask");
		jwArr_int(eepromSettings.subnetMask[0]);
		jwArr_int(eepromSettings.subnetMask[1]);
		jwArr_int(eepromSettings.subnetMask[2]);
		jwArr_int(eepromSettings.subnetMask[3]);
	jwEnd();
	jwObj_array("gw");
		jwArr_int(eepromSettings.gateway[0]);
		jwArr_int(eepromSettings.gateway[1]);
		jwArr_int(eepromSettings.gateway[2]);
		jwArr_int(eepromSettings.gateway[3]);
	jwEnd();
	jwObj_object("Port1");
		jwObj_string("portAlias", (char*) eepromSettings.portSettings[0].portAlias);
		jwObj_int("portMode", (unsigned int)eepromSettings.portSettings[0].portMode);
		jwObj_int("netMode", (unsigned int)eepromSettings.portSettings[0].networkMode);
		jwObj_int("term", (unsigned int)eepromSettings.portSettings[0].termEnable);
		jwObj_int("baud", eepromSettings.portSettings[0].baudRate);
		jwObj_array("remIpAddr");
			jwArr_int(eepromSettings.portSettings[0].remoteIpAddress[0]);
			jwArr_int(eepromSettings.portSettings[0].remoteIpAddress[1]);
			jwArr_int(eepromSettings.portSettings[0].remoteIpAddress[2]);
			jwArr_int(eepromSettings.portSettings[0].remoteIpAddress[3]);
		jwEnd();
		jwObj_int("locTcpPort", eepromSettings.portSettings[0].localTcpPort);
		jwObj_int("locUdpPort", eepromSettings.portSettings[0].localUdpPort);
		jwObj_int("remTcpPort", eepromSettings.portSettings[0].remoteTcpPort);
		jwObj_int("remUdpPort", eepromSettings.portSettings[0].remoteUdpPort);
	jwEnd();
	jwObj_object("Port2");
		jwObj_string("portAlias", (char*) eepromSettings.portSettings[1].portAlias);
		jwObj_int("portMode", (unsigned int)eepromSettings.portSettings[1].portMode);
		jwObj_int("netMode", (unsigned int)eepromSettings.portSettings[1].networkMode);
		jwObj_int("term", (unsigned int)eepromSettings.portSettings[1].termEnable);
		jwObj_int("baud", eepromSettings.portSettings[1].baudRate);
		jwObj_array("remIpAddr");
			jwArr_int(eepromSettings.portSettings[0].remoteIpAddress[0]);
			jwArr_int(eepromSettings.portSettings[1].remoteIpAddress[1]);
			jwArr_int(eepromSettings.portSettings[2].remoteIpAddress[2]);
			jwArr_int(eepromSettings.portSettings[3].remoteIpAddress[3]);
		jwEnd();
		jwObj_int("locTcpPort", eepromSettings.portSettings[1].localTcpPort);
		jwObj_int("locUdpPort", eepromSettings.portSettings[1].localUdpPort);
		jwObj_int("remTcpPort", eepromSettings.portSettings[1].remoteTcpPort);
		jwObj_int("remUdpPort", eepromSettings.portSettings[1].remoteUdpPort);
	jwEnd();

	err= jwClose();								// close and get error code

//	printf (jsonBuffer);
	if( err != JWRITE_OK )
		printf ( "Error: %s at function call %d\n", jwErrorToString(err), jwErrorPos() );

}
