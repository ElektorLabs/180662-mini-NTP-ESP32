#ifndef _NETWORK_H_
 #define _NETWORK_H_

#include <Arduino.h>

String SSIDList(String separator = ",");
void getSSIDList( void ) ;
void setWiFiSettings( void );
void getWiFiSettings( void );
void restart( void );
String getContentType(String filename);
void sendFile( void );
void sendData(String data);
void initWiFi( void );
bool connectWiFi( void );
void configureSoftAP( void );
void configureServer( void );
String WiFiStatusToString( void );
void NetworkTask( void );
int getQuality( void );
#endif
