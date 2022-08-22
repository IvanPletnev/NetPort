/*
 * jsonParcer.h
 *
 *  Created on: 10 мар. 2021 г.
 *      Author: Ivan
 */

#ifndef INC_JSONPARCER_H_
#define INC_JSONPARCER_H_


#include "jsmn.h"

extern jsmn_parser parcer;
extern jsmntok_t tokens[90]; /* We expect no more than 128 tokens */


void parse_Json (void);



#endif /* INC_JSONPARCER_H_ */
