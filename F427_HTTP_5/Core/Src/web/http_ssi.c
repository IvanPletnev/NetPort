#include "http_ssi.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "http_server.h"
#include "converter.h"

#include "utilities.h"
#include "mwerks.h"

/*shared variable*/
uint8_t default_language;

extern deviceSettingsType eepromSettings;

SSI_CMD SSI_CMD_ARRAY[]  = 
{
  SSI_CMD_IP_ADDRESS,
  SSI_CMD_SUB_MASK,
  SSI_CMD_GATEWAY,
  SSI_CMD_DEV_ALIAS,
  SSI_CMD_USER,
  SSI_CMD_PASSWORD,
  SSI_CMD_DHCP,     /*for default dynamic web page*/
  SSI_CMD_WEB_IF,
  SSI_CMD_PORT1_MODE,     /*baudrate reader*/
  SSI_CMD_PORT1BAUD,
  SSI_CMD_PORT1_TERM,
  SSI_CMD_PORT1_NET_MODE,
  SSI_CMD_PORT1_LOC_TCP_PORT,
  SSI_CMD_PORT1_REM_IP_1,
  SSI_CMD_PORT1_REM_TCP_PORT,
  SSI_CMD_PORT1_NAGLE,
  SSI_CMD_PORT1_LOC_UDP_PORT,
  SSI_CMD_PORT1_REM_UDP_PORT,
  SSI_CMD_PORT1_ALIAS,
  SSI_CMD_PORT2_MODE,     /*baudrate reader*/
  SSI_CMD_PORT2BAUD,
  SSI_CMD_PORT2_TERM,
  SSI_CMD_PORT2_NET_MODE,
  SSI_CMD_PORT2_LOC_TCP_PORT,
  SSI_CMD_PORT2_REM_IP_1,
  SSI_CMD_PORT2_REM_TCP_PORT,
  SSI_CMD_PORT2_NAGLE,
  SSI_CMD_PORT2_LOC_UDP_PORT,
  SSI_CMD_PORT2_REM_UDP_PORT,
  SSI_CMD_PORT2_ALIAS,
  SSI_CMD_PORT1_RX_COUNT,
  SSI_CMD_PORT2_RX_COUNT,
  SSI_CMD_PORT1_TX_COUNT,
  SSI_CMD_PORT2_TX_COUNT
};

LANGUAGES WEB_LANGUAGES[] =
{
  ENGLISH_OPTION,
  FRENCH_OPTION,  
  DEUTCH_OPTION,  
  CHINESSE_OPTION,
  ITALIAN_OPTION, 
  RUSSIAN_OPTION, 
  JAPANESE_OPTION,
  ARABIC_OPTION,  
  TAIWAN_OPTION,  
  PORTUGAL_OPTION,
  POLISH_OPTION,  
  SPANISH_OPTION, 
  KOREAN_OPTION   
};

/*FSL:sprintf prototype*/


/**
 * Implements a SSI Replacement
 *
 * @param array to replace
 * @param replaced array 
 * @return returned value by function linked to array to replace
 */
uint8_t
SSI_parser(uint8_t *input, uint8_t *output)
{
     uint8_t i;

     for(i=0;i<SSI_MAX_COMMANDS;i++)
     {
        if(!strncmp((const char *)input,(const char *)SSI_CMD_ARRAY[i].command,strlen(SSI_CMD_ARRAY[i].command)))//contains array
        {
            return SSI_CMD_ARRAY[i].func(output);//execute function if matches
        }
     }
     //SSI: no replacement was found!!!
     return 0;
}

//***************************************************************************//

/*inline */uint8_t
max_number_of_languages()
{
  return MAX_LANGUAGES;
}


int showIpAddress (void *var){
	char ipBuffer [16];
	strcpy((char*)var, "<input type=\"text\" name=\"ipAddress\" value=\"");
	sprintf (ipBuffer, IP_ADDRESS_STRING, eepromSettings.ipAddress[0],eepromSettings.ipAddress[1],eepromSettings.ipAddress[2],eepromSettings.ipAddress[3] );
	strcat((char*)var, (const char*)ipBuffer);
	strcat((char*)var, "\">");
	return 1;
}

int showSubMask (void *var){
	char ipBuffer [16];
	strcpy((char*)var, "<input type=\"text\" name=\"mask\" value=\"");
	sprintf (ipBuffer, IP_ADDRESS_STRING, eepromSettings.subnetMask[0],eepromSettings.subnetMask[1],eepromSettings.subnetMask[2],eepromSettings.subnetMask[3] );
	strcat((char*)var, (const char*)ipBuffer);
	strcat((char*)var, "\">");
	return 1;
}

