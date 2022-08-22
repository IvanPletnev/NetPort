#ifndef WEB_SSI_H
#define WEB_SSI_H

#include "main.h"

#include "lwip.h"
#include "lwip/api.h"
#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include "lwip/apps/fs.h"

#include "http_cgi.h"
#include "stdlib.h"
#include "stddef.h"
#include "main.h"
#include "utilities.h"
#include "stdbool.h"

#define SSI_START         "<!--#echo var=\""
#define SSI_END           "\"-->"

#define MAC_ADDRESS_STRING "%02X.%02X.%02X.%02X.%02X.%02X"
#define IP_ADDRESS_STRING  "%d.%d.%d.%d"

/*
 * The command table entry data structure
 */
typedef const struct
{
    char *  command;                /* SSI string to replace  */
    int    (*func)(void *);         /* actual function to call */
} SSI_CMD;

typedef const struct
{
    char *  language;
    char *  language_code;
} LANGUAGES;

/*inline */uint8_t
max_number_of_languages();

/*FSL: external functions*/
extern int showDhcp(void *var);
extern int showIpAddress (void *var);
extern int showSubMask (void *var);
extern int showGateway (void *var);
extern int showDevAlias (void *var);
extern int showUserName (void *var);
extern int showPassword (void *var);
extern int showWebInterfaceEn(void *var);
extern int showPort1mode(void *var);
extern int showPort1baudrate(void *var);
extern int showPort1Termination(void *var);
extern int showPort1NetMode(void *var);
extern int showPort1LocalTcpPort (void *var);
extern int showPort1remoteIp1(void *var);
extern int showPort1RemoteTcpPort (void *var);
extern int showPort1Nagle(void *var);
extern int showPort1localUdpPort (void *var);
extern int showPort1remoteUdpPort (void *var);
extern int showPort1Alias (void *var);
extern int showPort2mode(void *var);
extern int showPort2baudrate(void *var);
extern int showPort2Termination(void *var);
extern int showPort2NetMode(void *var);
extern int showPort2LocalTcpPort (void *var);
extern int showPort2remoteIp1(void *var);
extern int showPort2RemoteTcpPort (void *var);
extern int showPort2Nagle(void *var);
extern int showPort2localUdpPort (void *var);
extern int showPort2remoteUdpPort (void *var);
extern int showPort2Alias (void *var);
extern int sendPort1RxCount (void *var);
extern int sendPort2RxCount (void *var);
extern int sendPort1TxCount (void *var);
extern int sendPort2TxCount (void *var);

/*
 * Macros for User InterFace command table entries
 */
#ifndef SSI_CMD_DHCP
#define SSI_CMD_DHCP     \
    {"dhcp",showDhcp}
#endif

#ifndef SSI_CMD_IP_ADDRESS
#define SSI_CMD_IP_ADDRESS    \
    {"ipAddr", showIpAddress}
#endif

#ifndef SSI_CMD_SUB_MASK
#define SSI_CMD_SUB_MASK    \
    {"subMask",showSubMask}
#endif

#ifndef SSI_CMD_GATEWAY
#define SSI_CMD_GATEWAY    \
    {"gateway",showGateway}
#endif

#ifndef SSI_CMD_DEV_ALIAS
#define SSI_CMD_DEV_ALIAS    \
    {"devAlias",showDevAlias}
#endif

#ifndef SSI_CMD_USER
#define SSI_CMD_USER     \
    {"user",showUserName}
#endif

#ifndef SSI_CMD_PASSWORD
#define SSI_CMD_PASSWORD     \
    {"password",showPassword}
#endif

#ifndef SSI_CMD_WEB_IF
#define SSI_CMD_WEB_IF     \
    {"webIf",showWebInterfaceEn}
#endif

#ifndef SSI_CMD_PORT1_MODE
#define SSI_CMD_PORT1_MODE     \
    {"p1mode",showPort1mode}
#endif

#ifndef SSI_CMD_PORT1BAUD
#define SSI_CMD_PORT1BAUD    \
    {"p1baud",showPort1baudrate}
#endif
  
#ifndef SSI_CMD_PORT1_TERM
#define SSI_CMD_PORT1_TERM     \
    {"p1terminator",showPort1Termination}
#endif  

#ifndef SSI_CMD_PORT1_NET_MODE
#define SSI_CMD_PORT1_NET_MODE     \
    {"p1netMode",showPort1NetMode}
