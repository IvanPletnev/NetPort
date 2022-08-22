/*
 * httpServer.c
 *
 *  Created on: 4 апр. 2021 г.
 *      Author: Ivan
 */

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/*--------------------------------------------------------------------------------------*/
#include "lwip.h"
#include "lwip/api.h"
#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include "lwip/apps/fs.h"
/*--------------------------------------------------------------------------------------*/
#include "string.h"
#include "main.h"
/*--------------------------------------------------------------------------------------*/
#include "converter.h"
#include "http_server.h"
#include "http_ssi.h"
#include "http_cgi.h"
#include "utilities.h"
#include "stdbool.h"
#include "stdio.h"


extern const char *emg_static_ffs_filenames[];
extern const unsigned char *emg_static_ffs_ptrs[];
extern const unsigned short emg_static_ffs_len[];
extern const uint8_t index_shtml[];/*FSL: default webpage*/
extern const unsigned char emg_static_ffs_nof;

extern LANGUAGES WEB_LANGUAGES[];
extern uint8_t default_language;
extern uint8_t reset_var;

extern uint16_t secCounter;

uint8_t isAuthorized = 0;




extern uint8_t serviceBuf[2048];

/**
 * Returns Language greeting to be used for web page
 *
 * @param HTTP request to look for language string
 * @return index to language to use
 */
uint8_t
HTTP_GetLanguageIndex(void *string)
{
    uint8_t i;
    uint8_t *buffer;

    /*search for string on linked list: reusing SD buffer*/
    buffer = search_string_linked_list(string, serviceBuf, (uint8_t *)HTTP_CNTNT_LNG_REQUEST, (uint8_t *)STRING_END);

    for(i=0;i<max_number_of_languages();i++)
    {
      //contains array
      if(!strncmp((const char *)buffer,(const char *)WEB_LANGUAGES[i].language_code,GREETING_LANGUAGE_CODE_LENGTH))
      {
          return i;
      }
    }

    return 0;//default English
}
/*--------------------------------------------------------------------------------------*/

/**
 * Send "Transfer-Encoding: chunked" footer
 *
 * @param connection descriptor
 * @return none
 */
void
HTTP_SendDynamicFooter(struct netconn *pcb)
{
   //generated data
   netconn_write(pcb,"\r\n0\r\n\r\n",str_len("\r\n0\r\n\r\n"),0);
}


/*--------------------------------------------------------------------------------------*/


/**
 * Send page not found error to HTTP connection
 *
 * @param connection descriptor
 * @return none
 */
void
HTTP_SendError(struct netconn *pcb)
{
     //header
     netconn_write(pcb, HTTP_WRONG_HEADER, str_len(HTTP_WRONG_HEADER), 0 );
     //data
     netconn_write(pcb, HTTP_WRONG_DATA, str_len(HTTP_WRONG_DATA), 0 );
}
/*--------------------------------------------------------------------------------------*/

/**
 * Send a packet with "Transfer-Encoding: chunked" rules
 *
 * @param connection descriptor
 * @param data to send
 * @param data length
 * @param copy or a reference to be used for data
 * @return none
 */
void
HTTP_SendDynamicPacket(struct netconn *pcb, const void *data, u16_t len, u8_t apiflags)
{
   char number[16];/*hold number*/

   if(len)
   {
     sprintf(number,"\r\n%x\r\n",len);

     //chunk length
     netconn_write(pcb,number,strlen(number),NETCONN_COPY);
     //generated data
     netconn_write(pcb,data,len,apiflags);
   }
}

/*--------------------------------------------------------------------------------------*/

/**
 * Send a web page data replacing strings with SSI rules
 *
 * @param connection descriptor
 * @param temporal buffer for replacing
 * @param static data to be parsed with SSI rules
 * @param static data length
 * @param send "Transfer-Encoding: chunked" footer
 * @return none
 */
void HTTP_SendDynamicData(struct netconn *pcb,
		uint8_t *array_to_send, uint8_t *dynamic_info,
		uint16_t dynamic_info_length, uint8_t flag) {
	//FSL: dynamic web page implementation:
	uint8_t *index_a, *index_b, *index_c;
	uint8_t *dynamic_start = (uint8_t*) dynamic_info;
	uint8_t *temporal_buffer = (uint8_t*) array_to_send;
	uint8_t *dynamic_start_temp;
	uint16_t dynamic_length = dynamic_info_length;

	while (dynamic_length > 0) {
		if ((index_a = (uint8_t*) strstr((char*) dynamic_start, SSI_START))
				!= NULL) {
			if ((index_c = (uint8_t*) strstr((char*) index_a, SSI_END)) != NULL) {
				index_b = index_a + str_len(SSI_START);/*considering NULL character*/

				//send no dynamic part
				HTTP_SendDynamicPacket(pcb, &dynamic_start[0],
						(u16_t) (index_a - dynamic_start), 0);

				//replace with SSI parser
				if (SSI_parser(index_b, temporal_buffer)) {
					//send dynamic information get from SSI replacement
					HTTP_SendDynamicPacket(pcb, temporal_buffer,
							(u16_t) strlen((char*)temporal_buffer), NETCONN_COPY);
				}
				//update web page index
				dynamic_start_temp = dynamic_start;
				dynamic_start =
						index_c + str_len(SSI_END)/*considering NULL character*/;
				dynamic_length = dynamic_length
						- (uint16_t) (dynamic_start - dynamic_start_temp);
			} else {
				//send static content
				HTTP_SendDynamicPacket(pcb, dynamic_start,
						(uint16_t) dynamic_length, 0);
				break;
			}
		} else {
			//send static content
			HTTP_SendDynamicPacket(pcb, dynamic_start,
					(uint16_t) dynamic_length, 0);
			break;
		}
	}
	if (!flag) {
		/*FSL: end terminator for chunked encondig*/
		HTTP_SendDynamicFooter(pcb);
	}
	return;/*FSL: page has already been sent*/
}


