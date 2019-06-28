/*
###############################################################################
# Copyright (c) 2017, PulseRain Technology LLC 
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License (LGPL) as 
# published by the Free Software Foundation, either version 3 of the License,
# or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but 
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
# or FITNESS FOR A PARTICULAR PURPOSE.  
# See the GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
###############################################################################
*/
#ifndef M10ESP8266_H
#define M10ESP8266_H

#include "Arduino.h"

#define ESP8266_COMMAND_RESPONSE_TIMEOUT 10000


typedef enum esp8266_command_type {
  ESP8266_CMD_QUERY,
  ESP8266_CMD_SETUP,
  ESP8266_CMD_EXECUTE
};


typedef struct {
   int16_t (*resetESP8266)();
   uint16_t (*commandResponse) (uint8_t* cmd, enum esp8266_command_type type, uint8_t* params) __reentrant;
   void (*sendCommand)(uint8_t* cmd, enum esp8266_command_type type, uint8_t* params) __reentrant;
   uint16_t (*readForResponse)(uint8_t* rsp, uint16_t timeout) __reentrant;
   uint16_t (*setMux)(uint8_t mux);
   int16_t (*configureTCPServer)(uint8_t* params);
   void (*clearBuffer)();
} ESP8266_STRUCT;

extern const ESP8266_STRUCT ESP8266;


#define ESP8266_RX_BUFFER_LEN 256 // Number of bytes in the serial receive buffer
extern uint8_t esp8266RxBuffer[ESP8266_RX_BUFFER_LEN];
extern uint8_t esp8266bufferHead;

#define ESP8266_TEST ""
#define RESPONSE_OK "\n\rKO"
#define ESP8266_VERSION "+GMR"
#define ESP8266_UART_CUR "+UART_CUR"
#define ESP8266_UART_DEF "+UART_DEF"
#define ESP8266_DISCONNECT_AP "+CWQAP"
#define ESP8266_RESET "+RST"
#define ESP8266_WIFI_MODE "+CWMODE"
#define ESP8266_TCP_MULTIPLE "+CIPMUX"
#define ESP8266_SERVER_CONFIG "+CIPSERVER"
#define ESP8266_GET_LOCAL_IP "+CIFSR"
#define ESP8266_DHCP "+CWDHCP_CUR"
#define ESP8266_TCP_STATUS "+CIPSTATUS"
#define ESP8266_AP_CONFIG "+CWSAP_DEF"
#define ESP8266_TCP_SERVER_TIMEOUT "+CIPSTO"


#endif

