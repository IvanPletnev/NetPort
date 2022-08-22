/*
 * tcp_server.c
 *
 *  Created on: March 5, 2021
 *      Author: Ivan
 */

#include "cmsis_os.h"
#include "stdio.h"
#include <stdlib.h>
#include <string.h>
#include "converter.h"
#include "ethernetif.h"


void tcp_server_socket_thread(void *arg);
void socketReceiveTask(void *arg);
void socketSendTask(void *arg);

int closeResult=0;
int terminateStat  = 0;
extern int pingSocketNo;
serverParams srvParams[NO_OF_PORTS];

extern struct netif gnetif;

/*-------------------------------------------------------------------------*/
void tcp_server_socket_thread(void *arg) {
	char ip_address[16];
	int sock, newconn, size;
	struct sockaddr_in address;
	serverParams *pSrvParams;
	portSettingsType *pPortSettings;
	volatile uint8_t currentPortNumber = 0;
	struct sockaddr_in remotehost;
	osThreadId socketReceiveTaskHandle;
	osThreadId socketSendTaskHandle;
	int optValue = 1;

	pSrvParams = (serverParams*) arg;
	pPortSettings = &eepromSettings.portSettings[pSrvParams->number];
	currentPortNumber = pSrvParams->number; //выясняем текущий номер порта
	/* Создаём сокет */
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		return;
	}
#ifdef LOCAL_DEBUG
	printf("Created Socket No %i \r\n", sock);
#endif
	/* биндим */
	address.sin_family = AF_INET;
	address.sin_port = htons(pPortSettings->localTcpPort);
	address.sin_addr.s_addr = INADDR_ANY;
	if (bind(sock, (struct sockaddr *)&address, sizeof (address)) < 0) {
		return;
	}
	/* Слушаем */
	listen(sock, 5); // В очереди 5 подключений
	size = sizeof(remotehost);
	while (1) {
		if (pSrvParams->connCounter < MAX_SOCKET_CONN) {
			newconn = accept(sock, (struct sockaddr* )&remotehost,
					(socklen_t* )&size); //принимаем входящее подключение

#ifdef LOCAL_DEBUG
			printf("TCP socket number %i\r\n", newconn);
			printf("PortNum - %u\r\n", ntohs(remotehost.sin_port));
#endif


#ifdef LOCAL_DEBUG
			ip4addr_ntoa_r((const ip4_addr_t*) &remotehost.sin_addr.s_addr,
					ip_address, 16);

			printf("Address - %s\r\n ", ip_address);
#endif
			if (newconn < 0) {//не получилось создать соединение, вероятно памяти не хватает
				close(newconn);
				return;
			} else {
				/*Выключаем алгоритм Нейгла для реалтайма*/
				if (!pPortSettings->nagle){
					setsockopt(newconn, IPPROTO_TCP, TCP_NODELAY, (void*)&optValue, sizeof(optValue));
				}
				pSrvParams->currentFd = newconn;
				pSrvParams->connCounter++;
				pSrvParams->s_address = remotehost.sin_addr.s_addr;
				switch (pSrvParams->number) {
				/*Зажигаем светодиод соединения*/
				case 0:
					HAL_GPIO_WritePin(SOCK1_GPIO_Port, SOCK1_Pin, GPIO_PIN_SET);
					break;
				case 1:
					HAL_GPIO_WritePin(SOCK2_GPIO_Port, SOCK2_Pin, GPIO_PIN_SET);
					break;
				}
				/*Создаём приёмную и передающую задачи, передаём им как аргумент серверную структуру */
				socketReceiveTaskHandle = sys_thread_new(
						pSrvParams->receiveTaskName, socketReceiveTask,
						(void*) &srvParams[currentPortNumber], 256,
						osPriorityNormal);
				pSrvParams->receiveTaskHandle = &socketReceiveTaskHandle;
				socketSendTaskHandle = sys_thread_new(pSrvParams->sendTaskName,
						socketSendTask, (void*) &srvParams[currentPortNumber],
						256, osPriorityNormal);
				pSrvParams->sendTaskHandle = &socketSendTaskHandle;
			}
		}
		osDelay(10);
	}
}
/*-------------------------------------------------------------------------*/