#endif

#ifndef SSI_CMD_PORT1_LOC_TCP_PORT
#define SSI_CMD_PORT1_LOC_TCP_PORT     \
    {"p1locTcpPort",showPort1LocalTcpPort}
#endif

#ifndef SSI_CMD_PORT1_REM_IP_1
#define SSI_CMD_PORT1_REM_IP_1     \
    {"p1remoteIp1",showPort1remoteIp1}
#endif

#ifndef SSI_CMD_PORT1_REM_TCP_PORT
#define SSI_CMD_PORT1_REM_TCP_PORT     \
    {"p1remTcpPort",showPort1RemoteTcpPort}
#endif

#ifndef SSI_CMD_PORT1_NAGLE
#define SSI_CMD_PORT1_NAGLE     \
    {"p1nagle",showPort1Nagle}
#endif

#ifndef SSI_CMD_PORT1_LOC_UDP_PORT
#define SSI_CMD_PORT1_LOC_UDP_PORT     \
    {"p1locUdpPort",showPort1localUdpPort}
#endif

#ifndef SSI_CMD_PORT1_REM_UDP_PORT
#define SSI_CMD_PORT1_REM_UDP_PORT     \
    {"p1remUdpPort",showPort1remoteUdpPort}
#endif

#ifndef SSI_CMD_PORT1_ALIAS
#define SSI_CMD_PORT1_ALIAS     \
    {"p1Alias",showPort1Alias}
#endif

#ifndef SSI_CMD_PORT2_MODE
#define SSI_CMD_PORT2_MODE     \
    {"p2mode",showPort2mode}
#endif

#ifndef SSI_CMD_PORT2BAUD
#define SSI_CMD_PORT2BAUD    \
    {"p2baud",showPort2baudrate}
#endif

#ifndef SSI_CMD_PORT2_TERM
#define SSI_CMD_PORT2_TERM     \
    {"p2terminator",showPort2Termination}
#endif

#ifndef SSI_CMD_PORT2_NET_MODE
#define SSI_CMD_PORT2_NET_MODE     \
    {"p2netMode",showPort2NetMode}
#endif

#ifndef SSI_CMD_PORT2_LOC_TCP_PORT
#define SSI_CMD_PORT2_LOC_TCP_PORT     \
    {"p2locTcpPort",showPort2LocalTcpPort}
#endif

#ifndef SSI_CMD_PORT2_REM_IP_1
#define SSI_CMD_PORT2_REM_IP_1     \
    {"p2remoteIp1",showPort2remoteIp1}
#endif

#ifndef SSI_CMD_PORT2_REM_TCP_PORT
#define SSI_CMD_PORT2_REM_TCP_PORT     \
    {"p2remTcpPort",showPort2RemoteTcpPort}
#endif

#ifndef SSI_CMD_PORT2_NAGLE
#define SSI_CMD_PORT2_NAGLE     \
    {"p2nagle",showPort2Nagle}
#endif

#ifndef SSI_CMD_PORT2_LOC_UDP_PORT
#define SSI_CMD_PORT2_LOC_UDP_PORT     \
    {"p2locUdpPort",showPort2localUdpPort}
#endif

#ifndef SSI_CMD_PORT2_REM_UDP_PORT
#define SSI_CMD_PORT2_REM_UDP_PORT     \
    {"p2remUdpPort",showPort2remoteUdpPort}
#endif

#ifndef SSI_CMD_PORT2_ALIAS
#define SSI_CMD_PORT2_ALIAS     \
    {"p2Alias",showPort2Alias}
#endif

#ifndef SSI_CMD_PORT1_RX_COUNT
#define SSI_CMD_PORT1_RX_COUNT     \
    {"port1rx",sendPort1RxCount}
#endif

#ifndef SSI_CMD_PORT2_RX_COUNT
#define SSI_CMD_PORT2_RX_COUNT     \
    {"port2rx",sendPort2RxCount}
#endif

#ifndef SSI_CMD_PORT1_TX_COUNT
#define SSI_CMD_PORT1_TX_COUNT     \
    {"port1tx",sendPort1TxCount}
#endif

#ifndef SSI_CMD_PORT2_TX_COUNT
#define SSI_CMD_PORT2_TX_COUNT     \
    {"port2tx",sendPort2TxCount}
