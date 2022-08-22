/*
 * udp_server.h
 *
 *  Created on: 27 февр. 2021 г.
 *      Author: Ivan
 */

#ifndef INC_UDP_SERVER_H_
#define INC_UDP_SERVER_H_

#include "main.h"
#include "cmsis_os.h"
#include "lwip.h"
#include "converter.h"

void udp_server_socket_thread(void *arg);
void udp_send_task(void *arg);

#endif /* INC_UDP_SERVER_H_ */
