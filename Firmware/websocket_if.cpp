#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include "websocket_if.h"



WebSocketsServer webSocket = WebSocketsServer(8080);
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);

void ws_service_begin(){
  
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  
}

void ws_task( void ){
  webSocket.loop();
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    StaticJsonBuffer<200> jsonBuffer;
    switch(type) {
        case WStype_DISCONNECTED:
        {
            Serial.printf("[%u] Disconnected!\n", num);
        }
        break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
                webSocket.sendTXT(num, "Connected");
            }
            break;
        case WStype_TEXT:
                {
                    printf("[%u] get Text: %s\n", num, payload);
                }
            break;
        case WStype_BIN:
                printf("[%u] get binary length: %u\n", num, length);
            break;

       default:{
        
       }break;
    }

}
