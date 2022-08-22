/*
 * jsonParcer.c
 *
 *  Created on: 10 мар. 2021 г.
 *      Author: Ivan
 */

#include "main.h"
#include "cmsis_os.h"
#include "converter.h"
#include "string.h"
#include "stdio.h"
#include "jsmn.h"
#include "jsonParcer.h"
#include "jWrite.h"

jsmn_parser parcer;
jsmntok_t tokens[90]; /* We expect no more than 128 tokens */
int r;

int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
	if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start
			&& strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
		return 0;
	}
	return -1;
}

void parse_Json(void) {

	int i;

	jsmn_init(&parcer);

	r = jsmn_parse(&parcer, jsonBuffer, strlen(jsonBuffer), tokens,
			sizeof(tokens) / sizeof(tokens[0]));
	if (r < 0) {
		printf("Failed to parse JSON: %d\n", r);
	}
	for (i = 1; i < r; i++) {
		if (jsoneq(jsonBuffer, &tokens[i], "devAlias") == 0) {
			printf("\n\n- Device Alias: %.*s\n",
					tokens[i + 1].end - tokens[i + 1].start,
					&jsonBuffer[0] + tokens[i + 1].start);
			i++;
		} else if (jsoneq(jsonBuffer, &tokens[i], "dhcp") == 0) {
			printf("- DHCP enable: %.*s\n",
					tokens[i + 1].end - tokens[i + 1].start,
					&jsonBuffer[0] + tokens[i + 1].start);
			i++;
		} else if (jsoneq(jsonBuffer, &tokens[i], "ipAddr") == 0) {
			printf("- IP address: %.*s\n",
					tokens[i + 1].end - tokens[i + 1].start,
					&jsonBuffer[0] + tokens[i + 1].start);
			i++;
		} else if (jsoneq(jsonBuffer, &tokens[i], "mask") == 0) {
			printf("- Subnet mask: %.*s\n",
					tokens[i + 1].end - tokens[i + 1].start,
					&jsonBuffer[0] + tokens[i + 1].start);
			i++;
		} else if (jsoneq(jsonBuffer, &tokens[i], "gw") == 0) {
			printf("- Gateway: %.*s\n\n", tokens[i + 1].end - tokens[i + 1].start,
					&jsonBuffer[0] + tokens[i + 1].start);
			i++;
		} else if (jsoneq(jsonBuffer, &tokens[i], "Port1") == 0) {

			int j;
			int len = tokens[i + 1].size;
			for (j = 1; j < len; j++) {
				jsmntok_t *g = &tokens[i + j];
				if (jsoneq(jsonBuffer, &g[j], "portAlias") == 0) {
					printf("- - Port Alias: %.*s\n",
							g[j + 1].end - g[j + 1].start,
							&jsonBuffer[0] + g[j + 1].start);
				} else if (jsoneq(jsonBuffer, &g[j], "portMode") == 0) {
					printf("- - Port mode: %.*s\n",
							g[j + 1].end - g[j + 1].start,
							&jsonBuffer[0] + g[j + 1].start);
				} else if (jsoneq(jsonBuffer, &g[j], "netMode") == 0) {
					printf("- - Network mode: %.*s\n",
							g[j + 1].end - g[j + 1].start,
							&jsonBuffer[0] + g[j + 1].start);
				} else if (jsoneq(jsonBuffer, &g[j], "term") == 0) {
					printf("- - Terminator enable: %.*s\n",
							g[j + 1].end - g[j + 1].start,
							&jsonBuffer[0] + g[j + 1].start);
				} else if (jsoneq(jsonBuffer, &g[j], "baud") == 0) {
					printf("- - Baudrate: %.*s\n",
							g[j + 1].end - g[j + 1].start,
							&jsonBuffer[0] + g[j + 1].start);
				} else if (jsoneq(jsonBuffer, &g[j], "remIpAddr") == 0) {
					printf("- - Remote IP: %.*s\n",
							g[j + 1].end - g[j + 1].start,
							&jsonBuffer[0] + g[j + 1].start);
					len += g[j + 1].size;
					int k;
					for (k = 0; k < g[j + 1].size; k++) {
						jsmntok_t *n = &g[j + k + 2];
						printf("       * %.*s\n", n->end - n->start,
								&jsonBuffer[0] + n->start);
					}
				} else if (jsoneq(jsonBuffer, &g[j], "locTcpPort") == 0) {
					printf("- - Local TCP port: %.*s\n",
							g[j + 1].end - g[j + 1].start,
							&jsonBuffer[0] + g[j + 1].start);
				} else if (jsoneq(jsonBuffer, &g[j], "locUdpPort") == 0) {
					printf("- - Local UDP port: %.*s\n",
							g[j + 1].end - g[j + 1].start,
							&jsonBuffer[0] + g[j + 1].start);
				} else if (jsoneq(jsonBuffer, &g[j], "remTcpPort") == 0) {
					printf("- - Remote TCP port: %.*s\n",
							g[j + 1].end - g[j + 1].start,
							&jsonBuffer[0] + g[j + 1].start);
				} else if (jsoneq(jsonBuffer, &g[j], "remUdpPort") == 0) {
					printf("- - Remote UDP port: %.*s\n",
							g[j + 1].end - g[j + 1].start,
							&jsonBuffer[0] + g[j + 1].start);
				} else if (jsoneq(jsonBuffer, &g[j], "bufSize") == 0) {
					printf("- - Buffer size: %.*s\n\n",
							g[j + 1].end - g[j + 1].start,
							&jsonBuffer[0] + g[j + 1].start);
				}
			}

			i += tokens[i + 1].size + 1;
		} else if (jsoneq(jsonBuffer, &tokens[i], "Port2") == 0) {

			int j;
			int len = tokens[i + 1].size;
			for (j = 1; j < len; j++) {
				jsmntok_t *g = &tokens[i + j];
				if (jsoneq(jsonBuffer, &g[j], "portAlias") == 0) {
					printf("- - Port Alias: %.*s\n",
							g[j + 1].end - g[j + 1].start,
							&jsonBuffer[0] + g[j + 1].start);
				} else if (jsoneq(jsonBuffer, &g[j], "portMode") == 0) {
					printf("- - Port mode: %.*s\n",
							g[j + 1].end - g[j + 1].start,
							&jsonBuffer[0] + g[j + 1].start);
				} else if (jsoneq(jsonBuffer, &g[j], "netMode") == 0) {
					printf("- - Network mode: %.*s\n",
							g[j + 1].end - g[j + 1].start,
							&jsonBuffer[0] + g[j + 1].start);
				} else if (jsoneq(jsonBuffer, &g[j], "term") == 0) {
					printf("- - Terminator enable: %.*s\n",
							g[j + 1].end - g[j + 1].start,
							&jsonBuffer[0] + g[j + 1].start);
				} else if (jsoneq(jsonBuffer, &g[j], "baud") == 0) {
					printf("- - Baudrate: %.*s\n",
							g[j + 1].end - g[j + 1].start,
							&jsonBuffer[0] + g[j + 1].start);
				} else if (jsoneq(jsonBuffer, &g[j], "remIpAddr") == 0) {
					printf("- - Remote IP: %.*s\n",
							g[j + 1].end - g[j + 1].start,
							&jsonBuffer[0] + g[j + 1].start);
					len += g[j + 1].size;
					int k;
					for (k = 0; k < g[j + 1].size; k++) {
						jsmntok_t *n = &g[j + k + 2];
						printf("       * %.*s\n", n->end - n->start,
								&jsonBuffer[0] + n->start);
					}
				} else if (jsoneq(jsonBuffer, &g[j], "locTcpPort") == 0) {
					printf("- - Local TCP port: %.*s\n",
							g[j + 1].end - g[j + 1].start,
							&jsonBuffer[0] + g[j + 1].start);
				} else if (jsoneq(jsonBuffer, &g[j], "locUdpPort") == 0) {
					printf("- - Local UDP port: %.*s\n",
							g[j + 1].end - g[j + 1].start,
							&jsonBuffer[0] + g[j + 1].start);
				} else if (jsoneq(jsonBuffer, &g[j], "remTcpPort") == 0) {
					printf("- - Remote TCP port: %.*s\n",
							g[j + 1].end - g[j + 1].start,
							&jsonBuffer[0] + g[j + 1].start);
				} else if (jsoneq(jsonBuffer, &g[j], "remUdpPort") == 0) {
					printf("- - Remote UDP port: %.*s\n",
							g[j + 1].end - g[j + 1].start,
							&jsonBuffer[0] + g[j + 1].start);
				} else if (jsoneq(jsonBuffer, &g[j], "bufSize") == 0) {
					printf("- - Buffer size: %.*s\n\n",
							g[j + 1].end - g[j + 1].start,
							&jsonBuffer[0] + g[j + 1].start);
				}
			}
		}
	}
}

