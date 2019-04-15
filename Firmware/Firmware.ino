
/* 
 * Hardware used: 
 * 
 *  ESP32 based module from Elektor ( https://www.elektor.com/wemos-lolin-esp32-oled-module-with-wifi )
 *  GPS Receiver ( https://www.elektor.com/open-smart-gps-serial-gps-module-for-arduino-apm2-5-flight-control )
 *  DS3231 bases I²C RTC Module, e.g those used for the Pi
 * 
 * Librarys requierd:
 *  
 *  U8G2 by oliver
 *  Time by Michael Magolis
 *  Ticker by Bert Melis
 *  TinyGPS++ ( https://github.com/mikalhart/TinyGPSPlus )  
 *  RTCLib by Adafruit
 *  ArduinoJson 6.10.0
 *  CRC32 by Christopher Baker
 *  
 */

#include <WiFi.h>
#include <WiFiClient.h>
#include <esp_wifi.h>
#include <FS.h>
#include <SPIFFS.h>
#include <Wire.h>

#include <TimeLib.h>
#include <Ticker.h>
#include <TinyGPS++.h>
#include <RTClib.h>

#include <U8g2lib.h>


#include "ArduinoJson.h"
#include "timecore.h"
#include "datastore.h"

#include "ntp_server.h"

#define  GPSBAUD ( 9600 )

Timecore timec;








RTC_DS3231 rtc_clock;
//RTC_DS1307 rtc_clock;

HardwareSerial hws(1);
Ticker TimeKeeper;
TinyGPSPlus gps;
NTP_Server NTPServer;

//U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled_left(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
//U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled_right(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

/* As we use a pointer to the oled we need to make sure it's the same type as out displays */
U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled_left(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 4, /* data=*/ 5);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled_right(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 4, /* data=*/ 5);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C* oled_ptr=NULL;


/* 63 Char max and 17 missign for the mac */
TaskHandle_t GPSTaskHandle;
SemaphoreHandle_t xSemaphore = NULL;
SemaphoreHandle_t xi2cmtx = NULL;

//Used for the PPS interrupt 
const byte interruptPin = 25;


volatile uint32_t UptimeCounter=0;
volatile uint32_t GPS_Timeout=0;
volatile uint32_t pps_counter=0;
bool pps_active = false;
gps_settings_t gps_config;

/**************************************************************************************************
 *    Function      : GetUTCTime
 *    Description   : Reads the UTCTime
 *    Input         : none 
 *    Output        : uint32_t 
 *    Remarks       : none
 **************************************************************************************************/
 uint32_t GetUTCTime( void );

/**************************************************************************************************
 *    Function      : handlePPSInterrupt
 *    Description   : Interrupt from the GPS module
 *    Input         : none 
 *    Output        : none
 *    Remarks       : needs to be placed in RAM ans is only allowed to call functions also in RAM
 **************************************************************************************************/
void IRAM_ATTR handlePPSInterrupt() {
 pps_counter++;
 UptimeCounter++;
 timec.RTC_Tick();
 decGPSTimeout();
 pps_active = true; 
 xSemaphoreGiveFromISR( xSemaphore, NULL );       
}

