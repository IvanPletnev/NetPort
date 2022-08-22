#include "lwip.h"
#include "lwip/api.h"
#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include "lwip/apps/fs.h"

#include "converter.h"
#include "http_cgi.h"
#include "stdlib.h"
#include "main.h"
#include "utilities.h"
#include "stdbool.h"
#include "stdio.h"

/*variable to hold if a reset must be issued*/
uint8_t reset_var = false;
/*reusing buffers for CGI processing*/
uint8_t serviceBuf[2048] = {0};

extern uint16_t secCounter;
/*list of cgi's for this process*/
CGI_CMD CGI_CMD_ARRAY[]  = 
{
	CGI_DEVICE_CONFIGURATION,              /*index.shtml: configuration file*/
	CGI_PORT1_CONFIGURATION,               /*port1.shtml: configuration file*/
	CGI_PORT2_CONFIGURATION,               /*port2.shtml: configuration file*/
	CGI_LOGIN,
	CGI_RESTART,
	CGI_DEFAULTS
};

tCGIparams CGI_params;

extern deviceSettingsType eepromSettings;
/**
 * Implements a CGI call
 *
 * @param CGI name
 * @param argument to be used for CGI called function 
 * @return returned value by function linked to execute by POST request
 */
int extract_cgi_parameters(uint8_t *params, tCGIparams *paramPairs) {

	uint8_t *pair;
	uint8_t *equals;
	int loop;

	/* If we have no parameters at all, return immediately. */
	if (!params || (params[0] == '\0')) {
		return (0);
	}

	/* Get a pointer to our first parameter */
	pair = params;

	/* Parse up to LWIP_HTTPD_MAX_CGI_PARAMETERS from the passed string and ignore the
	 * remainder (if any) */
	for (loop = 0; (loop < CGI_MAX_PARAMS) && pair; loop++) {

		/* Save the name of the parameter */
		paramPairs->params[loop] = pair;

		/* Remember the start of this name=value pair */
		equals = pair;

		/* Find the start of the next name=value pair and replace the delimiter
		 * with a 0 to terminate the previous pair string. */
		pair = (uint8_t *)strchr((const char *)pair, '&');
		if (pair) {
			*pair = '\0';
			pair++;
		} else {
			/* We didn't find a new parameter so find the end of the URI and
			 * replace the space with a '\0' */
			pair = (uint8_t *)strchr((const char *)equals, ' ');
			if (pair) {
				*pair = '\0';
			}

			/* Revert to NULL so that we exit the loop as expected. */
			pair = NULL;
		}

		/* Now find the '=' in the previous pair, replace it with '\0' and save
		 * the parameter value string. */
		equals = (uint8_t *)strchr((const char *)equals, '=');
		if (equals) {
			*equals = '\0';
			paramPairs->param_vals[loop] = equals + 1;;
		} else {
			paramPairs->param_vals[loop] = NULL;
		}
	}
	return loop;
}

int getCgiParams (uint8_t *name, struct pbuf *req){

	uint8_t i;
	uint8_t *string;
	tCGIparams *cgiPointer;

	for(i=0;i<CGI_MAX_COMMANDS;i++)
	{
	if(!strncmp((const char *)name,(const char *)CGI_CMD_ARRAY[i].command,strlen(CGI_CMD_ARRAY[i].command)))//contains array
		{
			if((string = search_string_linked_list(req, serviceBuf, (uint8_t *)STRING_DOUBLE_END, 0)) != NULL){
				printf ("find first CGI parameter %s, try to extract \r\n", string);
				cgiPointer = &CGI_params;
				extract_cgi_parameters(string, cgiPointer);
				return CGI_CMD_ARRAY[i].func((char *)name,cgiPointer);
			} else puts("CRLFCRLF not found\r\n\r\n");
		}
	}
	//CGI: no replacement was found!!!
	return 0;
}