int showGateway (void *var){
	char ipBuffer [16];
	strcpy((char*)var, "<input type=\"text\" name=\"gateway\" value=\"");
	sprintf (ipBuffer, IP_ADDRESS_STRING, eepromSettings.gateway[0],eepromSettings.gateway[1],eepromSettings.gateway[2],eepromSettings.gateway[3] );
	strcat((char*)var, (const char*)ipBuffer);
	strcat((char*)var, "\">");
	return 1;
}

int showDevAlias (void *var){
	strcpy((char*)var, "<input type=\"text\" name=\"devAlias\" value=\"");
	strcat((char*)var, (const char*) eepromSettings.deviceAlias);
	strcat((char*)var, "\">");
	return 1;
}

int showUserName (void *var){
	strcpy((char*)var, "<input type=\"text\" name=\"userName\" value=\"");
	strcat((char*)var, (const char*) eepromSettings.userName);
	strcat((char*)var, "\">");
	return 1;
}

int showPassword (void *var){
	strcpy((char*)var, "<p><input type=\"password\" id=\"show1\" name=\"password\" value=\"");
	strcat((char*)var, (const char*) eepromSettings.password);
	strcat((char*)var, "\"><img width=\"20\" height=\"20\" src=\"show_password.png\" id=\"show\" /></p>");
	return 1;
}

int showDhcp(void *var) {

	switch (eepromSettings.dhcpEnable) {
	case 0:
		strcpy((char*) var,
				"<select name=\"dhcp\">\r\n<option disabled>Enable or disable DHCP</option>"
				"\r\n<option selected value=\"0\">DHCP Disable</option>\r\n"
				"<option value=\"1\">DHCP Enable</option>\r\n</select>");
		break;
	case 1:
		strcpy((char*) var,
				"<select name=\"dhcp\">\r\n<option disabled>Enable or disable DHCP</option>"
				"\r\n<option value=\"0\">DHCP Disable</option>\r\n"
				"<option selected value=\"1\">DHCP Enable</option>\r\n</select>");
		break;
	default:
		return 0;
	}
	return 1;
}

int showWebInterfaceEn(void *var) {

	switch (eepromSettings.webInterfaceEn) {
	case 0:
		strcpy((char*) var,
				"<select name=\"webInterface\">\r\n<option disabled>Enable web interface</option>"
				"\r\n<option selected value=\"0\">Web interface disabled</option>\r\n"
				"<option value=\"1\">Web interface enabled</option>\r\n</select>");
		break;
	case 1:
		strcpy((char*) var,
				"<select name=\"webInterface\">\r\n<option disabled>Enable web interface</option>"
				"\r\n<option value=\"0\">Web interface disabled</option>\r\n"
				"<option selected value=\"1\">Web interface enabled</option>\r\n</select>");
		break;
	default:
		return 0;
	}
	return 1;
}

int showPort1mode(void *var) {

	switch (eepromSettings.portSettings[0].portMode) {
	case 0:
		strcpy((char*) var,
				"<select name=\"portMode\">"
				"<option disabled>Select an interface mode</option>"
				"<option selected value=\"0\">RS-485 2 wire mode</option>"
				"<option value=\"1\">RS-485 4 wire mode</option>"
				"<option value=\"2\">RS-422 4 wire mode</option>"
				"</select>");
		break;
	case 1:
		strcpy((char*) var,
				"<select name=\"portMode\">"
				"<option disabled>Select an interface mode</option>"
				"<option value=\"0\">RS-485 2 wire mode</option>"
				"<option selected value=\"1\">RS-485 4 wire mode</option>"
				"<option value=\"2\">RS-422 4 wire mode</option>"
				"</select>");
		break;
	case 2:
		strcpy((char*) var,
				"<select name=\"portMode\">"
				"<option disabled>Select an interface mode</option>"
				"<option value=\"0\">RS-485 2 wire mode</option>"
				"<option value=\"1\">RS-485 4 wire mode</option>"
				"<option selected value=\"2\">RS-422 4 wire mode</option>"
				"</select>");
		break;
	default:
		return 0;
	}
	return 1;
}

