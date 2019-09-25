#include <ArduinoJson.h>
#include <WebServer.h>
#include <WebSocketsServer.h>

#include <TinyGPS++.h>
#include "timecore.h"
#include "datastore.h"

#include "webfunctions.h"

extern Timecore timec;
extern void sendData(String data);
extern WebServer * server;
extern TinyGPSPlus gps;

extern gps_settings_t gps_config;

/**************************************************************************************************
*    Function      : response_settings
*    Description   : Sends the timesettings as json 
*    Input         : non
*    Output        : none
*    Remarks       : none
**************************************************************************************************/ 
void response_settings(){
StaticJsonDocument<350> root;
char strbuffer[129];
String response="";  
  
  memset(strbuffer,0,129);
  datum_t d = timec.GetLocalTimeDate();
  snprintf(strbuffer,64,"%02d:%02d:%02d",d.hour,d.minute,d.second);
  
  root["time"] = strbuffer;
 
  memset(strbuffer,0,129);
  snprintf(strbuffer,64,"%04d-%02d-%02d",d.year,d.month,d.day);
  root["date"] = strbuffer;


  root["tzidx"] = (int32_t)timec.GetTimeZone();
  root["gpsena"] = true;
  root["zoneoverride"]=timec.GetTimeZoneManual();;
  root["gmtoffset"]=timec.GetGMT_Offset();;
  root["dlsdis"]=!timec.GetAutomacitDLS();
  root["dlsmanena"]=timec.GetManualDLSEna();
  uint32_t idx = timec.GetDLS_Offset();
  root["dlsmanidx"]=idx;
  root["gps_sync"]=gps_config.sync_on_gps;
  serializeJson(root, response);
  sendData(response);
}


/**************************************************************************************************
*    Function      : settime_update
*    Description   : Parses POST for new local time
*    Input         : none
*    Output        : none
*    Remarks       : none
**************************************************************************************************/ 
void settime_update( ){ /* needs to process date and time */
  datum_t d;
  d.year=2000;
  d.month=1;
  d.day=1;
  d.hour=0;
  d.minute=0;
  d.second=0;

  bool time_found=false;
  bool date_found=false;
  
  if( ! server->hasArg("date") || server->arg("date") == NULL ) { // If the POST request doesn't have username and password data
    /* we are missong something here */
  } else {
   
    Serial.printf("found date: %s\n\r",server->arg("date").c_str());
    uint8_t d_len = server->arg("date").length();
    Serial.printf("datelen: %i\n\r",d_len);
    if(server->arg("date").length()!=10){
      Serial.println("date len failed");
    } else {   
      String year=server->arg("date").substring(0,4);
      String month=server->arg("date").substring(5,7);
      String day=server->arg("date").substring(8,10);
      d.year = year.toInt();
      d.month = month.toInt();
      d.day = day.toInt();
      date_found=true;
    }   
  }

  if( ! server->hasArg("time") || server->arg("time") == NULL ) { // If the POST request doesn't have username and password data
    
  } else {
    if(server->arg("time").length()!=8){
      Serial.println("time len failed");
    } else {
    
      String hour=server->arg("time").substring(0,2);
      String minute=server->arg("time").substring(3,5);
      String second=server->arg("time").substring(6,8);
      d.hour = hour.toInt();
      d.minute = minute.toInt();
      d.second = second.toInt();     
      time_found=true;
    }
     
  } 
  if( (time_found==true) && ( date_found==true) ){
    Serial.printf("Date: %i, %i, %i ", d.year , d.month, d.day );
    Serial.printf("Time: %i, %i, %i ", d.hour , d.minute, d.second );
    timec.SetLocalTime(d);
  }
  
  server->send(200);   
 
 }


/**************************************************************************************************
*    Function      : timezone_update
*    Description   : Parses POST for new timezone settings
*    Input         : none
*    Output        : none
*    Remarks       : none
**************************************************************************************************/  
void timezone_update( ){ /*needs to handel timezoneid */
  if( ! server->hasArg("timezoneid") || server->arg("timezoneid") == NULL ) { // If the POST request doesn't have username and password data
    /* we are missong something here */
  } else {
   
    Serial.printf("New TimeZoneID: %s\n\r",server->arg("timezoneid").c_str());
    uint32_t timezoneid = server->arg("timezoneid").toInt();
    timec.SetTimeZone( (TIMEZONES_NAMES_t)timezoneid );   
  }
  timec.SaveConfig();
  server->send(200);    

 }

 