/**************************************************************************************************
 *    Function      : setup
 *    Description   : Get all components in ready state
 *    Input         : none 
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void setup()
{
  /* First we setup the serial console with 115k2 8N1 */
  Serial.begin (115200);
  /* The next is to initilaize the datastore, here the eeprom emulation */
  datastoresetup();
  /* This is for the flash file system to access the webcontent */
  SPIFFS.begin();
  /* We setup the xSemaphore to sync the screen update */
  xSemaphore = xSemaphoreCreateBinary();
  /* We setup the i2c semaphore to sync i2c access */
  xi2cmtx = xSemaphoreCreateBinary();
  /* Make access to the i2c possible */
  xSemaphoreGive(xi2cmtx); 
   /* We read the Config from flash */
  Serial.println(F("Read Timecore Config"));
  timecoreconf_t cfg = read_timecoreconf();
  timec.SetConfig(cfg);
  /* This creates a new task bound to the APP CPU */
  xTaskCreatePinnedToCore(
   Display_Task,
   "Display_Task",
   10000,
   NULL,
   1,
   NULL,
   1);
  /* 
   * This delay the boot for a few seconds and will erase all config
   * if the boot btn is pressed 
   */
   
  Serial.println(F("Booting..."));
  for(uint32_t i=0;i<25;i++){
    if(digitalRead( 0 ) == false){
        Serial.println(F("Erase EEPROM"));
        erase_eeprom();  
        break;
    } else {
      vTaskDelay( 100 / portTICK_PERIOD_MS ); 
    }
  }
  /* We start to cpnfigure the WiFi */
  Serial.println(F("Init WiFi"));     
  initWiFi();
  /* 
   *  Next is to read how the GPS is configured
   *  here, if we use it for sync or not
   *  
   */
  gps_config = read_gps_config();
  /* We reassign the I2C Pins to 4 and 5 with 100kHz */
  Wire.begin(5,4,100000);

  /* This will check if the RTC is on the I2C */
  Wire.beginTransmission(0x68);
  if(Wire.endTransmission() == 0 ){

    /* Clock is found */
    Serial.print(F("I2C RTC at 0x68 found"));
    /* We now register the clock in the time core component */
    rtc_source_t I2C_DS3231;

    I2C_DS3231.SecondTick=NULL;
    I2C_DS3231.type = RTC_CLOCK;
    I2C_DS3231.ReadTime=RTC_ReadUnixTimeStamp;
    I2C_DS3231.WriteTime=RTC_WriteUnixTimestamp;
    timec.RegisterTimeSource(I2C_DS3231);

    /* Force a snyc to the clock */
    DateTime now = rtc_clock.now();
    timec.SetUTC(now.unixtime()  , RTC_CLOCK );
    
    /* Next is to output the time we have form the clock to the user */
    Serial.print(F("Read RTC Time:"));
    Serial.println(now.unixtime());
   
  } else {
    /* We can run without rtc */
    Serial.println("RTC is Missing");
  }
  
  /* Last step is to get the NTP running */
  NTPServer.begin(123 , GetUTCTime );
  /* Now we start with the config for the Timekeeping and sync */
  TimeKeeper.attach_ms(200, _200mSecondTick);

  /* We setup the PPS Pin as interrupt source */
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), handlePPSInterrupt, RISING);
 
  hws.begin( GPSBAUD ,SERIAL_8N1,13,15); // 13->RX , 15->TX
  /* We will drain the RX buffer to avoid having old data in it*/
  if (hws.available() > 0) {
                byte incomingByte = hws.read();
  }
  
}