int showPort1baudrate(void *var) {

	switch (eepromSettings.portSettings[0].baudRate) {
	case 1200:
		strcpy((char*) var,
				"<select name=\"baudrate\">"
				"<option disabled>Select a baudrate</option>"
				"<option selected value=\"1200\">1200</option>"
				"<option value=\"2400\">2400</option>"
				"<option value=\"4800\">4800</option>"
				"<option value=\"9600\">9600</option>"
				"<option value=\"14400\">14400</option>"
				"<option value=\"19200\">19200</option>"
				"<option value=\"38400\">38400</option>"
				"<option value=\"57600\">57600</option>"
				"<option value=\"115200\">115200</option>"
				"<option value=\"230400\">230400</option>"
				"<option value=\"460800\">460800</option>"
				"</select>");
		break;
	case 2400:
		strcpy((char*) var,
				"<select name=\"baudrate\">"
				"<option disabled>Select a baudrate</option>"
				"<option value=\"1200\">1200</option>"
				"<option selected value=\"2400\">2400</option>"
				"<option value=\"4800\">4800</option>"
				"<option value=\"9600\">9600</option>"
				"<option value=\"14400\">14400</option>"
				"<option value=\"19200\">19200</option>"
				"<option value=\"38400\">38400</option>"
				"<option value=\"57600\">57600</option>"
				"<option value=\"115200\">115200</option>"
				"<option value=\"230400\">230400</option>"
				"<option value=\"460800\">460800</option>"
				"</select>");
		break;
	case 4800:
		strcpy((char*) var,
				"<select name=\"baudrate\">"
				"<option disabled>Select a baudrate</option>"
				"<option value=\"1200\">1200</option>"
				"<option value=\"2400\">2400</option>"
				"<option selected value=\"4800\">4800</option>"
				"<option value=\"9600\">9600</option>"
				"<option value=\"14400\">14400</option>"
				"<option value=\"19200\">19200</option>"
				"<option value=\"38400\">38400</option>"
				"<option value=\"57600\">57600</option>"
				"<option value=\"115200\">115200</option>"
				"<option value=\"230400\">230400</option>"
				"<option value=\"460800\">460800</option>"
				"</select>");
		break;
	case 9600:
		strcpy((char*) var,
				"<select name=\"baudrate\">"
				"<option disabled>Select a baudrate</option>"
				"<option value=\"1200\">1200</option>"
				"<option value=\"2400\">2400</option>"
				"<option value=\"4800\">4800</option>"
				"<option selected value=\"9600\">9600</option>"
				"<option value=\"14400\">14400</option>"
				"<option value=\"19200\">19200</option>"
				"<option value=\"38400\">38400</option>"
				"<option value=\"57600\">57600</option>"
				"<option value=\"115200\">115200</option>"
				"<option value=\"230400\">230400</option>"
				"<option value=\"460800\">460800</option>"
				"</select>");
		break;
	case 14400:
		strcpy((char*) var,
				"<select name=\"baudrate\">"
				"<option disabled>Select a baudrate</option>"
				"<option value=\"1200\">1200</option>"
				"<option value=\"2400\">2400</option>"
				"<option value=\"4800\">4800</option>"
				"<option value=\"9600\">9600</option>"
				"<option selected value=\"14400\">14400</option>"
				"<option value=\"19200\">19200</option>"
				"<option value=\"38400\">38400</option>"
				"<option value=\"57600\">57600</option>"
				"<option value=\"115200\">115200</option>"
				"<option value=\"230400\">230400</option>"
				"<option value=\"460800\">460800</option>"
				"</select>");
		break;
	case 19200:
		strcpy((char*) var,
				"<select name=\"baudrate\">"
				"<option disabled>Select a baudrate</option>"
				"<option value=\"1200\">1200</option>"
				"<option value=\"2400\">2400</option>"
				"<option value=\"4800\">4800</option>"
				"<option value=\"9600\">9600</option>"
				"<option value=\"14400\">14400</option>"
				"<option selected value=\"19200\">19200</option>"
				"<option value=\"38400\">38400</option>"
				"<option value=\"57600\">57600</option>"
				"<option value=\"115200\">115200</option>"
				"<option value=\"230400\">230400</option>"
				"<option value=\"460800\">460800</option>"
				"</select>");
		break;
	case 38400:
		strcpy((char*) var,
				"<select name=\"baudrate\">"
				"<option disabled>Select a baudrate</option>"
				"<option value=\"1200\">1200</option>"
				"<option value=\"2400\">2400</option>"
				"<option value=\"4800\">4800</option>"
				"<option value=\"9600\">9600</option>"
				"<option value=\"14400\">14400</option>"
				"<option value=\"19200\">19200</option>"
				"<option selected value=\"38400\">38400</option>"
				"<option value=\"57600\">57600</option>"
				"<option value=\"115200\">115200</option>"
				"<option value=\"230400\">230400</option>"
				"<option value=\"460800\">460800</option>"
				"</select>");
		break;
	case 57600:
		strcpy((char*) var,
				"<select name=\"baudrate\">"
				"<option disabled>Select a baudrate</option>"
				"<option value=\"1200\">1200</option>"
				"<option value=\"2400\">2400</option>"
				"<option value=\"4800\">4800</option>"
				"<option value=\"9600\">9600</option>"
				"<option value=\"14400\">14400</option>"
				"<option value=\"19200\">19200</option>"
				"<option value=\"38400\">38400</option>"
				"<option selected value=\"57600\">57600</option>"
				"<option value=\"115200\">115200</option>"
				"<option value=\"230400\">230400</option>"
				"<option value=\"460800\">460800</option>"
				"</select>");
		break;
	case 0:
	case 115200:
		strcpy((char*) var,
				"<select name=\"baudrate\">"
				"<option disabled>Select a baudrate</option>"
				"<option value=\"1200\">1200</option>"
				"<option value=\"2400\">2400</option>"
				"<option value=\"4800\">4800</option>"
				"<option value=\"9600\">9600</option>"
				"<option value=\"14400\">14400</option>"
				"<option value=\"19200\">19200</option>"
				"<option value=\"38400\">38400</option>"
				"<option value=\"57600\">57600</option>"
				"<option selected value=\"115200\">115200</option>"
				"<option value=\"230400\">230400</option>"
				"<option value=\"460800\">460800</option>"
				"</select>");
		break;
	case 230400:
		strcpy((char*) var,
				"<select name=\"baudrate\">"
				"<option disabled>Select a baudrate</option>"
				"<option value=\"1200\">1200</option>"
				"<option value=\"2400\">2400</option>"
				"<option value=\"4800\">4800</option>"
				"<option value=\"9600\">9600</option>"
				"<option value=\"14400\">14400</option>"
				"<option value=\"19200\">19200</option>"
				"<option value=\"38400\">38400</option>"
				"<option value=\"57600\">57600</option>"
				"<option value=\"115200\">115200</option>"
				"<option selected value=\"230400\">230400</option>"
				"<option value=\"460800\">460800</option>"
				"</select>");
		break;
	case 460800:
		strcpy((char*) var,
				"<select name=\"baudrate\">"
				"<option disabled>Select a baudrate</option>"
				"<option value=\"1200\">1200</option>"
				"<option value=\"2400\">2400</option>"
				"<option value=\"4800\">4800</option>"
				"<option value=\"9600\">9600</option>"
				"<option value=\"14400\">14400</option>"
				"<option value=\"19200\">19200</option>"
				"<option value=\"38400\">38400</option>"
				"<option value=\"57600\">57600</option>"
				"<option value=\"115200\">115200</option>"
				"<option value=\"230400\">230400</option>"
				"<option selected value=\"460800\">460800</option>"
				"</select>");
		break;
	default:
		return 0;
	}
	return 1;
}