/**************************************************************************************************
*    Function      : timezone_overrides_update
*    Description   : Parses POST for new timzone overrides
*    Input         : none
*    Output        : none
*    Remarks       : none
**************************************************************************************************/  
 void timezone_overrides_update( ){ /* needs to handle DLSOverrid,  ManualDLS, dls_offset, ZONE_OVERRRIDE and GMT_OFFSET */

  bool DLSOverrid=false;
  bool ManualDLS = false;
  bool ZONE_OVERRRIDE = false;
  int32_t gmt_offset = 0;
  DLTS_OFFSET_t dls_offsetidx = DLST_OFFSET_0;
  if( ! server->hasArg("dlsdis") || server->arg("dlsdis") == NULL ) { // If the POST request doesn't have username and password data
      /* we are missing something here */
  } else {
    DLSOverrid=true;  
  }

  if( ! server->hasArg("dlsmanena") || server->arg("dlsmanena") == NULL ) { // If the POST request doesn't have username and password data
      /* we are missing something here */
  } else {
    ManualDLS=true;  
  }

  if( ! server->hasArg("ZONE_OVERRRIDE") || server->arg("ZONE_OVERRRIDE") == NULL ) { // If the POST request doesn't have username and password data
      /* we are missing something here */
  } else {
    ZONE_OVERRRIDE=true;  
  }

  if( ! server->hasArg("gmtoffset") || server->arg("gmtoffset") == NULL ) { // If the POST request doesn't have username and password data
      /* we are missing something here */
  } else {
    gmt_offset = server->arg("gmtoffset").toInt();
  }

  if( ! server->hasArg("dlsmanidx") || server->arg("dlsmanidx") == NULL ) { // If the POST request doesn't have username and password data
      /* we are missing something here */
  } else {
    dls_offsetidx = (DLTS_OFFSET_t) server->arg("dlsmanidx").toInt();
  }
  timec.SetGMT_Offset(gmt_offset);
  timec.SetDLS_Offset( (DLTS_OFFSET_t)(dls_offsetidx) );
  timec.SetAutomaticDLS(!DLSOverrid);
  timec.SetManualDLSEna(ManualDLS);
  timec.SetTimeZoneManual(ZONE_OVERRRIDE);

 
  timec.SaveConfig();
  server->send(200);    

  
 }










/**************************************************************************************************
*    Function      : update_notes
*    Description   : Parses POST for new notes
*    Input         : none
*    Output        : none
*    Remarks       : none
**************************************************************************************************/ 
void update_notes(){
  char data[501]={0,};
  if( ! server->hasArg("notes") || server->arg("notes") == NULL ) { // If the POST request doesn't have username and password data
    /* we are missing something here */
  } else {
   
    Serial.printf("New Notes: %s\n\r",server->arg("notes").c_str());
    /* direct commit */
    uint32_t str_size = server->arg("notes").length();
    if(str_size<501){
      strncpy((char*)data,server->arg("notes").c_str(),501);
      eepwrite_notes((uint8_t*)data,501);
    } else {
      Serial.println("Note > 512 char");
    }
  }

  server->send(200);    
}

/**************************************************************************************************
*    Function      : read_notes
*    Description   : none
*    Input         : none
*    Output        : none
*    Remarks       : Retunrs the notes as plain text
**************************************************************************************************/ 
void read_notes(){
  char data[501]={0,};
  eepread_notes((uint8_t*)data,501);
  sendData(data);    
}


