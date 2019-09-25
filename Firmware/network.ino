#include "datastore.h"
#include "webfunctions.h"

#include <WebServer.h>
#include <ESPmDNS.h>        // Include the mDNS library



String APSSID = "Elektor NTP Server";

String ssid ="";
String pass="";
IPAddress ip;
bool IsAP=false;

WebServer * server =NULL;


/**************************************************************************************************
 *    Function      : SSIDList
 *    Description   : Returns the SSID List 
 *    Input         : String separator 
 *    Output        : String
 *    Remarks       : none
 **************************************************************************************************/
String SSIDList(String separator = ",") {
  Serial.println("Scanning networks");
  String ssidList;
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; i++) {
    String ssid = WiFi.SSID(i);
    Serial.println(String(i) + ": " + ssid);
    if (ssidList.indexOf(ssid) != -1) {
      Serial.println("SSID already in list");
    }
    else {
      if (ssidList != "")
        ssidList += separator;
      ssidList += ssid;
    }
  }
  return ssidList;
}

/**************************************************************************************************
 *    Function      : getSSIDList
 *    Description   : Sends a SSID List to the client 
 *    Input         : none
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void getSSIDList() {
  Serial.println("SSID list requested");
  sendData(SSIDList());
}

/**************************************************************************************************
 *    Function      : setWiFiSettings
 *    Description   : Applies the WiFi settings 
 *    Input         : none
 *    Output        : none
 *    Remarks       : Store the wifi settings configured on the webpage and restart the esp to connect to this network
 **************************************************************************************************/
void setWiFiSettings() {
  credentials_t c;
  Serial.println("WiFi settings received");
  ssid = server->arg("ssid");
  pass = server->arg("pass");
  String response = "Attempting to connect to '" + ssid + "'. The WiFi module restarts and tries to connect to the network.";
  sendData(response);
  Serial.println("Saving network credentials and restart.");
  strncpy((char*)(c.ssid),(char*)(ssid.c_str()),128);
  strncpy((char*)(c.pass),(char*)(pass.c_str()),128);
  Serial.printf("write ssid:%s ,pass:%s \n\r",c.ssid,c.pass);
  write_credentials(c);
  
  c = read_credentials();
  Serial.printf("read ssid:%s ,pass:%s \n\r",c.ssid,c.pass);
  /* if we do this we end up in flashloader */
  WiFi.softAPdisconnect(true);
  delay(2000);
 /* Any eSP8266 fix goes here */
  ESP.restart();
}

//send the wifi settings to the connected client of the webserver
void getWiFiSettings() {
  Serial.println("WiFi settings requested");
  String response;
  response += ssid + ",";
  response += SSIDList(";");
  sendData(response);
}




/**************************************************************************************************
 *    Function      : setWiFiSettings
 *    Description   : Applies the WiFi settings 
 *    Input         : none
 *    Output        : none
 *    Remarks       : restart the esp as requested on the webpage
 **************************************************************************************************/
void restart() {
  sendData("The ESP will restart and you will be disconnected from the '" + APSSID + WiFi.macAddress() + "' network.");
  delay(1000);
  /* Any fixes for ESP8266 may be here */
  ESP.restart();
}

/**************************************************************************************************
 *    Function      : getContentType
 *    Description   : Gets the contenttype depending on a filename 
 *    Input         : String
 *    Output        : String
 *    Remarks       : none
 **************************************************************************************************/