#endif




#define SSI_MAX_COMMANDS       		sizeof(SSI_CMD_ARRAY)/sizeof(SSI_CMD )

//languages
#define GREETING_ENGLISH        "Hello!!"
#define GREETING_FRENCH    		  "Salut!!"
#define GREETING_DEUTCH         "Hallo!!"
#define GREETING_CHINESSE       "China"
#define GREETING_ITALIAN        "Chau!!"
#define GREETING_RUSSIAN        "Russia"
#define GREETING_JAPANESE       "Japan"
#define GREETING_ARABIC         "Arabic"
#define GREETING_TAIWAN         "Taiwan"
#define GREETING_PORTUGAL       "Portugal"
#define GREETING_POLISH         "Polish"
#define GREETING_SPANISH        "Hola!!"
#define GREETING_KOREAN         "Korean"

//language codes
#define GREETING_ENGLISH_CODE   "en"
#define GREETING_FRENCH_CODE    "fr"
#define GREETING_DEUTCH_CODE    "de"
#define GREETING_CHINESSE_CODE  "zh"
#define GREETING_ITALIAN_CODE   "it"
#define GREETING_RUSSIAN_CODE   "ru"
#define GREETING_JAPANESE_CODE  "ja"
#define GREETING_ARABIC_CODE    "ar"
#define GREETING_TAIWAN_CODE    "tw"
#define GREETING_PORTUGAL_CODE  "pt"
#define GREETING_POLISH_CODE    "pl"
#define GREETING_SPANISH_CODE   "es"
#define GREETING_KOREAN_CODE    "ko"

#define GREETING_LANGUAGE_CODE_LENGTH  sizeof(GREETING_ENGLISH_CODE)-1

//language structs
#ifndef ENGLISH_OPTION
#define ENGLISH_OPTION    \
    {GREETING_ENGLISH,GREETING_ENGLISH_CODE}
#endif
#ifndef FRENCH_OPTION
#define FRENCH_OPTION    \
    {GREETING_FRENCH,GREETING_FRENCH_CODE}
#endif
#ifndef DEUTCH_OPTION
#define DEUTCH_OPTION    \
    {GREETING_DEUTCH,GREETING_DEUTCH_CODE}
#endif
#ifndef CHINESSE_OPTION
#define CHINESSE_OPTION    \
    {GREETING_CHINESSE,GREETING_CHINESSE_CODE}
#endif
#ifndef ITALIAN_OPTION
#define ITALIAN_OPTION    \
    {GREETING_ITALIAN,GREETING_ITALIAN_CODE}
#endif
#ifndef RUSSIAN_OPTION
#define RUSSIAN_OPTION    \
    {GREETING_RUSSIAN,GREETING_RUSSIAN_CODE}
#endif
#ifndef JAPANESE_OPTION
#define JAPANESE_OPTION    \
    {GREETING_JAPANESE,GREETING_JAPANESE_CODE}
#endif
#ifndef ARABIC_OPTION
#define ARABIC_OPTION    \
    {GREETING_ARABIC,GREETING_ARABIC_CODE}
#endif
#ifndef TAIWAN_OPTION
#define TAIWAN_OPTION    \
    {GREETING_TAIWAN,GREETING_TAIWAN_CODE}
#endif
#ifndef PORTUGAL_OPTION
#define PORTUGAL_OPTION    \
    {GREETING_PORTUGAL,GREETING_PORTUGAL_CODE}
#endif
#ifndef POLISH_OPTION
#define POLISH_OPTION    \
    {GREETING_POLISH,GREETING_POLISH_CODE}
#endif
#ifndef SPANISH_OPTION
#define SPANISH_OPTION    \
    {GREETING_SPANISH,GREETING_SPANISH_CODE}
#endif
#ifndef KOREAN_OPTION
#define KOREAN_OPTION    \
    {GREETING_KOREAN,GREETING_KOREAN_CODE}
#endif

#ifndef MAX_LANGUAGES
  #define MAX_LANGUAGES        sizeof(WEB_LANGUAGES)/sizeof(LANGUAGES)
#endif

/*FSL:prototypes*/
/**
 * Implements a SSI Replacement
 *
 * @param array to replace
 * @param replaced array 
 * @return returned value by function linked to array to replace
 */
uint8_t
SSI_parser(uint8_t *input, uint8_t *output);

#endif