/**************************************************************************************************
*    Function      : getipv4settings_settings
*    Description   : Sends the ipv4 settings as json 
*    Input         : non
*    Output        : none
*    Remarks       : none
**************************************************************************************************/ 
void getipv4settings_settings(){
StaticJsonDocument<2048> doc;
String response="";  

ipv4_settings s = read_ipv4_settings();
  
  doc["USE_STATIC"] = s.use_static;

  JsonArray IP = doc.createNestedArray("IP");
  IP.add(  (uint8_t)( ( s.address >> 0) & 0xFF) );
  IP.add(  (uint8_t)( ( s.address >> 8) & 0xFF) );
  IP.add(  (uint8_t)( ( s.address >> 16) & 0xFF) );
  IP.add(  (uint8_t)( ( s.address >> 24) & 0xFF) );
  
  JsonArray SUBNET = doc.createNestedArray("SUBNET");
  SUBNET.add( (uint8_t)( ( s.subnet >> 0) & 0xFF));
  SUBNET.add( (uint8_t)( ( s.subnet >> 8) & 0xFF));
  SUBNET.add( (uint8_t)( ( s.subnet >> 16) & 0xFF));
  SUBNET.add( (uint8_t)( ( s.subnet >> 24) & 0xFF));
  
  JsonArray GATEWAY = doc.createNestedArray("GATEWAY");
  GATEWAY.add( (uint8_t)( ( s.gateway >> 0) & 0xFF));
  GATEWAY.add( (uint8_t)( ( s.gateway >> 8) & 0xFF));
  GATEWAY.add( (uint8_t)( ( s.gateway >> 16) & 0xFF));
  GATEWAY.add( (uint8_t)( ( s.gateway >> 24) & 0xFF));
  
  JsonArray DNS0 = doc.createNestedArray("DNS0");
  DNS0.add( (uint8_t)( ( s.dns0 >> 0) & 0xFF));
  DNS0.add( (uint8_t)( ( s.dns0 >> 8) & 0xFF));
  DNS0.add( (uint8_t)( ( s.dns0 >> 16) & 0xFF));
  DNS0.add( (uint8_t)( ( s.dns0 >> 24) & 0xFF));
  
  JsonArray DNS1 = doc.createNestedArray("DNS1");
  DNS1.add( (uint8_t)( ( s.dns1 >> 0) & 0xFF));
  DNS1.add( (uint8_t)( ( s.dns1 >> 8) & 0xFF));
  DNS1.add( (uint8_t)( ( s.dns1 >> 16) & 0xFF));
  DNS1.add( (uint8_t)( ( s.dns1 >> 24) & 0xFF));
  serializeJson(doc, response);
  sendData(response);
}




/**************************************************************************************************
*    Function      : getGPS_Location
*    Description   : none
*    Input         : none
*    Output        : none
*    Remarks       : Retunrs the GPS location
**************************************************************************************************/ 
void getGPS_Location(){
String response;
const size_t capacity = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(3);
DynamicJsonDocument  root(capacity);

  root["sensor"] = "gps";
  root["utc_time"] = timec.GetUTC();

  if (false == gps.location.isValid())
  {
    root["valid"] = false;
  } else {
    root["valid"] = true;
  }

  JsonArray data = root.createNestedArray("data");
  data.add(gps.location.lat());
  data.add(gps.location.lng());
  serializeJson(root, response);
  sendData(response);
  
}

/**************************************************************************************************
*    Function      : update_gps_syncclock
*    Description   : set or unset form web is gps will be used to sync clock
*    Input         : none
*    Output        : none
*    Remarks       : none
**************************************************************************************************/
void update_gps_syncclock( void ){
  if( ! server->hasArg("GPS_SYNC_ON") || server->arg("GPS_SYNC_ON") == NULL ) { // If the POST request doesn't have username and password data
    /* we are missing something here */
    gps_config.sync_on_gps = false;
  } else {
    gps_config.sync_on_gps = true;
   /* Enable Sync on GPS */  
  }
  write_gps_config((gps_settings_t)(gps_config));
  server->send(200);   

}


/**************************************************************************************************
*    Function      : update_display_settings
*    Description   : set or unset form web if the display will be swapped
*    Input         : none
*    Output        : none
*    Remarks       : none
**************************************************************************************************/
void update_display_settings( void ){
  display_settings_t disp_config = read_display_config( );
  if( ! server->hasArg("SWAP_DISP") || server->arg("SWAP_DISP") == NULL ) { // If the POST request doesn't have username and password data
    /* we are missing something here */
    disp_config.swap_display = false;
  } else {
    if(server->arg("SWAP_DISP")=="true"){
      disp_config.swap_display = true;
    } else {
      disp_config.swap_display = false;
    }
   /* Enable Swap */  
  }  
  write_display_config(disp_config);
  server->send(200);   

}