int showPort1Termination(void *var) {

	switch (eepromSettings.portSettings[0].termEnable) {
	case 0:
		strcpy((char*) var,
				"<select name=\"term\">"
				"<option selected value=\"0\">Disable </option>"
				"<option value=\"1\">Enable</option>"
				"</select>");
		break;
	case 1:
		strcpy((char*) var,
				"<select name=\"term\">"
				"<option value=\"0\">Disable </option>"
				"<option selected value=\"1\">Enable</option>"
				"</select>");
		break;
	default:
		return 0;
	}
	return 1;
}

int showPort1NetMode(void *var) {

	switch (eepromSettings.portSettings[0].networkMode) {
	case 0:
		strcpy((char*) var,
				"<select id=\"selectNetMode\" name=\"netmode\">"
				    "<option disabled>Select a network mode</option>"
				    "<option selected value=\"0\">TCP Server mode</option>"
				    "<option value=\"1\">TCP Client mode</option>"
				    "<option value=\"2\">UDP Mode</option>"
			    "</select>");
		break;
	case 1:
		strcpy((char*) var,
				"<select id=\"selectNetMode\" name=\"netmode\">"
				    "<option disabled>Select a network mode</option>"
				    "<option value=\"0\">TCP Server mode</option>"
				    "<option selected value=\"1\">TCP Client mode</option>"
				    "<option value=\"2\">UDP Mode</option>"
			    "</select>");
		break;
	case 2:
		strcpy((char*) var,
				"<select id=\"selectNetMode\" name=\"netmode\">"
				    "<option disabled>Select a network mode</option>"
				    "<option value=\"0\">TCP Server mode</option>"
				    "<option value=\"1\">TCP Client mode</option>"
				    "<option selected value=\"2\">UDP Mode</option>"
			    "</select>");
		break;
	default:
		return 0;
	}
	return 1;
}