String getContentType(String filename) {
  if (server->hasArg("download")) return "application/octet-stream";
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

/**************************************************************************************************
 *    Function      : sendFile
 *    Description   : Sends a requestedfile from SPIFFS
 *    Input         : none 
 *    Output        : none
 *    Remarks       : send a file from the SPIFFS to the connected client of the webserver
 **************************************************************************************************/
void sendFile() {
  String path = server->uri();
  Serial.println("Got request for: " + path);
  if (path.endsWith("/")) path += "index.html";
  String contentType = getContentType(path);
  Serial.println("Content type: " + contentType);
  if (SPIFFS.exists(path)) {
    Serial.println("File " + path + " found");
    File file = SPIFFS.open(path, "r");
    server->streamFile(file, contentType);
    file.close();
  }
  else {
    Serial.println("File '" + path + "' doesn't exist");
    server->send(404, "text/plain", "The requested file doesn't exist");
  }
  

}

/**************************************************************************************************
 *    Function      : sendFile
 *    Description   : Sends a requestedfile from SPIFFS
 *    Input         : String 
 *    Output        : none
 *    Remarks       : send data to the connected client of the webserver
 **************************************************************************************************/
void sendData(String data) {
  server->send(200, "text/plain", data);
}

/**************************************************************************************************
 *    Function      : initWiFi
 *    Description   : initializes the WiFi
 *    Input         : String 
 *    Output        : none
 *    Remarks       : initialize wifi by connecting to a wifi network or creating an accesspoint
 **************************************************************************************************/
void initWiFi() {

credentials_t c =  read_credentials();
  Serial.print("WiFi: ");
 
  
  if ( 0==c.ssid[0]) {
    Serial.println("AP");
    configureSoftAP();
   
  }
  else {
    Serial.println("STA");
    
 
      ssid=String(c.ssid);
      pass=String(c.pass);
      if(true==connectWiFi()){
        configureServer();
      } else {
        configureSoftAP();
      }  
  }
   
}

/**************************************************************************************************
 *    Function      : connectWiFi
 *    Description   : trys to establish a WiFi connection
 *    Input         : none 
 *    Output        : bool
 *    Remarks       : connect the esp to a wifi network, retuns false if failed
 **************************************************************************************************/
bool connectWiFi() {
  ipv4_settings nws;
  nws= read_ipv4_settings();

  if (ssid == "") {
    Serial.println("SSID unknown");
   
    return false;
  }
  WiFi.mode(WIFI_STA);
  Serial.println("Attempting to connect to " + ssid + ", pass: " + pass);
  if(false != nws.use_static ){
    IPAddress local_IP(nws.address);
    IPAddress gateway(nws.gateway);
    IPAddress subnet(nws.subnet);
    IPAddress primaryDNS(nws.dns0); //optional
    IPAddress secondaryDNS(nws.dns1); //optional
    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
      Serial.println("STA Failed to configure");
    }

   
  }
  
  WiFi.begin(( char*)ssid.c_str(), ( char*)pass.c_str());
  for (int timeout = 0; timeout < 15; timeout++) { //max 15 seconds
    int status = WiFi.status();
    if ((status == WL_CONNECTED)  || (status == WL_NO_SSID_AVAIL) || (status == WL_CONNECT_FAILED))
      break;
    Serial.print(".");
  
    delay(1000);
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Failed to connect to " + ssid);
    Serial.println("Second attempt " + ssid);
    WiFi.disconnect(true);
    WiFi.begin(( char*)ssid.c_str(), ( char*)pass.c_str());
    for (int timeout = 0; timeout < 15; timeout++) { //max 15 seconds
      int status = WiFi.status();
      if ((status == WL_CONNECTED)  || (status == WL_NO_SSID_AVAIL) || (status == WL_CONNECT_FAILED))
        break;
      Serial.print(".");
      delay(1000);
    }
  
  }
  Serial.println();
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Failed to connect to " + ssid);
    Serial.println("WiFi status: " + WiFiStatusToString());
    
    WiFi.disconnect();

  
    return false;
  }
  Serial.println("Connected to " + ssid);
  Serial.print("Local IP: ");
  ip = WiFi.localIP();
  Serial.println(ip);   
  return true;
}


/**************************************************************************************************
 *    Function      : configureSoftAP
 *    Description   : Configures the ESP as SoftAP
 *    Input         : none 
 *    Output        : none
 *    Remarks       : configure the access point of the esp
 **************************************************************************************************/