/*--------------------------------------------------------------------------------------*/
void
HTTP_SendData(struct netconn *pcb, uint8_t dynamic_flag, uint8_t *array_to_send, uint8_t *dynamic_info, uint16_t dynamic_info_length)
{
#if HTTP_DYNAMIC_SUPPORT == 1
    /*dynamic process*/
    if( dynamic_flag )
    {
       HTTP_SendDynamicData(pcb, (uint8_t *)array_to_send, dynamic_info, dynamic_info_length, 0/*send footer*/);
    }
    /*static process*/
    else
    {
#endif
       //FSL: send as it is: OK
       netconn_write(pcb, &dynamic_info[0], (uint16_t)dynamic_info_length, 0);
#if HTTP_DYNAMIC_SUPPORT == 1
    }
#endif
}

/*--------------------------------------------------------------------------------------*/

void HTTP_SendHeader(struct netconn *pcb, uint8_t *array_to_send, uint16_t dynamic_info_length, uint8_t dynamic_flag) {

#if HTTP_DYNAMIC_SUPPORT == 1
	/*get MIME type to be sent with the HTTP response*/
	uint8_t *dynamic_type = MIME_GetMediaType(array_to_send);

	//FSL: build HTTP 200 OK and that stuff!!
	strcpy((char*) array_to_send, HTTP_HEADER_OK_PART1);
	strcat((char*) array_to_send, (const char*) dynamic_type);

	if (!dynamic_flag) {
#endif
		strcat((char*) array_to_send, HTTP_HEADER_OK_PART2);
		sprintf((char*)array_to_send, "%s%d", array_to_send,
				dynamic_info_length);
		strcat((char*) array_to_send, HTTP_CONN_KEEP_ALIVE);
		strcat((char*) array_to_send, STRING_DOUBLE_END);	//static terminator
#if HTTP_DYNAMIC_SUPPORT == 1
	} else {
		strcat((char*) array_to_send, HTTP_HEADER_CONN_CLOSE);
		strcat((char*) array_to_send, HTTP_DYMAMIC_HEADER);		//dynamic header
	}
#endif
	/*send header*/
	netconn_write(pcb, &array_to_send[0],
			(uint16_t )strlen((char* )array_to_send), NETCONN_COPY);
}

/*--------------------------------------------------------------------------------------*/

uint8_t HTTP_isHtmlExtension (uint8_t * name){
	if ((strstr((const char*) name, ".shtml") != NULL)
			|| (strstr((const char*) name, ".html") != NULL)) {
		return true;
	}
	return false;
}

/*--------------------------------------------------------------------------------------*/

uint8_t HTTP_IsDynamicExtension(uint8_t *array_to_send) {
	if ((strstr((const char*) array_to_send, ".shtml") != NULL)
			|| (strstr((const char*) array_to_send, ".fsl") != NULL)) {
		return true;
	}
	return false;
}

/*--------------------------------------------------------------------------------------*/

void
HTTP_SendWebPage(struct netconn *pcb, uint8_t *array_to_send, uint8_t *dynamic_info, uint16_t dynamic_info_length)
{
     /*get if a dynamic process is needed*/
     uint8_t dynamic_flag = HTTP_IsDynamicExtension(array_to_send);

     //send header
     HTTP_SendHeader(pcb, array_to_send, dynamic_info_length, dynamic_flag);
     //send data
     HTTP_SendData(pcb, dynamic_flag, array_to_send, dynamic_info, dynamic_info_length);

     return;
}

/*--------------------------------------------------------------------------------------*/

/*
 * Original function with offset to raw content
 *
 * */
uint8_t
HTTP_GetFileFromInternalMemory(struct netconn *pcb, uint8_t *name)
{
   uint8_t i;
   uint8_t *file_pointer;

   //look for the file in internal memory
   for(i=0;i < emg_static_ffs_nof; i++)
   {
      //a file matchs with internal flash system
      if(!strcmp((const char*) name,(const char*) emg_static_ffs_filenames[i]))
      {
          /*data and data length*/
    	  file_pointer  = searchContent(emg_static_ffs_ptrs[i], (const uint8_t*) STRING_DOUBLE_END);
    	  uint16_t headerLength = (uint16_t)((uint32_t)file_pointer - (uint32_t)emg_static_ffs_ptrs[i]);
          uint16_t file_length = (uint16_t)emg_static_ffs_len[i] - headerLength;

          /*send web page*/
          HTTP_SendWebPage(pcb, (uint8_t *)name, file_pointer, file_length);
          return 0;
      }
   }
   return 1;/*file not found*/
}
/*--------------------------------------------------------------------------------------*/