void update_ipv4_settings( void ){
  ipv4_settings s = read_ipv4_settings();
  Serial.println("New IPv4 Settings");
  if( ! server->hasArg("JSON") || server->arg("JSON") == NULL ) { // If the POST request doesn't have username and password data
      /* we are missing something here */
      Serial.println("JSON Print missing");
  } else {
    



 if(server->arg("JSON").length() < 2048 ){ /* we only process messages up to 2048 elements */
  DynamicJsonDocument doc(2048);
 
  if(0 != deserializeJson(doc, server->arg("JSON" ) ) ){
    Serial.println("JSON deserilize failed");
    server->send(200);  
    return;
  }
   
    bool USE_STATIC = doc["USE_STATIC"];
    s.use_static = USE_STATIC;
    Serial.print("Static Mode=");
    Serial.println(USE_STATIC);
    
    JsonArray  IP = doc["IP"];
    if( (IP.isNull() != true)  ){
      uint8_t IP_0 = IP[0]; // 192
      uint8_t IP_1 = IP[1]; // 168
      uint8_t IP_2 = IP[2]; // 0
      uint8_t IP_3 = IP[3]; // 1
      uint32_t addr = ( IP_0 << 0 ) | ( IP_1 << 8 ) | ( IP_2 << 16 ) | ( IP_3 << 24 ) ;
      IPAddress IP_ADR(addr);
      Serial.print("New IP:");
      Serial.println(IP_ADR);
      s.address = addr;
    }
      
    

    JsonArray  SUBNET = doc["SUBNET"];
    if( (SUBNET.isNull() != true)  ){
      uint8_t SUBNET_0 = SUBNET[0]; // 255
      uint8_t SUBNET_1 = SUBNET[1]; // 255
      uint8_t SUBNET_2 = SUBNET[2]; // 255
      uint8_t SUBNET_3 = SUBNET[3]; // 0
      uint32_t subnet = ( SUBNET_0 << 0 ) | ( SUBNET_1 << 8 ) | ( SUBNET_2 << 16 ) | ( SUBNET_3 << 24 ) ;
      IPAddress SUBNET_ADR(subnet);
      Serial.print("New SUBNET:");
      Serial.println(SUBNET_ADR);
      s.subnet = subnet;
    }

    JsonArray  GATEWAY = doc["GATEWAY"];
    if( (GATEWAY.isNull() != true)  ){
      uint8_t GATEWAY_0 = GATEWAY[0]; // 192
      uint8_t GATEWAY_1 = GATEWAY[1]; // 168
      uint8_t GATEWAY_2 = GATEWAY[2]; // 0
      uint8_t GATEWAY_3 = GATEWAY[3]; // 1  
      uint32_t gateway = ( GATEWAY_0 << 0 ) | ( GATEWAY_1 << 8 ) | ( GATEWAY_2 << 16 ) | ( GATEWAY_3 << 24 ) ;
      IPAddress GATEWAY_ADR(gateway);
      Serial.print("New GATEWAY:");
      Serial.println(GATEWAY_ADR);
      s.gateway = gateway;
    }

    JsonArray  DNS0 = doc["DNS0"];
    if( (DNS0.isNull() != true)  ){
      uint8_t DNS0_0 = DNS0[0]; // 192
      uint8_t DNS0_1 = DNS0[1]; // 168
      uint8_t DNS0_2 = DNS0[2]; // 0
      uint8_t DNS0_3 = DNS0[3]; // 1  
      uint32_t dns0 = ( DNS0_0 << 0 ) | ( DNS0_1 << 8 ) | ( DNS0_2 << 16 ) | ( DNS0_3 << 24 ) ;
      IPAddress DNS_ADR(dns0);
      Serial.print("New DNS0:");
      Serial.println(DNS_ADR);
      s.dns0 = dns0;
    }

    JsonArray  DNS1 = doc["DNS1"];
    if( (DNS1.isNull() != true) ){
    
      uint8_t DNS1_0 = DNS1[0]; // 192
      uint8_t DNS1_1 = DNS1[1]; // 168
      uint8_t DNS1_2 = DNS1[2]; // 0
      uint8_t DNS1_3 = DNS1[3]; // 1
      uint32_t dns1 = ( DNS1_0 << 0 ) | ( DNS1_1 << 8 ) | ( DNS1_2 << 16 ) | ( DNS1_3 << 24 ) ;  
      IPAddress DNS_ADR(dns1);
      s.dns1 = dns1;          
      Serial.print("New DNS1:");
      Serial.println(DNS_ADR);
    }
  
  }else {
    Serial.println("JSON to long");
  }

  // We print the new settings and save them
  write_ipv4_settings(s);
  } 
  server->send(200);  
  
}
/**************************************************************************************************
*    Function      : send_display_settings
*    Description   : set or unset form web if the display will be swapped
*    Input         : none
*    Output        : none
*    Remarks       : none
**************************************************************************************************/
void send_display_settings( void ){
  display_settings_t disp_config = read_display_config( );
  String response ="";
  const size_t capacity = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(3);
  DynamicJsonDocument  root(capacity);
  
  root["swap_display"] = disp_config.swap_display;
  serializeJson(root, response);
  sendData(response);

}