int showPort1LocalTcpPort (void *var){
	char tempBuf [16];
	strcpy((char*) var, "<input type=\"text\" name=\"locTcpPort\" value=\"" );
	strcat ((char *) var, (const char*) itoa(eepromSettings.portSettings[0].localTcpPort, tempBuf, 10));
	strcat ((char*) var, "\">");
	return 1;
}

int showPort1remoteIp1(void *var) {
	char ipBuffer[16];
	strcpy((char*) var, "<input type=\"text\" name=\"remTcpIp\" value=\"");
	sprintf(ipBuffer, IP_ADDRESS_STRING,
			eepromSettings.portSettings[0].remoteIpAddress[0],
			eepromSettings.portSettings[0].remoteIpAddress[1],
			eepromSettings.portSettings[0].remoteIpAddress[2],
			eepromSettings.portSettings[0].remoteIpAddress[3]);
	strcat((char*) var, (const char*) ipBuffer);
	strcat((char*) var, "\">");
	return 1;
}

int showPort1RemoteTcpPort (void *var){
	char tempBuf [16];
	strcpy((char*) var, "<input type=\"text\" name=\"remTcpPort\" value=\"" );
	strcat ((char *) var, (const char*) itoa(eepromSettings.portSettings[0].remoteTcpPort, tempBuf, 10));
	strcat ((char*) var, "\">");
	return 1;
}

int showPort1Nagle(void *var) {

	switch (eepromSettings.portSettings[0].nagle) {
	case 0:
		strcpy((char*) var,
				"<select name=\"nagle\">"
				"<option selected value=\"0\">Disable </option>"
				"<option value=\"1\">Enable</option>"
				"</select>");
		break;
	case 1:
		strcpy((char*) var,
				"<select name=\"nagle\">"
				"<option value=\"0\">Disable </option>"
				"<option selected value=\"1\">Enable</option>"
				"</select>");
		break;
	default:
		return 0;
	}
	return 1;
}

int showPort1localUdpPort (void *var){
	char tempBuf [16];
	strcpy((char*) var, "<input type=\"text\" name=\"locUdpPort\" value=\"" );
	strcat ((char *) var, (const char*) itoa(eepromSettings.portSettings[0].localUdpPort, tempBuf, 10));
	strcat ((char*) var, "\">");
	return 1;
}


int showPort1remoteUdpPort (void *var){
	char tempBuf [16];
	strcpy((char*) var, "<input type=\"text\" name=\"remUdpPort\" value=\"" );
	strcat ((char *) var, (const char*) itoa(eepromSettings.portSettings[0].remoteUdpPort, tempBuf, 10));
	strcat ((char*) var, "\">");
	return 1;
}

int showPort1Alias (void *var){
	strcpy((char*)var, "<input type=\"text\" name=\"portAlias\" value=\"");
	strcat((char*)var, (const char*) eepromSettings.portSettings[0].portAlias);
	strcat((char*)var, "\">");
	return 1;
}

int showPort2mode(void *var) {

	switch (eepromSettings.portSettings[1].portMode) {
	case 0:
		strcpy((char*) var,
				"<select name=\"portMode\">"
				"<option disabled>Select an interface mode</option>"
				"<option selected value=\"0\">RS-485 2 wire mode</option>"
				"<option value=\"1\">RS-485 4 wire mode</option>"
				"<option value=\"2\">RS-422 4 wire mode</option>"
				"</select>");
		break;
	case 1:
		strcpy((char*) var,
				"<select name=\"portMode\">"
				"<option disabled>Select an interface mode</option>"
				"<option value=\"0\">RS-485 2 wire mode</option>"
				"<option selected value=\"1\">RS-485 4 wire mode</option>"
				"<option value=\"2\">RS-422 4 wire mode</option>"
				"</select>");
		break;
	case 2:
		strcpy((char*) var,
				"<select name=\"portMode\">"
				"<option disabled>Select an interface mode</option>"
				"<option value=\"0\">RS-485 2 wire mode</option>"
				"<option value=\"1\">RS-485 4 wire mode</option>"
				"<option selected value=\"2\">RS-422 4 wire mode</option>"
				"</select>");
		break;
	default:
		return 0;
	}
	return 1;
}

