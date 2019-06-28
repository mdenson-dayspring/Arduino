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

#include "M10ESP8266.h"
#include "M10SerialAUX.h"


uint8_t esp8266RxBuffer[ESP8266_RX_BUFFER_LEN];
uint8_t esp8266bufferHead; // Holds position of latest byte placed in buffer.


//----------------------------------------------------------------------------
// clear()
//
// Parameters:
//      None
//
// Return Value:
//      None
//
// Remarks:
//      function to fill zero to esp8266RxBuffer
//----------------------------------------------------------------------------

static void clearBuffer()
{
  uint8_t i;

  esp8266RxBuffer[ESP8266_RX_BUFFER_LEN - 1] = 0;
  for (i = 0; i < (ESP8266_RX_BUFFER_LEN - 1); ++i) {
  
    esp8266RxBuffer[i] = 0;
  } // End of for loop
  esp8266bufferHead = 0;
} // End of clearBuffer()




//----------------------------------------------------------------------------
// bufferStrMatch()
//
// Parameters:
//      pStr : reversed string to match
//
// Return Value:
//      1 : got match
//      0 : mismatch
//
// Remarks:
//      function to match string against the end of esp8266RxBuffer
//----------------------------------------------------------------------------
static int8_t bufferStrMatch (uint8_t* pStr)
{
  uint8_t t, i;

  i = esp8266bufferHead - 1;
  t = *pStr;
  
  while(t) {
    if (t != esp8266RxBuffer[i]) {
      return 0;
    }
    
    ++pStr;
    --i;
    t = *pStr;
  } // End of while loop
  
  return 1;
} // End of bufferStrMatch()


//----------------------------------------------------------------------------
// readForResponse()
//
// Parameters:
//      rsp     : expected string response (reversed)
//      timeout : timeout in number of loops
//
// Return Value:
//      number of matched characters
//
// Remarks:
//      function to put received data into esp8266RxBuffer. It exits when
//  a response string is matched, or quits after timeout.
//----------------------------------------------------------------------------
static uint16_t readForResponse(uint8_t* rsp, uint16_t timeout) __reentrant
{
    uint16_t received = 0; // received keeps track of number of chars read
    uint8_t c;
    uint16_t time_count = 0;
  
    while (time_count < timeout) {
        if (SerialAUX.available()) { // If data is available on UART RX
            c = SerialAUX.read();

            esp8266RxBuffer[esp8266bufferHead++] = c;
            ++received;
  
            if (bufferStrMatch(rsp)) {
                return received;
            }

            time_count = 0;
    
        } else {
           ++time_count;
        }
    } // End of while loop

    return received;
    
} // End of readForResponse()



//----------------------------------------------------------------------------
// sendCommand()
//
// Parameters:
//      cmd     : command string
//      type    : ESP8266_CMD_QUERY / ESP8266_CMD_SETUP / ESP8266_CMD_EXECUTE
//      params  : parameter string if type is ESP8266_CMD_SETUP
//
// Return Value:
//      number of matched characters
//
// Remarks:
//      function to send AT command to ESP8266
//----------------------------------------------------------------------------
static void sendCommand(uint8_t* cmd, enum esp8266_command_type type, uint8_t* params) __reentrant
{
    clearBuffer();
    SerialAUX.write("AT"); // Serial.write("AT"); 
    SerialAUX.write(cmd);  //Serial.write(cmd);
    if (type == ESP8266_CMD_QUERY) {
        SerialAUX.write("?"); //Serial.write("?");
    } else if (type == ESP8266_CMD_SETUP) {
        SerialAUX.write("="); //Serial.write("=");
        SerialAUX.write(params); //Serial.write(params);   
    }
    SerialAUX.write("\r\n"); //Serial.write("\r\n");
} // End of sendCommand()


//----------------------------------------------------------------------------
// commandResponse()
//
// Parameters:
//      cmd     : command string
//      type    : ESP8266_CMD_QUERY / ESP8266_CMD_SETUP / ESP8266_CMD_EXECUTE
//      params  : parameter string if type is ESP8266_CMD_SETUP
//
// Return Value:
//      number of matched characters
//
// Remarks:
//      function to send AT command to ESP8266 and get response
//----------------------------------------------------------------------------

static uint16_t commandResponse (uint8_t* cmd, enum esp8266_command_type type, uint8_t* params) __reentrant
{
    sendCommand (cmd, type, params);
    return readForResponse(RESPONSE_OK, ESP8266_COMMAND_RESPONSE_TIMEOUT); 
} // commandResponse()


//----------------------------------------------------------------------------
// setMux()
//
// Parameters:
//      mux     : 1  - enable multiple connections
//                0  - disable multiple connections
//
// Return Value:
//      zero    : command failed
//    None zero : command succeeded
//
// Remarks:
//      function to enable / disable multiple connections.
//----------------------------------------------------------------------------
uint16_t setMux(uint8_t mux)
{
  char params[2] = {0, 0};
  params[0] = (mux > 0) ? '1' : '0';
  return commandResponse(ESP8266_TCP_MULTIPLE, ESP8266_CMD_SETUP, params);
} // End of setMux()

//----------------------------------------------------------------------------
// configureTCPServer()
//
// Parameters:
//      params   : "mode, port"
//                 mode - 1 create server, 0 delete server
//                 port - TCP port
//
// Return Value:
//      zero    : command failed
//    None zero : command succeeded
//
// Remarks:
//      function to create / delete server.
//----------------------------------------------------------------------------

int16_t configureTCPServer(uint8_t* params)
{
    return commandResponse(ESP8266_SERVER_CONFIG, ESP8266_CMD_SETUP, params);
} // End of configureTCPServer()

//----------------------------------------------------------------------------
// resetESP8266()
//
// Parameters:
//      None
//
// Return Value:
//      zero    : command failed
//    None zero : command succeeded
//
// Remarks:
//      function to create / delete server.
//----------------------------------------------------------------------------

int16_t resetESP8266()
{
    return commandResponse (ESP8266_RESET, ESP8266_CMD_EXECUTE, 0);
} // End of resetESP8266()



const ESP8266_STRUCT ESP8266 = {
    .resetESP8266 = resetESP8266,
    .commandResponse = commandResponse,
    .sendCommand = sendCommand,
    .readForResponse = readForResponse,
    .setMux = setMux,
    .configureTCPServer = configureTCPServer,
    .clearBuffer = clearBuffer
};