/**************************************************************************************************
 *    Function      : GetUTCTime
 *    Description   : Returns the UTC Timestamp
 *    Input         : none 
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
uint32_t GetUTCTime( void ){
  uint32_t timest = 0;
  timest = timec.GetUTC();
  Serial.printf("Timestamp is %i\n\r",timest);
  return timest;
}

/**************************************************************************************************
 *    Function      : _200mSecondTick
 *    Description   : Runs all functions inside once a second
 *    Input         : none 
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void _200mSecondTick( void ){
   static uint8_t callcount=0;
   static uint8_t pps_check_cnt=0;
   static uint32_t pps_count_last = 0;
   static bool last_pps_state = false;

   if( (last_pps_state != pps_active) && ( true == pps_active ) ){
     Serial.println("Switch to PPS");
   }

   /* As long as the PPS is active we set the internal counter to zero */
   if(true == pps_active ){
     callcount=0;
   } else {
     callcount++;
   }

   /* If we have no new PPS interrupt we increment the timout value */
   if(pps_count_last != pps_counter){
      pps_check_cnt=0;
   } else {
      pps_check_cnt++;
   }

   /*  If we are running on internal clock
    *  we increment every  second the time 
    *  also we give to the xSemaphore to inform
    *  that the display needs to update
    */
   if( callcount >=5 ){
     if(false == pps_active ){
       timec.RTC_Tick();
       GPS_Timeout=0;
       UptimeCounter++; 
       xSemaphoreGive(  xSemaphore );   
     } 
     callcount=0;
        
   } 

   /*
    * if the pps is active and over 1200ms since the last time
    * we switch back to the internal clock and also we 
    * compensate the overdue by setting the next second 
    * to be fired 600ms after this
    * 
    */
   if( ( pps_active== true) && (pps_check_cnt>=7) ){ /*1400ms*/
    /* We assume that the pps is gone now and switch back to the internal timsource */
    /* Also we need to increment time here */
    pps_active=false;
    callcount=2;
    timec.RTC_Tick();
    GPS_Timeout=0;
    UptimeCounter++; 
    xSemaphoreGive(  xSemaphore );       
    /* The callcount shall now be 2 and we need  to set it to keep time*/
    Serial.println("Switch to internal clock");
   }
   
   pps_count_last=pps_counter;
   last_pps_state = pps_active;
   
}

/**************************************************************************************************
 *    Function      : decGPSTimeout
 *    Description   : decements the timputvaue for GPS Time Update
 *    Input         : none 
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void decGPSTimeout( void ){
  if(GPS_Timeout>0){
    GPS_Timeout--;
  }
}


/**************************************************************************************************
 *    Function      : loop
 *    Description   : Superloop
 *    Input         : none 
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void loop()
{  
  /* Process all networkservices */
  NetworkTask();
 
  /* timeupdate done here is here */
  while (hws.available()){
      gps.encode(hws.read());
      /* We check here if we have a new timestamp, and a valid GPS position  */
      if( (gps.date.isValid()==true) && ( gps.time.isValid()==true) && ( gps.location.isValid() == true ) ) {
        if(gps.time.isUpdated()==true){   
          uint32_t ts_age = gps.time.age(); //this are milliseconds and should be lower than 1 second 
          if(ts_age> 1000 ){
            Serial.println("Warning GPS Timestamp older than a second");      
          }
          /* We need to feed the gps task */
          datum_t newtime;
          newtime.year=gps.date.year();
          if(newtime.year>=2000){
            newtime.year-=2000;
          }
          newtime.month=gps.date.month();
          newtime.day=gps.date.day();
          newtime.dow=0;
          newtime.hour=gps.time.hour();
          newtime.minute=gps.time.minute();
          newtime.second=gps.time.second();
          /* We print the time for debug */
          if( (true == gps_config.sync_on_gps) && (GPS_Timeout<=0) ){
            Serial.println("Update Time from GPS");
            Serial.printf("Date is: %i/%i/%i at %i:%i:%i \r\n",newtime.year,newtime.month,newtime.day,newtime.hour,newtime.minute,newtime.second);
            timec.SetUTC(newtime,GPS_CLOCK);
            GPS_Timeout= 600; //10 Minute timeout
          } else {
            // Do nothing           
          }
      } 
    }      
  }
}