int showPort2baudrate(void *var) {

	switch (eepromSettings.portSettings[1].baudRate) {
	case 1200:
		strcpy((char*) var,
				"<select name=\"baudrate\">"
				"<option disabled>Select a baudrate</option>"
				"<option selected value=\"1200\">1200</option>"
				"<option value=\"2400\">2400</option>"
				"<option value=\"4800\">4800</option>"
				"<option value=\"9600\">9600</option>"
				"<option value=\"14400\">14400</option>"
				"<option value=\"19200\">19200</option>"
				"<option value=\"38400\">38400</option>"
				"<option value=\"57600\">57600</option>"
				"<option value=\"115200\">115200</option>"
				"<option value=\"230400\">230400</option>"
				"<option value=\"460800\">460800</option>"
				"</select>");
		break;
	case 2400:
		strcpy((char*) var,
				"<select name=\"baudrate\">"
				"<option disabled>Select a baudrate</option>"
				"<option value=\"1200\">1200</option>"
				"<option selected value=\"2400\">2400</option>"
				"<option value=\"4800\">4800</option>"
				"<option value=\"9600\">9600</option>"
				"<option value=\"14400\">14400</option>"
				"<option value=\"19200\">19200</option>"
				"<option value=\"38400\">38400</option>"
				"<option value=\"57600\">57600</option>"
				"<option value=\"115200\">115200</option>"
				"<option value=\"230400\">230400</option>"
				"<option value=\"460800\">460800</option>"
				"</select>");
		break;
	case 4800:
		strcpy((char*) var,
				"<select name=\"baudrate\">"
				"<option disabled>Select a baudrate</option>"
				"<option value=\"1200\">1200</option>"
				"<option value=\"2400\">2400</option>"
				"<option selected value=\"4800\">4800</option>"
				"<option value=\"9600\">9600</option>"
				"<option value=\"14400\">14400</option>"
				"<option value=\"19200\">19200</option>"
				"<option value=\"38400\">38400</option>"
				"<option value=\"57600\">57600</option>"
				"<option value=\"115200\">115200</option>"
				"<option value=\"230400\">230400</option>"
				"<option value=\"460800\">460800</option>"
				"</select>");
		break;
	case 9600:
		strcpy((char*) var,
				"<select name=\"baudrate\">"
				"<option disabled>Select a baudrate</option>"
				"<option value=\"1200\">1200</option>"
				"<option value=\"2400\">2400</option>"
				"<option value=\"4800\">4800</option>"
				"<option selected value=\"9600\">9600</option>"
				"<option value=\"14400\">14400</option>"
				"<option value=\"19200\">19200</option>"
				"<option value=\"38400\">38400</option>"
				"<option value=\"57600\">57600</option>"
				"<option value=\"115200\">115200</option>"
				"<option value=\"230400\">230400</option>"
				"<option value=\"460800\">460800</option>"
				"</select>");
		break;
	case 14400:
		strcpy((char*) var,
				"<select name=\"baudrate\">"
				"<option disabled>Select a baudrate</option>"
				"<option value=\"1200\">1200</option>"
				"<option value=\"2400\">2400</option>"
				"<option value=\"4800\">4800</option>"
				"<option value=\"9600\">9600</option>"
				"<option selected value=\"14400\">14400</option>"
				"<option value=\"19200\">19200</option>"
				"<option value=\"38400\">38400</option>"
				"<option value=\"57600\">57600</option>"
				"<option value=\"115200\">115200</option>"
				"<option value=\"230400\">230400</option>"
				"<option value=\"460800\">460800</option>"
				"</select>");
		break;
	case 19200:
		strcpy((char*) var,
				"<select name=\"baudrate\">"
				"<option disabled>Select a baudrate</option>"
				"<option value=\"1200\">1200</option>"
				"<option value=\"2400\">2400</option>"
				"<option value=\"4800\">4800</option>"
				"<option value=\"9600\">9600</option>"
				"<option value=\"14400\">14400</option>"
				"<option selected value=\"19200\">19200</option>"
				"<option value=\"38400\">38400</option>"
				"<option value=\"57600\">57600</option>"
				"<option value=\"115200\">115200</option>"
				"<option value=\"230400\">230400</option>"
				"<option value=\"460800\">460800</option>"
				"</select>");
		break;
	case 38400:
		strcpy((char*) var,
				"<select name=\"baudrate\">"
				"<option disabled>Select a baudrate</option>"
				"<option value=\"1200\">1200</option>"
				"<option value=\"2400\">2400</option>"
				"<option value=\"4800\">4800</option>"
				"<option value=\"9600\">9600</option>"
				"<option value=\"14400\">14400</option>"
				"<option value=\"19200\">19200</option>"
				"<option selected value=\"38400\">38400</option>"
				"<option value=\"57600\">57600</option>"
				"<option value=\"115200\">115200</option>"
				"<option value=\"230400\">230400</option>"
				"<option value=\"460800\">460800</option>"
				"</select>");
		break;
	case 57600:
		strcpy((char*) var,
				"<select name=\"baudrate\">"
				"<option disabled>Select a baudrate</option>"
				"<option value=\"1200\">1200</option>"
				"<option value=\"2400\">2400</option>"
				"<option value=\"4800\">4800</option>"
				"<option value=\"9600\">9600</option>"
				"<option value=\"14400\">14400</option>"
				"<option value=\"19200\">19200</option>"
				"<option value=\"38400\">38400</option>"
				"<option selected value=\"57600\">57600</option>"
				"<option value=\"115200\">115200</option>"
				"<option value=\"230400\">230400</option>"
				"<option value=\"460800\">460800</option>"
				"</select>");
		break;
	case 0:
	case 115200:
		strcpy((char*) var,
				"<select name=\"baudrate\">"
				"<option disabled>Select a baudrate</option>"
				"<option value=\"1200\">1200</option>"
				"<option value=\"2400\">2400</option>"
				"<option value=\"4800\">4800</option>"
				"<option value=\"9600\">9600</option>"
				"<option value=\"14400\">14400</option>"
				"<option value=\"19200\">19200</option>"
				"<option value=\"38400\">38400</option>"
				"<option value=\"57600\">57600</option>"
				"<option selected value=\"115200\">115200</option>"
				"<option value=\"230400\">230400</option>"
				"<option value=\"460800\">460800</option>"
				"</select>");
		break;
	case 230400:
		strcpy((char*) var,
				"<select name=\"baudrate\">"
				"<option disabled>Select a baudrate</option>"
				"<option value=\"1200\">1200</option>"
				"<option value=\"2400\">2400</option>"
				"<option value=\"4800\">4800</option>"
				"<option value=\"9600\">9600</option>"
				"<option value=\"14400\">14400</option>"
				"<option value=\"19200\">19200</option>"
				"<option value=\"38400\">38400</option>"
				"<option value=\"57600\">57600</option>"
				"<option value=\"115200\">115200</option>"
				"<option selected value=\"230400\">230400</option>"
				"<option value=\"460800\">460800</option>"
				"</select>");
		break;
	case 460800:
		strcpy((char*) var,
				"<select name=\"baudrate\">"
				"<option disabled>Select a baudrate</option>"
				"<option value=\"1200\">1200</option>"
				"<option value=\"2400\">2400</option>"
				"<option value=\"4800\">4800</option>"
				"<option value=\"9600\">9600</option>"
				"<option value=\"14400\">14400</option>"
				"<option value=\"19200\">19200</option>"
				"<option value=\"38400\">38400</option>"
				"<option value=\"57600\">57600</option>"
				"<option value=\"115200\">115200</option>"
				"<option value=\"230400\">230400</option>"
				"<option selected value=\"460800\">460800</option>"
				"</select>");
		break;
	default:
		return 0;
	}
	return 1;
}

