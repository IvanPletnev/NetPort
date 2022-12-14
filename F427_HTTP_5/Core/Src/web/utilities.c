/*********************************************************************\

MODULE NAME:    b64.c

AUTHOR:         Bob Trower 08/04/01

PROJECT:        Crypt Data Packaging

COPYRIGHT:      Copyright (c) Trantor Standard Systems Inc., 2001

NOTE:           This source code may be used as you wish, subject to
                the MIT license.  See the LICENCE section below.

DEPENDENCIES:   None

LICENCE:        Copyright (c) 2001 Bob Trower, Trantor Standard Systems Inc.

                Permission is hereby granted, free of charge, to any person
                obtaining a copy of this software and associated
                documentation files (the "Software"), to deal in the
                Software without restriction, including without limitation
                the rights to use, copy, modify, merge, publish, distribute,
                sublicense, and/or sell copies of the Software, and to
                permit persons to whom the Software is furnished to do so,
                subject to the following conditions:

                The above copyright notice and this permission notice shall
                be included in all copies or substantial portions of the
                Software.

                THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
                KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
                WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
                PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
                OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
                OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
                OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
                SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

VERSION HISTORY:
                Bob Trower 08/04/01 -- Create Version 0.00.00B
                Anthony Huereca 05/11/09 -- Small changes made for an 
                                            embedded device
                Mr Alcantara 06/09/09 -- Networking and String support
                Mr Alcantara 09/23/10 -- Parsing fix
\******************************************************************* */


/* ------------------------ FreeRTOS includes ----------------------------- */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* ------------------------ lwIP includes --------------------------------- */
#include "api.h"
#include "utilities.h"
#include "stdbool.h"
#include "string.h"
/*
** Translation Table as described in RFC1113
*/
#include "http_cgi.h"
static const 
char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

extern uint8_t serviceBuf[2048];
static const 
MIME_TYPE MIME_TYPE_ARRAY[]  = 
{
  MIME_TYPE_HTM,
  MIME_TYPE_SHTML,
  MIME_TYPE_CSS,
  MIME_TYPE_TXT,
  MIME_TYPE_FSL,
  MIME_TYPE_JPG,
  MIME_TYPE_JPEG,
  MIME_TYPE_GIF,
  MIME_TYPE_PNG,
  MIME_TYPE_WOFF,
  MIME_TYPE_JS,
  MIME_TYPE_ICO
};

/**
 * Receives tcp/udp information copying to a static
 *  array or use network buffer directly depending on flag var
 *  Info is received thru tcp/udp/raw connection descriptor
 *  Features: reentrant
 *
 * @param connection descriptor
 * @param static array to be used to copy network buffers
 * @param selector from apps array or use directly from lwIP network buffers
 * @param network buffer pointer of pointer
 * @return length of buffer. Read conn->err for details: 
 *    OK, (ERR_OK) CLSD (ERR_CLSD), TIMEOUT (ERR_TIMEOUT), OUT OF MEM (ERR_MEM)
 */
uint16_t
netconn_rcv_req(void *connec, uint8_t *alloc_rq, void **nbuffer, uint8_t flag)
{ 
  /*joining temp pbuf*/
  struct netbuf *inbuf;
  struct pbuf *q;
  
  struct netconn *conn = (struct netconn *)connec;

  
  /*temporal len*/
  uint16_t len = 0;

  /*FSL: receive the packet*/
  netconn_recv(conn, &inbuf);
  
  /*receiving from the buffer*/
  if( inbuf != NULL )
  {
    /*if receiver is expecting a big rx packet, use it directly from the network buffers*/
    if(flag)
    {
      /*use buffer directly from lwIP network buffers*/
      len = inbuf->ptr->tot_len;
      *nbuffer = (void *)inbuf;
      return len;     
    }
    
    /*if not you can copy it to a small buffer*/
    
    /*start segment index*/
    q = inbuf->ptr;
    do
    {
        memcpy( &alloc_rq[len], q->payload, q->len );
        len += q->len;
    }
    while( ( q = q->next ) != NULL );            

    /*NULL char terminator. Useful for ASCII transfers*/
    alloc_rq[len] = '\0';

    /*free pbuf memory*/
    netbuf_delete(inbuf);
  }
  
  return len;/*return value*/
}

/**
 * Set timeout for selected connection. Call it after netconn_listen(...)
 *   for client or netconn_new(...) for server
 *
 * @param connection descriptor
 * @param time before connection timeout. Zero means forever. Other, TICKs
 * @return none
 */
void
netconn_set_timeout(void *connec, uint32_t timeout)
{
   struct netconn *conn = (struct netconn *)connec;

   conn->recv_timeout = timeout;
}