void socketReceiveTask(void *arg) {

	int16_t nbytes = 0;
	serverParams *pSrvParams;
	pSrvParams = (serverParams*) arg;

	while (1) {
		nbytes = recv(pSrvParams->currentFd, pSrvParams->srvRcvBuf, BUFFER_SIZE,
				MSG_WAITALL);
#ifdef LOCAL_DEBUG
		printf("Received %i bytes\r\n", nbytes);
#endif
		if (nbytes <= 0) { //если клиент закрывает соединение или пришёл RESET
#ifdef LOCAL_DEBUG
			printf("Conn closing by remote host or error. Value = %i\r\n", nbytes);
#endif
			__HAL_UART_CLEAR_IDLEFLAG(pSrvParams->uart); //очищаем прерывание
			HAL_UART_AbortReceive(pSrvParams->uart); // выключаем приём UART
			close(pSrvParams->currentFd); //закрываем соединение
				pSrvParams->connCounter--; //уменьшаем счётчик соединений
#ifdef LOCAL_DEBUG
			printf("Close socket %i return %i\r\n", pSrvParams->currentFd,
					closeResult);
#endif
			switch (pSrvParams->number) {
			//Выключаем светодиод
			case 0:
				HAL_GPIO_WritePin(SOCK1_GPIO_Port, SOCK1_Pin, GPIO_PIN_RESET);
				break;
			case 1:
				HAL_GPIO_WritePin(SOCK2_GPIO_Port, SOCK2_Pin, GPIO_PIN_RESET);
				break;
			}
#ifdef LOCAL_DEBUG
			puts("Try to terminate 2 tasks\r\n");
#endif
			/*Удаляем задачи*/
			osThreadTerminate(
					(osThreadId) *pSrvParams->sendTaskHandle);
#ifdef LOCAL_DEBUG
			printf("Terminate send thread = %i\r\n", terminateStat);
			puts("Terminating receive thread\r\n\r\n");
#endif
			osThreadTerminate(
					(osThreadId) *pSrvParams->receiveTaskHandle);
		} else { //если пришли полезные данные
			//переключам соответствующий порт на передачу
			setTxMode(pSrvParams->number);
			//передаём в порт
			HAL_UART_Transmit_DMA(pSrvParams->uart, pSrvParams->srvRcvBuf,
					nbytes);
			switch (pSrvParams->number) {
			case 0:
				port1UartTransmitted += nbytes;
				break;
			case 1:
				port2UartTransmitted += nbytes;
				break;
			}
			//переключаем контекст
			osThreadYield();
		}
	}
}
/*-------------------------------------------------------------------------*/

void socketSendTask(void *arg) {
	UART_RX_Data *pDataToSend;
	osEvent event;
	serverParams *pSrvParams;
	pSrvParams = (serverParams*) arg;

	while (1) {
		/*получаем указатель из очереди*/
		event = osMessageGet((osMessageQId) *pSrvParams->uartRxQueue, osWaitForever);
		if (event.status == osEventMessage) {
			pDataToSend = (UART_RX_Data*) event.value.v;
			/*отправляем данные в сокет*/
			send(pSrvParams->currentFd, pDataToSend->payload, pDataToSend->size,
					0);
			/*Взводим приём из UART с DMA*/
			HAL_UART_Receive_DMA(pSrvParams->uart,
					pSrvParams->uartRxData->payload, BUFFER_SIZE);
			/*переключаем контекст, не дожидаясь очередного тика*/
			osThreadYield();
		}
	}
}

/*-------------------------------------------------------------------------*/
/*контролируем физический линк*/

void ethernetif_notify_conn_changed(struct netif *netif) {
	if (netif_is_link_up(netif)) {
//		HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
		netif_set_up(netif);
		if(eepromSettings.dhcpEnable){
			dhcp_start(&gnetif);
		}
	} else {
//		HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);
		netif_set_down(netif);
		if(eepromSettings.dhcpEnable){
			dhcp_stop(&gnetif);
		}
	}
}
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