void configureSoftAP() {
  IsAP=true;
  Serial.println("Configuring AP: " + String(APSSID));
  
  WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
  WiFi.softAP(APSSID.c_str(), NULL, 1, 0, 1);
  Serial.print("AP IP: ");
  Serial.println(ip);
  
  if (!MDNS.begin("ntpserver")) {             // Start the mDNS responder for esp8266.local
    Serial.println("Error setting up MDNS responder!");
  }
  Serial.println("mDNS responder started");
  configureServer();
  ip = WiFi.softAPIP();
}

/**************************************************************************************************
 *    Function      : configureServer
 *    Description   : Setup for the Webserver
 *    Input         : none 
 *    Output        : none
 *    Remarks       : initialize the webserver on port 80
 **************************************************************************************************/
void configureServer() {
  server = new WebServer (80);
  server->on("/setWiFiSettings", HTTP_GET, setWiFiSettings);
  server->on("/getWiFiSettings", HTTP_GET, getWiFiSettings);
  server->on("/getSSIDList", HTTP_GET, getSSIDList);
  server->on("/restart", HTTP_GET, restart);
  server->on("/timesettings", HTTP_GET, response_settings);
  server->on("/settime.dat", HTTP_POST, settime_update); /* needs to process date and time */
  server->on("/timezone.dat",timezone_update); /*needs to handel timezoneid */
  server->on("/overrides.dat",timezone_overrides_update); /* needs to handle DLSOverrid,  ManualDLS, dls_offset, ZONE_OVERRRIDE and GMT_OFFSET */
  server->on("/notes.dat",HTTP_GET,read_notes);
  server->on("/notes.dat",HTTP_POST,update_notes);
  server->on("/gps/syncclock.dat",HTTP_POST,update_gps_syncclock);
  server->on("/gps/data",HTTP_GET,getGPS_Location);
  server->on("/display/settings",HTTP_GET,send_display_settings);
  server->on("/display/settings",HTTP_POST,update_display_settings);  
  server->on("/ipv4settings.json",HTTP_GET,getipv4settings_settings);
  server->on("/ipv4settings.json",HTTP_POST,update_ipv4_settings);
  server->onNotFound(sendFile); //handle everything except the above things
  server->begin();
  Serial.println("Webserver started");
}



/**************************************************************************************************
 *    Function      : WiFiStatusToString
 *    Description   : Gives a string representing the WiFi status
 *    Input         : none 
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
String WiFiStatusToString() {
  switch (WiFi.status()) {
    case WL_IDLE_STATUS:     return "IDLE"; break;
    case WL_NO_SSID_AVAIL:   return "NO SSID AVAIL"; break;
    case WL_SCAN_COMPLETED:  return "SCAN COMPLETED"; break;
    case WL_CONNECTED:       return "CONNECTED"; break;
    case WL_CONNECT_FAILED:  return "CONNECT_FAILED"; break;
    case WL_CONNECTION_LOST: return "CONNECTION LOST"; break;
    case WL_DISCONNECTED:    return "DISCONNECTED"; break;
    case WL_NO_SHIELD:       return "NO SHIELD"; break;
    default:                 return "Undefined: " + String(WiFi.status()); break;
  }
}

/**************************************************************************************************
 *    Function      : NetworkTask
 *    Description   : All things that needs to be done in the superloop
 *    Input         : none 
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void NetworkTask(){

  if(server != NULL){
    server->handleClient();
  }
}


/**************************************************************************************************
 *    Function      : getQuality
 *    Description   : Gets the Signalquality
 *    Input         : none 
 *    Output        : none
 *    Remarks       : Return the quality (Received Signal Strength Indicator)
                      of the WiFi network.
                      Returns a number between 0 and 100 if WiFi is connected.
                      Returns -1 if WiFi is disconnected.
 **************************************************************************************************/
int getQuality() {
  if (WiFi.status() != WL_CONNECTED)
    return -1;
  int dBm = WiFi.RSSI();
  if (dBm <= -100)
    return 0;
  if (dBm >= -50)
    return 100;
  return 2 * (dBm + 100);
}