void Display_Task( void* param ){
  char timestr[9]={0,};
  char loc_timestr[9]={0,};
  char gps_lat_str[16]={0,};
  char gps_lng_str[16]={0,};
  char datestr[16]={0,};
  char loc_datestr[16]={0,};


  display_settings_t displayconfig = read_display_config();
  
  //oled_left.setI2CAddress(0x78);
  //oled_right.setI2CAddress(0x7A);
  oled_ptr= &oled_left;
  oled_ptr->setI2CAddress(0x7A);
  oled_ptr->begin();
  oled_ptr= &oled_right;
  oled_ptr->setI2CAddress(0x78);
  oled_ptr->begin();
 
  /* Display boot splsh to the oled */
  if(displayconfig.swap_display == true ){
    oled_ptr= &oled_right;
  } else {
    oled_ptr= &oled_left;
  }
  
  oled_ptr->setFont(u8g2_font_open_iconic_all_8x_t);
  oled_ptr->drawGlyph(0,64,247);
  oled_ptr->setFont(u8g2_font_inb16_mf); 
  oled_ptr->drawStr(58,16,"Setup");
  oled_ptr->drawStr(70,48,"WiFi");

  if( true == xSemaphoreTake(xi2cmtx,(500 / portTICK_PERIOD_MS) ) ){
         oled_ptr->sendBuffer();     
        xSemaphoreGive(xi2cmtx);
  }

  /* Display boot splsh to the oled */
  if(displayconfig.swap_display == false ){
    oled_ptr= &oled_right;
  } else {
    oled_ptr= &oled_left;
  }
  oled_ptr->clearBuffer(); 
  oled_ptr->setFont(u8g2_font_open_iconic_all_8x_t);
  oled_ptr->drawGlyph(32,64,269);

  if( true == xSemaphoreTake(xi2cmtx,(500 / portTICK_PERIOD_MS) ) ){
        oled_ptr->sendBuffer();     
        xSemaphoreGive(xi2cmtx);
  }

 

  
  /* clear the screen and draw the time */
  while(1==1){
    if( xSemaphoreTake( xSemaphore, portMAX_DELAY  ) == pdTRUE )
    {
      displayconfig = read_display_config();
      if(displayconfig.swap_display == true ){
        oled_ptr= &oled_right;
      } else {
        oled_ptr= &oled_left;
      }
      oled_ptr->clearBuffer();
      oled_ptr->setFont(u8g2_font_amstrad_cpc_extended_8f );
      oled_ptr->drawStr(0,8,"NTP Server ");
      oled_ptr->drawStr(0,16,WiFi.localIP().toString().c_str());
      /* Determine if we are AP or STA */
      wifi_mode_t op_mode;
      if(ESP_OK == esp_wifi_get_mode( &op_mode) ){
        if(WIFI_MODE_STA == op_mode){
          oled_ptr->setFont(u8g2_font_open_iconic_all_1x_t);
          oled_ptr->drawGlyph(120,8,247);
          
        } else {
          oled_ptr->setFont(u8g2_font_open_iconic_all_1x_t);
          oled_ptr->drawGlyph(120,8,125);
          
        }
      } else {
        /* Error collecting WiFi Mode */
          oled_ptr->setFont(u8g2_font_open_iconic_all_1x_t);
          oled_ptr->drawGlyph(120,8,121);
      }
      oled_ptr->setFont(u8g2_font_inb16_mn ); 
      datum_t utc_time = timec.ConvertToDatum(timec.GetUTC());
      snprintf(timestr, sizeof(timestr),"%02d:%02d:%02d",utc_time.hour,utc_time.minute,utc_time.second);
      oled_ptr->drawStr(8,42,timestr);
      oled_ptr->setFont(u8g2_font_amstrad_cpc_extended_8f );
      snprintf(datestr, sizeof(datestr),"%04d-%02d-%02d GMT",utc_time.year,utc_time.month,utc_time.day);
      oled_ptr->drawStr(16,58,datestr);
      oled_ptr->setFont(u8g2_font_open_iconic_all_1x_t);
      if(true==pps_active){
        oled_ptr->drawGlyph(0,58,197);
      } else {
        oled_ptr->drawGlyph(0,58,123);
      }
      if( true == xSemaphoreTake(xi2cmtx,(100 / portTICK_PERIOD_MS) ) ){    
        oled_ptr->sendBuffer();
        xSemaphoreGive(xi2cmtx);
      }


      /* on the other side we draw the gps location if we have one and also the local time */
      /* Display boot splsh to the oled */
      if(displayconfig.swap_display == false ){
        oled_ptr= &oled_right;
      } else {
        oled_ptr= &oled_left;
      }
      oled_ptr->clearBuffer();
      if (true ==  gps.location.isValid() ){
        oled_ptr->setFont(u8g2_font_amstrad_cpc_extended_8f );
        snprintf(gps_lng_str,sizeof(gps_lng_str),"Lng: %0.6f",gps.location.lng());
        snprintf(gps_lat_str,sizeof(gps_lat_str),"Lat: %0.6f",gps.location.lat());
        oled_ptr->drawStr(0,8,gps_lat_str);
        oled_ptr->drawStr(0,16,gps_lng_str);
        oled_ptr->setFont(u8g2_font_open_iconic_all_2x_t);
        oled_ptr->drawGlyph(16,112,209);
      } else {
         oled_ptr->setFont(u8g2_font_amstrad_cpc_extended_8f );
         oled_ptr->drawStr(24,16,"No GPS Fix");
      }
    
      
      oled_ptr->setFont(u8g2_font_inb16_mn ); 
      datum_t loc_time = timec.GetLocalTimeDate();
      snprintf(loc_timestr, sizeof(loc_timestr),"%02d:%02d:%02d",loc_time.hour,loc_time.minute,loc_time.second);
      oled_ptr->drawStr(8,42,loc_timestr);
      oled_ptr->setFont(u8g2_font_amstrad_cpc_extended_8f );
      snprintf(loc_datestr, sizeof(loc_datestr),"%04d-%02d-%02d",loc_time.year,loc_time.month,loc_time.day);
      oled_ptr->drawStr(16,58,loc_datestr);
      oled_ptr->setFont(u8g2_font_open_iconic_all_1x_t);
      oled_ptr->drawGlyph(104,58,184);
      oled_ptr->setFont(u8g2_font_open_iconic_all_1x_t);
      if(true==pps_active){
        oled_ptr->drawGlyph(0,58,197);
      } else {
        oled_ptr->drawGlyph(0,58,123);
      }
      
      if( true == xSemaphoreTake(xi2cmtx,(100 / portTICK_PERIOD_MS) ) ){
       
        oled_ptr->sendBuffer();
        xSemaphoreGive(xi2cmtx);
      }


      
    }
    

      
  }
  
}


