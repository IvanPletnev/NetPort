/*
 * tcp_server.h
 *
 *  Created on: Nov 5, 2019
 *      Author: Ivan
 */

#ifndef TCP_SERVER_H_
#define TCP_SERVER_H_

#include "main.h"
#include "converter.h"
#include "cmsis_os.h"
#include "lwip.h"
#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"


//#define LOCAL_DEBUG	1
#define TCPECHO_THREAD_PRIO  ( tskIDLE_PRIORITY + 4 )

#define MAX_SOCKET_CONN	1

void tcp_server_socket_thread(void *arg);
void socketReceiveTask(void *arg);
void socketSendTask(void *arg);

#endif /* INC_TCP_SERVER_H_ */