uint8_t
CGI_parser(int8_t *name, void *request)
{
     uint8_t i;

     for(i=0;i<CGI_MAX_COMMANDS;i++)
     {
        if(!strncmp((const char *)name,(const char *)CGI_CMD_ARRAY[i].command,strlen(CGI_CMD_ARRAY[i].command)))//contains array
        {
            return CGI_CMD_ARRAY[i].func((char *)name,request);//execute function if matches
        }
     }
     //CGI: no replacement was found!!!
     return 0;
}

//***************************************************************************//

int
reset_configuration_process(char *name, void *request)
{
    /*stop watchdog, let's see what happens*/
    (void)request;

    /*file for POST request*/
    strcpy(name,"X");
    reset_var = true;
    
    return 1;
}

int replacePlusInPost (uint8_t* input) {
	int i = 0;
	while (*input !='\0'){
		if (*input == '+'){
			*input = 0x20;
			i++;
		}
		input++;
	}
	return i;
}

int searchParamValueByName (const char *name){
	int i = 0;
	for (i = 0; i< CGI_MAX_PARAMS; i++){
		if ((strcmp(name, (const char *)CGI_params.params[i]) == 0) && (*CGI_params.param_vals[i] != '\0')){
			return i;
		}
	}
	return -1;
}

int device_configuration_process(char *name, tCGIparams *inputVal)
{        
      eepromSettings.dhcpEnable = (uint8_t)atoi((const char*)inputVal->param_vals[0]);
      ip_convert_address((char*)inputVal->param_vals[1], (char*)eepromSettings.ipAddress);
      ip_convert_address((char*)inputVal->param_vals[2], (char*)eepromSettings.subnetMask);
      ip_convert_address((char*)inputVal->param_vals[3], (char*)eepromSettings.gateway);
      strcpy((char*) eepromSettings.deviceAlias, (const char*)inputVal->param_vals[4]);
      replacePlusInPost(eepromSettings.deviceAlias);
      strcpy((char*) eepromSettings.userName, (const char*)inputVal->param_vals[5]);
      strcpy((char*) eepromSettings.password, (const char*)inputVal->param_vals[6]);
      eepromSettings.webInterfaceEn = (uint8_t)atoi((const char*)inputVal->param_vals[7]);

      eepromWrite((uint8_t*)&eepromSettings, sizeof (defaultSettings));

      strcpy((char*)name, (const char*) "index.shtml");
    
    return 1;
}

int port1_configuration_process(char *name, tCGIparams *inputVal)
{
	int i = 0;

	eepromSettings.portSettings[0].portMode = (portModeType)atoi((const char*)inputVal->param_vals[0]);
	eepromSettings.portSettings[0].baudRate = (uint32_t)atoi((const char*)inputVal->param_vals[1]);
	eepromSettings.portSettings[0].termEnable = (uint8_t)atoi((const char*)inputVal->param_vals[2]);
	eepromSettings.portSettings[0].networkMode = (networkModeType)atoi((const char*)inputVal->param_vals[3]);
	i = searchParamValueByName("locTcpPort");
	if (i >= 0){
		eepromSettings.portSettings[0].localTcpPort = (uint16_t)atoi((const char*)inputVal->param_vals[i]);
	}
	i = searchParamValueByName("remTcpIp");
	if (i >= 0){
		ip_convert_address((char*)inputVal->param_vals[i], (char*)eepromSettings.portSettings[0].remoteIpAddress);
	}
	i = searchParamValueByName("remTcpPort");
	if (i >= 0){
		eepromSettings.portSettings[0].remoteTcpPort = (uint16_t)atoi((const char*)inputVal->param_vals[i]);
	}
	i = searchParamValueByName("nagle");
	if (i >= 0){
		eepromSettings.portSettings[0].nagle = (uint8_t)atoi((const char*)inputVal->param_vals[i]);
	}
	i = searchParamValueByName("locUdpPort");
	if (i >= 0){
		eepromSettings.portSettings[0].localUdpPort = (uint16_t)atoi((const char*)inputVal->param_vals[i]);
	}
	i = searchParamValueByName("locUdpPort");
	if (i >= 0){
		eepromSettings.portSettings[0].localUdpPort = (uint16_t)atoi((const char*)inputVal->param_vals[i]);
	}
	i = searchParamValueByName("remUdpPort");
	if (i >= 0){
		eepromSettings.portSettings[0].remoteUdpPort = (uint16_t)atoi((const char*)inputVal->param_vals[i]);
	}
	i = searchParamValueByName("portAlias");
	if (i >= 0){
		strcpy((char*) eepromSettings.portSettings[0].portAlias, (const char*)inputVal->param_vals[i]);
		replacePlusInPost(eepromSettings.portSettings[0].portAlias);
	}
	eepromWrite((uint8_t*)&eepromSettings, sizeof (defaultSettings));

	strcpy((char*)name, (const char*) "port1.shtml");
    return 1;
}

