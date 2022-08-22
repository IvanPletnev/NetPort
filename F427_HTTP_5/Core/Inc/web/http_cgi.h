#ifndef WEB_CGI_H
#define WEB_CGI_H

#include "main.h"
#include "string.h"
#include "http_server.h"
//#include "cf_board.h"

/*
 * The command table entry data structure
 */

#define CGI_MAX_COMMANDS       		sizeof(CGI_CMD_ARRAY)/sizeof(CGI_CMD )
#define CGI_MAX_PARAMS		13

typedef struct _tCGIparams {
	uint8_t *params[CGI_MAX_PARAMS]; /* Params extracted from the request URI */
	uint8_t *param_vals[CGI_MAX_PARAMS]; /* Values for each extracted param */
}tCGIparams;

typedef const struct
{
    char *  command;                /* CGI string id  */
    int    (*func)(char *, tCGIparams *); /* actual function to call */
} CGI_CMD;


/*FSL:prototypes*/

/*FSL: extern functions*/

extern int device_configuration_process(char *name, tCGIparams *inputVal);
extern int port1_configuration_process(char *name, tCGIparams *inputVal);
extern int port2_configuration_process(char *name, tCGIparams *inputVal);
extern int login_process(char *name, tCGIparams *inputVal);
extern int restart_process (char *name, tCGIparams *inputVal);
extern int load_defaults_process (char *name, tCGIparams *inputVal);

/*
 * Macros for User InterFace command table entries
 */

#ifndef CGI_DEVICE_CONFIGURATION
#define CGI_DEVICE_CONFIGURATION    \
	{"device.cgi",device_configuration_process}
#endif

#ifndef CGI_PORT1_CONFIGURATION
#define CGI_PORT1_CONFIGURATION    \
    {"port1.cgi",port1_configuration_process}
#endif

#ifndef CGI_PORT2_CONFIGURATION
#define CGI_PORT2_CONFIGURATION    \
    {"port2.cgi",port2_configuration_process}
#endif

#ifndef CGI_LOGIN
#define CGI_LOGIN    \
    {"login.cgi",login_process}
#endif

#ifndef CGI_RESTART
#define CGI_RESTART    \
    {"restart.cgi",restart_process}
#endif

#ifndef CGI_DEFAULTS
#define CGI_DEFAULTS    \
    {"defaults.cgi",load_defaults_process}
#endif


/**
 * Implements a CGI call
 *
 * @param CGI name
 * @param argument to be used for CGI called function 
 * @return returned value by function linked to execute by POST request
 */

int getCgiParams (uint8_t *name, struct pbuf *request);
uint8_t
CGI_parser(int8_t *name, void *request);

#endif