void processRequest(struct netconn *pcb, struct pbuf *request, uint8_t *space,
		uint8_t flag) {

	char *end_temp;
	char *buffer;
	uint16_t u16Length;

	if (flag == GET_FLAG) {
		buffer = (char*) ((uint32_t) request->payload
				+ (uint32_t) (str_len(HTTP_GET_REQUEST)));
	} else if (flag == POST_FLAG) {
		buffer = (char*) ((uint32_t) request->payload
				+ (uint32_t) (str_len(HTTP_POST_REQUEST)));
	}
	//looking for file's name: assuming name's at first linked-list packet
	if ((end_temp = (char*) strchr((const char*) buffer, ' ')) != NULL) {
		if ((u16Length = (uint8_t) (end_temp - buffer)) != 0) {
			//build name string: assuming name's at first linked-list packet
			memcpy(space, (const char*) buffer, u16Length); //cpy file's name for http request
			printf("File %.*s requested\r\n",u16Length, space);
			space[u16Length] = '\0';        //string delimitator
		} else {// Empty GET request
			if (isAuthorized){
				strcpy((char*)space, (const char*) "index.shtml");
			} else strcpy((char*)space, (const char*) "login.html");
			HTTP_GetFileFromInternalMemory(pcb, space);
			puts ("Send default page index.shtml");
		}
		if (flag == POST_FLAG) {
			if (getCgiParams(space, request)) {

			} else {
				//cgi not found
				HTTP_SendError(pcb);
				return;
			}
		}
		if (HTTP_isHtmlExtension(space)){
			if (!isAuthorized){
				strcpy((char*)space, (const char*) "login.html");
			}
		}
		if (!HTTP_GetFileFromInternalMemory(pcb, space)) {
			//TODO: more support if file was succesfully found and sent
		} else {
			strcpy((char*)space, (const char*) "e404.html");
			HTTP_GetFileFromInternalMemory(pcb, space);
		}
	}
}

/*--------------------------------------------------------------------------------------*/

void receiveRequest(struct netconn *conn, uint8_t *buffer) {

	struct netbuf *inbuf;
	struct pbuf *q;
	err_t recv_err;
	u16_t buflen;

	recv_err = netconn_recv(conn, &inbuf);
	q = inbuf->ptr;
	secCounter = 0;
	if (recv_err == ERR_OK) {
		if (netconn_err(conn) == ERR_OK) {
			netbuf_data(inbuf, (void**) &buffer, &buflen);
			printf("Received %u bytes\r\n\r\n", buflen);
			if (strstr((const char*) q->payload, HTTP_GET_REQUEST) != NULL) {
				puts ("GET request\r\n\r\n");
				processRequest(conn, q, serviceBuf, GET_FLAG);
			} else if (strstr((const char*) q->payload, HTTP_POST_REQUEST) != NULL) {
				puts ("POST request\r\n\r\n");
				processRequest(conn, q, serviceBuf, POST_FLAG);
			} else {
				/*FSL:not supported request*/
				HTTP_SendError(conn);
				puts ("Unsupported request\r\n");
			}
		}
	}
	netconn_close(conn);

	netbuf_delete(inbuf);
}

/*--------------------------------------------------------------------------------------*/

void httpServerTask (void *arg) {
	struct netconn *conn, *newconn;
	err_t err, accept_err;

	/*FSL:holding information*/
	uint8_t *tempBuffer;

	if ((tempBuffer = (uint8_t *)mem_malloc((mem_size_t)HTTP_WORKING_SPACE)) == 0){
		puts ("Allocating memory for tempBuf failed\r\n");
		goto http_server_exit;
	} else {
		printf ("tempBuf allocated, size %u\r\n", HTTP_WORKING_SPACE);
	}
	/* Create a new TCP connection handle */
	conn = netconn_new(NETCONN_TCP);

	if (conn != NULL) {
		/* Bind to port 80 (HTTP) with default IP address */
		err = netconn_bind(conn, NULL, 80);

		if (err == ERR_OK) {
			/* Put the connection into LISTEN state */
			netconn_listen(conn);

			while (1) {
				/* accept any icoming connection */
				accept_err = netconn_accept(conn, &newconn);
				if (accept_err == ERR_OK) {
					/* serve connection */
					puts ("Connection established. Accepted\r\n");
					receiveRequest(newconn, tempBuffer);
					/* delete connection */
					netconn_delete(newconn);
				} else puts ("Accept failed\r\n");
			}
		}
	}
	http_server_exit:
		mem_free(tempBuffer);
		osThreadTerminate(NULL);
		return;
}