/**************************************************************************************************
 *    Function      : RTC_ReadUnixTimeStamp
 *    Description   : Writes a UTC Timestamp to the RTC
 *    Input         : bool*  
 *    Output        : uint32_t
 *    Remarks       : Requiered to do some conversation
 **************************************************************************************************/
uint32_t RTC_ReadUnixTimeStamp(bool* delayed_result){
  DateTime now = 0;
  if( true == xSemaphoreTake(xi2cmtx,(100 / portTICK_PERIOD_MS) ) ){
   now = rtc_clock.now();
   xSemaphoreGive(xi2cmtx);
  }
   *delayed_result=false;
   return now.unixtime();
}


/**************************************************************************************************
 *    Function      : RTC_WriteUnixTimestamp
 *    Description   : Writes a UTC Timestamp to the RTC
 *    Input         : uint32_t 
 *    Output        : none
 *    Remarks       : Requiered to do some conversation
 **************************************************************************************************/
void RTC_WriteUnixTimestamp( uint32_t ts){
   uint32_t start_wait = millis();
   if( true == xSemaphoreTake(xi2cmtx,(40 / portTICK_PERIOD_MS) ) ){  
    ts = ts + ( ( millis()-start_wait)/1000);
    rtc_clock.adjust(DateTime( ts)); 
    DateTime now = rtc_clock.now();
    Serial.println("Update RTC");
    if( ts != now.unixtime() ){
      Serial.println(F("I2C-RTC W-Fault"));
    }
     xSemaphoreGive(xi2cmtx);
   }
   
  
}



 