int showPort2Termination(void *var) {

	switch (eepromSettings.portSettings[1].termEnable) {
	case 0:
		strcpy((char*) var,
				"<select name=\"term\">"
				"<option selected value=\"0\">Disable </option>"
				"<option value=\"1\">Enable</option>"
				"</select>");
		break;
	case 1:
		strcpy((char*) var,
				"<select name=\"term\">"
				"<option value=\"0\">Disable </option>"
				"<option selected value=\"1\">Enable</option>"
				"</select>");
		break;
	default:
		return 0;
	}
	return 1;
}

int showPort2NetMode(void *var) {

	switch (eepromSettings.portSettings[1].networkMode) {
	case 0:
		strcpy((char*) var,
				"<select id=\"selectNetMode\" name=\"netmode\">"
				    "<option disabled>Select a network mode</option>"
				    "<option selected value=\"0\">TCP Server mode</option>"
				    "<option value=\"1\">TCP Client mode</option>"
				    "<option value=\"2\">UDP Mode</option>"
			    "</select>");
		break;
	case 1:
		strcpy((char*) var,
				"<select id=\"selectNetMode\" name=\"netmode\">"
				    "<option disabled>Select a network mode</option>"
				    "<option value=\"0\">TCP Server mode</option>"
				    "<option selected value=\"1\">TCP Client mode</option>"
				    "<option value=\"2\">UDP Mode</option>"
			    "</select>");
		break;
	case 2:
		strcpy((char*) var,
				"<select id=\"selectNetMode\" name=\"netmode\">"
				    "<option disabled>Select a network mode</option>"
				    "<option value=\"0\">TCP Server mode</option>"
				    "<option value=\"1\">TCP Client mode</option>"
				    "<option selected value=\"2\">UDP Mode</option>"
			    "</select>");
		break;
	default:
		return 0;
	}
	return 1;
}

