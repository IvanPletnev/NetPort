/*
 * tcpclient.c
 *
 *  Created on: 28 февр. 2021 г.
 *      Author: Ivan
 */
#include "main.h"
#include "tcp_client.h"
#include "tcp_server.h"
#include "lwip/ip_addr.h"
#include "cmsis_os.h"

extern RNG_HandleTypeDef hrng;

void tcp_client_socket_thread(void *arg) {

	serverParams *pSrvParams = (serverParams*) arg;
	portSettingsType *pPortSettings =
			&eepromSettings.portSettings[pSrvParams->number];
	volatile uint8_t currentPortNumber = 0;
	int socket, newconn = 0;
	struct ip4_addr tempIp;
	struct sockaddr_in addr;
	osThreadId socketReceiveTaskHandle;
	osThreadId socketSendTaskHandle;
	int optValue = 1;

	currentPortNumber = pSrvParams->number;

	int16_t freertosMinimumHeeapSize = 0;
	memset(&addr, 0, sizeof(addr));
	addr.sin_len = sizeof(addr);
	addr.sin_family = AF_INET;
	IP4_ADDR(&tempIp, pPortSettings->remoteIpAddress[0],
			pPortSettings->remoteIpAddress[1],
			pPortSettings->remoteIpAddress[2],
			pPortSettings->remoteIpAddress[3]);
	addr.sin_addr.s_addr = tempIp.addr;
	addr.sin_port = htons(pPortSettings->remoteTcpPort);

	osDelay(3000);

	for (;;){
		if (pSrvParams->connCounter < MAX_SOCKET_CONN){
			socket = socket (AF_INET, SOCK_STREAM, 0);
			if (socket < 0){
				puts ("Create client socket failed\r\n");
				return;
			} else {
				printf ("1. TCP client socket # %i for Port # %u has been created\r\n", socket, pSrvParams->number);
				newconn = connect(socket, (struct sockaddr*)&addr, sizeof (addr));
				if (newconn < 0){
					printf("Unable to connect to remote server, newconn = %i\r\n\r\n", newconn);
					close (socket);
					osDelay(10000);
				} else {
					if (!pPortSettings->nagle){
						setsockopt(newconn, IPPROTO_TCP, TCP_NODELAY, (void*)&optValue, sizeof(optValue));
					}
					printf("Connection to remote server established. Socket No %i \r\n\r\n", socket);
						freertosMinimumHeeapSize = xPortGetMinimumEverFreeHeapSize();
						printf ("Minimum ever free heap size is %u\r\n", freertosMinimumHeeapSize);
					printf ("Size of parameters structure = %i\r\n", sizeof (eepromSettings));
					pSrvParams->currentFd = socket;
					pSrvParams->connCounter++;
					switch (pSrvParams->number) {
					case 0:
						HAL_GPIO_WritePin(SOCK1_GPIO_Port, SOCK1_Pin, GPIO_PIN_SET);
						break;
					case 1:
						HAL_GPIO_WritePin(SOCK2_GPIO_Port, SOCK2_Pin, GPIO_PIN_SET);
						break;
					}
					socketReceiveTaskHandle = sys_thread_new(
							pSrvParams->receiveTaskName, socketReceiveTask,
							(void*) &srvParams[currentPortNumber], 256,
							osPriorityNormal);
					pSrvParams->receiveTaskHandle = &socketReceiveTaskHandle;
					socketSendTaskHandle = sys_thread_new(pSrvParams->sendTaskName,
							socketSendTask, (void*) &srvParams[currentPortNumber],
							256, osPriorityNormal);
					pSrvParams->sendTaskHandle = &socketSendTaskHandle;
				}//connect
			}//bind
		}//if (connCounter)
		osDelay(10);
	}//infinite loop
} //thread