int port2_configuration_process(char *name, tCGIparams *inputVal)
{
	int i = 0;

	eepromSettings.portSettings[1].portMode = (portModeType)atoi((const char*)inputVal->param_vals[0]);
	eepromSettings.portSettings[1].baudRate = (uint32_t)atoi((const char*)inputVal->param_vals[1]);
	eepromSettings.portSettings[1].termEnable = (uint8_t)atoi((const char*)inputVal->param_vals[2]);
	eepromSettings.portSettings[1].networkMode = (networkModeType)atoi((const char*)inputVal->param_vals[3]);
	i = searchParamValueByName("locTcpPort");
	if (i >= 0){
		eepromSettings.portSettings[1].localTcpPort = (uint16_t)atoi((const char*)inputVal->param_vals[i]);
	}
	i = searchParamValueByName("remTcpIp");
	if (i >= 0){
		ip_convert_address((char*)inputVal->param_vals[i], (char*)eepromSettings.portSettings[1].remoteIpAddress);
	}
	i = searchParamValueByName("remTcpPort");
	if (i >= 0){
		eepromSettings.portSettings[1].remoteTcpPort = (uint16_t)atoi((const char*)inputVal->param_vals[i]);
	}
	i = searchParamValueByName("nagle");
	if (i >= 0){
		eepromSettings.portSettings[1].nagle = (uint8_t)atoi((const char*)inputVal->param_vals[i]);
	}
	i = searchParamValueByName("locUdpPort");
	if (i >= 0){
		eepromSettings.portSettings[1].localUdpPort = (uint16_t)atoi((const char*)inputVal->param_vals[i]);
	}
	i = searchParamValueByName("locUdpPort");
	if (i >= 0){
		eepromSettings.portSettings[1].localUdpPort = (uint16_t)atoi((const char*)inputVal->param_vals[i]);
	}
	i = searchParamValueByName("remUdpPort");
	if (i >= 0){
		eepromSettings.portSettings[1].remoteUdpPort = (uint16_t)atoi((const char*)inputVal->param_vals[i]);
	}
	i = searchParamValueByName("portAlias");
	if (i >= 0){
		strcpy((char*) eepromSettings.portSettings[1].portAlias, (const char*)inputVal->param_vals[i]);
		replacePlusInPost(eepromSettings.portSettings[1].portAlias);
	}

	eepromWrite((uint8_t*)&eepromSettings, sizeof (defaultSettings));

	strcpy((char*)name, (const char*) "port2.shtml");
    return 1;
}

int login_process(char *name, tCGIparams *inputVal) {

	if ((strcmp((const char*) eepromSettings.userName,
			(const char*) inputVal->param_vals[0]) == 0)
			&& (strcmp((const char*) eepromSettings.password,
					(const char*) inputVal->param_vals[1]) == 0)) {

		isAuthorized = 1;
	} else isAuthorized = 0;

	if (isAuthorized) {
		strcpy((char*)name, (const char*) "index.shtml");
	} else {
		strcpy((char*)name, (const char*) "login.html");
	}
	return 1;
}

int restart_process (char *name, tCGIparams *inputVal){
	NVIC_SystemReset();
	return 1;
}

int load_defaults_process (char *name, tCGIparams *inputVal){
	deviceInitDefaults();
	NVIC_SystemReset();
}