int showPort2LocalTcpPort (void *var){
	char tempBuf [16];
	strcpy((char*) var, "<input type=\"text\" name=\"locTcpPort\" value=\"" );
	strcat ((char *) var, (const char*) itoa(eepromSettings.portSettings[1].localTcpPort, tempBuf, 10));
	strcat ((char*) var, "\">");
	return 1;
}

int showPort2remoteIp1(void *var) {
	char ipBuffer[16];
	strcpy((char*) var, "<input type=\"text\" name=\"remTcpIp\" value=\"");
	sprintf(ipBuffer, IP_ADDRESS_STRING,
			eepromSettings.portSettings[1].remoteIpAddress[0],
			eepromSettings.portSettings[1].remoteIpAddress[1],
			eepromSettings.portSettings[1].remoteIpAddress[2],
			eepromSettings.portSettings[1].remoteIpAddress[3]);
	strcat((char*) var, (const char*) ipBuffer);
	strcat((char*) var, "\">");
	return 1;
}

int showPort2RemoteTcpPort (void *var){
	char tempBuf [16];
	strcpy((char*) var, "<input type=\"text\" name=\"remTcpPort\" value=\"" );
	strcat ((char *) var, (const char*) itoa(eepromSettings.portSettings[1].remoteTcpPort, tempBuf, 10));
	strcat ((char*) var, "\">");
	return 1;
}

int showPort2Nagle(void *var) {

	switch (eepromSettings.portSettings[1].nagle) {
	case 0:
		strcpy((char*) var,
				"<select name=\"nagle\">"
				"<option selected value=\"0\">Disable </option>"
				"<option value=\"1\">Enable</option>"
				"</select>");
		break;
	case 1:
		strcpy((char*) var,
				"<select name=\"nagle\">"
				"<option value=\"0\">Disable </option>"
				"<option selected value=\"1\">Enable</option>"
				"</select>");
		break;
	default:
		return 0;
	}
	return 1;
}

int showPort2localUdpPort (void *var){
	char tempBuf [16];
	strcpy((char*) var, "<input type=\"text\" name=\"locUdpPort\" value=\"" );
	strcat ((char *) var, (const char*) itoa(eepromSettings.portSettings[1].localUdpPort, tempBuf, 10));
	strcat ((char*) var, "\">");
	return 1;
}


int showPort2remoteUdpPort (void *var){
	char tempBuf [16];
	strcpy((char*) var, "<input type=\"text\" name=\"remUdpPort\" value=\"" );
	strcat ((char *) var, (const char*) itoa(eepromSettings.portSettings[1].remoteUdpPort, tempBuf, 10));
	strcat ((char*) var, "\">");
	return 1;
}

int showPort2Alias (void *var){
	strcpy((char*)var, "<input type=\"text\" name=\"portAlias\" value=\"");
	strcat((char*)var, (const char*) eepromSettings.portSettings[1].portAlias);
	strcat((char*)var, "\">");
	return 1;
}

int sendPort1RxCount (void *var){
	sprintf((char *)var, "%lu", port1UartReceived);
	return 1;
}

int sendPort2RxCount (void *var){
	sprintf((char *)var, "%lu", port2UartReceived);
	return 1;
}

int sendPort1TxCount (void *var){
	sprintf((char *)var, "%lu", port1UartTransmitted);
	return 1;
}

int sendPort2TxCount (void *var){
	sprintf((char *)var, "%lu", port2UartTransmitted);
	return 1;
}