/**
 * Returns MIME type depending on file extension
 *
 * @param file's name
 * @return pointer to MIME type
 */
uint8_t *
MIME_GetMediaType(uint8_t *array_to_send)
{
  uint8_t i;
  
  
  for(i=0;i<MIME_MAX_TYPES;i++)
  {
     if( strstr((char*)array_to_send,MIME_TYPE_ARRAY[i].mime_extension) != NULL )
     {
         /*extension found*/
         return (uint8_t *)MIME_TYPE_ARRAY[i].mime_type;
     }
  }
  return (uint8_t *)CONTENT_UNKNOWN;/*not found, send unknown MIME TYPE*/
}

/*
** base64_encodeblock
**
** encode 3 8-bit binary bytes as 4 '6-bit' characters
*/
void
base64_encodeblock( unsigned char in[3], unsigned char out[4], int len )
{
    out[0] = cb64[ in[0] >> 2 ];
    out[1] = cb64[ ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ];
    out[2] = (uint8_t) (len > 1 ? cb64[ ((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6) ] : '=');
    out[3] = (uint8_t) (len > 2 ? cb64[ in[2] & 0x3f ] : '=');
}

/*
** decode
**
** decode a base64 encoded stream discarding padding, line breaks and noise
*/
char *
base64_encode(char *source, char *destination)
{
  unsigned char in[3], out[4];
  int i, n, p,len;
  
  i=0;
  n=0;
  p=0;
  len=0;

  *destination=0;
  
  while( source[n]!=0 ) 
  {
    len = 0;
    for( i = 0; i < 3; i++ ) 
    {
      in[i] = source[n];
      if( source[n]!=0  ) 
      {
        len++;
        n++;
      }
      else 
      {
        in[i] = 0;
      }
    }
    
    if( len ) 
    {
      base64_encodeblock( in, out, len );
      for( i = 0; i < 4; i++ ) 
      {
        destination[p]= out[i];
        p++;
      }
    }
  }
  
  destination[p]=0;
  return destination;
}

/**
 * Gets a string in 10.23.12.98 format and returns a four byte IP address array
 *
 * @param string containing IP address
 * @param numerical representation of IP address
 * @return 1 for OK, otherwise failed
 */
uint8_t
ip_convert_address (char *ipstring, char ipaddr[])
{
    int i,j,k,l;

    l = strlen(ipstring);

    for (i = 0; i < l; i++)
    {
        if ((ipstring[i] != IP_DELIMITATOR) && ((ipstring[i] < '0') ||
            (ipstring[i] > '9')))
        {
            return 0;
        }
    }
    /* the characters in the string are at least valid */
    j = 0;
    k = 0;
    for (i = 0; i < l; i++)
    {
        if (ipstring[i] != IP_DELIMITATOR)
        {
            if  (++j > 3)   /* number of digits in portion */
                return 0;
        }
        else
        {
            if (++k > 3)    /* number of periods */
                return 0;

            if (j == 0)     /* number of digits in portion */
                return 0;
            else
                j = 0;
        }
    }

    /* at this point, all the pieces are good, form ip_address */
    k = 0; j = 0;
    for (i = 0; i < l; i++)
    {
        if (ipstring[i] != IP_DELIMITATOR)
        {
            k = (k * 10) + ipstring[i] - '0';
        }
        else
        {
            ipaddr[j++] = (uint8_t)k;
            k = 0;
        }
    }
    ipaddr[j] = (uint8_t)k;
    return 1;
}

/**
 * Gets a string in 00.CF.52.35.00.01 format and returns a six byte MAC address array
 *
 * @param string containing MAC address
 * @param numerical representation of MAC address 
 * @return 1 for OK, otherwise failed
 */
uint8_t
parse_ethaddr (char *ethstr, uint8_t *ethaddr)
{
    int i, j, l, ti;
    char *m;
    char tstr[3];
    uint32_t ul;

    l = strlen(ethstr) + 1;
    j = 0;
    ti = 0;

    for (i = 0; i < l; i++)
    {
        if ((ethstr[i] != MAC_DELIMITATOR) && (ethstr[i] != '\0'))
        {
            if (ti >= 2)
            {
                return 0;
            }
            else
            {
                tstr[ti++] = ethstr[i];
            }
        }
        else
        {
            /*
             * convert string portion started at 'p'
             */
            tstr[ti] = '\0';
            ti = 0;
            ul = strtoul(tstr,&m,16);
            if ((ul == 0) && (m == tstr))
            {
                return 0;
            }
            ethaddr[j++] = (uint8_t)ul;
        }
    }
    if (j == 6)
        return 1;
    else
        return 0;
}

/**
 * Returns an integer number contained in the string starting with
 * the key and in selected base. Stops when a non-number char is found.
 * Example:  "SPIPORT=1&SPOPOLA=2" is the string
 *  Returns an integer 1 with key '=' and base 10
 *
 * @param string to parse
 * @param key to start parsing
 * @param base of number to parse
 * @return parsed numbered, otherwise zero. 
 *         string pointer is modified pointing to the end of the parsed number
 */
uint32_t
parse_number(uint8_t **string, uint8_t key, uint8_t number_base)
{
   /*temporal pointers*/
   char *start;
   
   start = (char *)strchr((const char *)string,key);
   return strtoul((const char *)start+1,(char**)string,(int)number_base);
}

/**
 * Build a string from another string using start and end delimitator pointers
 *
 * @param string to be search
 * @param built string
 * @param start character delimitator (not considered at built string)
 * @param end character delimitator
 * @return none
 */
void
parse_mac_ip_address_string(uint8_t **string, uint8_t *found, uint8_t start, uint8_t end)
{
  uint8_t *temp, *temp2, *tempx;
  uint32_t value;
  
  temp = (uint8_t *)strchr((const char *)string,start);
  tempx = temp;
  temp2 = (uint8_t *)strchr((const char *)temp,end);
  value = (uint32_t)temp2 - (uint32_t)tempx;
  if(value!=1)
    strncpy((char*)found,(const char *)tempx+1,(size_t)value);//built string
  found[value-1] = '\0';//null character
  *string = temp2;
}

/**
 * Search and return a string from a linked list
 *
 * @param linked list in pbuf typedef
 * @param buffer to use. Must be greater than 2 * pbuf max size.
 * @param starting string
 * @param ending string
 * @return pointer to found string. NULL if was not found
 */
uint8_t *
search_string_linked_list(struct pbuf *linked, uint8_t *buffer, uint8_t *search_start, uint8_t *search_end)
{  
   struct pbuf *temp/*,q*/;
   uint8_t *found_string;
//   uint8_t u8Flag = false;
   
   temp = (struct pbuf *)linked;
//   do
//   {
//     /*is there a next buffer on linked list?*/
//     if( ( q = temp->next ) != 0 )
//     {
//        u8Flag = true;
//
//        /*first buffer*/
//        memcpy(buffer,temp->payload,temp->len);
//        /*second buffer*/
//        memcpy(buffer+temp->len,q->payload,q->len);
//        buffer[temp->len + q->len] = '\0';/*NULL string terminator*/
//
//        if( (found_string = search_string(buffer, search_start, search_end)) != NULL )
//        {
//           /*string found*/
//           return found_string;
//        }
//     }
//     else
//     {
//        if( u8Flag == true)
//        {
//           /*string not found*/
//           break;
//        }
//        else
//        {
           /*copy only from first buffer*/
           memcpy(buffer,temp->payload,temp->len);
           /*terminator*/
           buffer[temp->len] = '\0';/*NULL string terminator*/
                     
           if( (found_string = search_string(serviceBuf, search_start, search_end)) != NULL )
           {
              /*string found*/
              return found_string;
           }
//        }
//     }
//     /*update index to the next linked list buffer*/
//     temp = q;
//   }while(1);
   
   /*string not found*/
   return NULL;
}

/**
 * Search string in a continous string
 *
 * @param string to search
 * @param starting string
 * @param ending string
 * @return found string using buffer as allocator. NULL if not found
 */
uint8_t *
search_string(uint8_t *buffer, uint8_t *search_start, uint8_t *search_end)
{
   uint8_t *start_pointer, *end_pointer;
   
   if( ( start_pointer = (uint8_t *)strstr((const char*)buffer,(const char*)search_start)) != NULL )
   {
      /*return the rest of the string until this point*/
      if( search_end == NULL )
      {
         /*return string*/
         return (start_pointer + (uint32_t)strlen((const char*)search_start));
      }
      else
      {
         /*start searching from the the found starting string*/
         if( ( end_pointer = (uint8_t *)strstr((const char *)(start_pointer + (uint32_t)strlen((const char*)search_start)),(const char*)search_end)) != NULL )
         { 
           end_pointer[0] = '\0';/*NULL terminator*/
           
           /*return pointer of string*/
           return (uint8_t*)(start_pointer + (uint32_t)strlen((const char*)search_start));
         }
      }
   }
   /*string not found*/
   return NULL;  
}

uint8_t * searchContent (const uint8_t* file, const uint8_t *crlfcrlf){

	char *startPointer;

	if ((startPointer = strstr((const char*)file, (const char *)crlfcrlf)) != NULL){
		return (uint8_t*)(startPointer + (uint32_t)strlen((const char *)crlfcrlf));
	} else
		return NULL;
}
