/*
 * udp_server.c
 *
 *  Created on: 27 февр. 2021 г.
 *      Author: Ivan
 */
#include "udp_server.h"
#include "tcp_server.h"
#include "stddef.h"

void udp_server_socket_thread(void *arg) {

	osDelay(3000);

	int udpsock;
	struct sockaddr_in address;
	volatile uint8_t currentPortNumber = 0;
	serverParams *pSrvParams;
	portSettingsType *pPortSettings;
	osThreadId udpSendTaskHandle;
	int received = 0;
	socklen_t size;
	struct sockaddr_in remotehost;
	size = sizeof(remotehost);

	pSrvParams = (serverParams*) arg;
	pPortSettings = &eepromSettings.portSettings[pSrvParams->number];
	currentPortNumber = pSrvParams->number; //выясняем текущий номер порта

	if ((udpsock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		return;
	}
	else {
		pSrvParams->currentFd = udpsock;
		address.sin_family = AF_INET;
		address.sin_port = htons(pPortSettings->localUdpPort);
		address.sin_addr.s_addr = INADDR_ANY;
		if (bind(udpsock, (struct sockaddr *)&address, sizeof (address)) < 0) {
			close(udpsock);
			return;
		}
		else {
			udpSendTaskHandle = sys_thread_new(
					pSrvParams->sendTaskName, udp_send_task,
					(void*) &srvParams[currentPortNumber], 256,
					osPriorityNormal);
			pSrvParams->sendTaskHandle = &udpSendTaskHandle;
		}
	}
	memset(&remotehost, 0, sizeof (remotehost));

	while (1) {
		received = recvfrom(pSrvParams->currentFd, pSrvParams->srvRcvBuf, BUFFER_SIZE, MSG_WAITALL,
					(struct sockaddr* )&remotehost, &size);
		if (received > 0) {
			pSrvParams->remotehost = &remotehost;
			setTxMode(pSrvParams->number);
			HAL_UART_Transmit_DMA(pSrvParams->uart, pSrvParams->srvRcvBuf,
					received);
			switch (pSrvParams->number) {
			case 0:
				port1UartTransmitted += received;
				break;
			case 1:
				port2UartTransmitted += received;
				break;
			}
			osThreadYield();
		}
	}
}
/*-------------------------------------------------------------------------*/

void udp_send_task(void *arg) {

	UART_RX_Data *pDataToSend;
	osEvent event;
	serverParams *pSrvParams;
	portSettingsType *pPortSettings;
	struct sockaddr_in remotehost;
	pSrvParams = (serverParams*) arg;
	pPortSettings = &eepromSettings.portSettings[pSrvParams->number];
	struct ip4_addr tempIp;
	memset(&remotehost, 0, sizeof(struct sockaddr));
	IP4_ADDR(&tempIp, pPortSettings->remoteIpAddress[0],
			pPortSettings->remoteIpAddress[1],
			pPortSettings->remoteIpAddress[2],
			pPortSettings->remoteIpAddress[3]);
	remotehost.sin_port = htons(pPortSettings->remoteUdpPort);
	remotehost.sin_addr.s_addr = tempIp.addr;
	remotehost.sin_family = AF_INET;
	remotehost.sin_len = sizeof (struct sockaddr_in);
	pSrvParams->remotehost = &remotehost;

	while (1) {
		event = osMessageGet((osMessageQId) *pSrvParams->uartRxQueue, osWaitForever);
		if (event.status == osEventMessage) {
			pDataToSend = (UART_RX_Data*) event.value.v;
			sendto(pSrvParams->currentFd, pDataToSend->payload, (size_t)pDataToSend->size,
					0, (struct sockaddr*) pSrvParams->remotehost, sizeof (struct sockaddr));
			HAL_UART_Receive_DMA(pSrvParams->uart,
					pSrvParams->uartRxData->payload, BUFFER_SIZE);
			osThreadYield();
		}
	}
}

