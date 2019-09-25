#include <EEPROM.h>
#include <CRC32.h>
#include "datastore.h"


/* function prototypes */
void eepwrite_struct(void* data_in, uint32_t e_size, uint32_t address );
bool eepread_struct( void* element, uint32_t e_size, uint32_t startaddr  );

/* This will be the layout used by the data within the flash */
#define CREDENTIALS_START 0
/* credentials are 256+4Byte */
#define TIMECORECONFIG_START 280
/* config is 24 byte + 4 byte */

#define GPSCONFIG_START 320
/* config is 4 byte + 4 byte */

#define DISPLAYCONFIG_START 400

#define IPV4SETTINGS_START  420

#define NOTES_START 500
/* notes take 512 byte */



/**************************************************************************************************
 *    Function      : datastoresetup
 *    Description   : Gets the EEPROM Emulation set up
 *    Input         : none 
 *    Output        : none
 *    Remarks       : We use 4096 byte for EEPROM 
 **************************************************************************************************/
void datastoresetup() {
  /* We emulate 4096 byte here */
  EEPROM.begin(4096);

}

/**************************************************************************************************
 *    Function      : write_display_config
 *    Description   : writes the display config
 *    Input         : display_settings_t
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void write_display_config(display_settings_t c){
   eepwrite_struct( ( (void*)(&c) ), sizeof(display_settings_t) , DISPLAYCONFIG_START );  
}

/**************************************************************************************************
 *    Function      : read_display_config
 *    Description   : reads the display config
 *    Input         : display_settings_t
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
display_settings_t read_display_config( void ){
   display_settings_t retval;
  if(false == eepread_struct( (void*)(&retval), sizeof(display_settings_t) , DISPLAYCONFIG_START ) ){ 
    Serial.println("DISPLAY CONF");
     bzero((void*)&retval,sizeof( display_settings_t ));
     write_display_config(retval);
  }
  return retval;
}


/**************************************************************************************************
 *    Function      : write_timecoreconf
 *    Description   : writes the time core config
 *    Input         : timecoreconf_t
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void write_timecoreconf(timecoreconf_t c){
  eepwrite_struct( ( (void*)(&c) ), sizeof(timecoreconf_t) , TIMECORECONFIG_START );  
}


/**************************************************************************************************
 *    Function      : read_timecoreconf
 *    Description   : reads the time core config
 *    Input         : none
 *    Output        : timecoreconf_t
 *    Remarks       : none
 **************************************************************************************************/
timecoreconf_t read_timecoreconf( void ){
  timecoreconf_t retval;
  if(false == eepread_struct( (void*)(&retval), sizeof(timecoreconf_t) , TIMECORECONFIG_START ) ){ 
    Serial.println("TIME CONF");
    retval = Timecore::GetDefaultConfig();
    write_timecoreconf(retval);
  }
  return retval;
}

/**************************************************************************************************
 *    Function      : write_gps_config
 *    Description   : writes the gps config
 *    Input         : gps_settings_t
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void write_gps_config(gps_settings_t c){
  eepwrite_struct( ( (void*)(&c) ), sizeof(gps_settings_t) , GPSCONFIG_START );
}

/**************************************************************************************************
 *    Function      : read_gps_config
 *    Description   : reads the gops config
 *    Input         : gps_settings_t
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
gps_settings_t read_gps_config( void ){
  gps_settings_t gps_conf;
  if(false == eepread_struct( (void*)(&gps_conf), sizeof(gps_settings_t) , GPSCONFIG_START ) ){ 
    bzero((void*)&gps_conf,sizeof( gps_settings_t ));
    gps_conf.sync_on_gps = true;
    eepwrite_struct( ( (void*)(&gps_conf) ), sizeof(gps_settings_t) , GPSCONFIG_START );
  }
  return gps_conf;   
}


/**************************************************************************************************
 *    Function      : write_credentials
 *    Description   : writes the wifi credentials
 *    Input         : credentials_t
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void write_credentials(credentials_t c){
   eepwrite_struct( ( (void*)(&c) ), sizeof(credentials_t) , CREDENTIALS_START );
}

/**************************************************************************************************
 *    Function      : read_credentials
 *    Description   : reads the wifi credentials
 *    Input         : none
 *    Output        : credentials_t
 *    Remarks       : none
 **************************************************************************************************/
credentials_t read_credentials( void ){
  credentials_t retval;
  if(false == eepread_struct( (void*)(&retval), sizeof(credentials_t) , CREDENTIALS_START ) ){ 
    Serial.println("WIFI CONF");
    bzero((void*)&retval,sizeof( credentials_t ));
    write_credentials(retval);
  }
  return retval;
}

/**************************************************************************************************
 *    Function      : write_credentials
 *    Description   : writes the wifi ipv4 settings
 *    Input         : ipv4_settings
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void write_ipv4_settings(ipv4_settings c){
  eepwrite_struct( ( (void*)(&c) ), sizeof(ipv4_settings) , IPV4SETTINGS_START );
}


/**************************************************************************************************
 *    Function      : read_credentials
 *    Description   : reads the ipv4 settings
 *    Input         : none
 *    Output        : ipv4_settings
 *    Remarks       : none
 **************************************************************************************************/
ipv4_settings read_ipv4_settings( void ){
  ipv4_settings retval;
  if(false == eepread_struct( (void*)(&retval), sizeof(ipv4_settings) , IPV4SETTINGS_START ) ){ 
    Serial.println("WIFI IPv4");
    bzero((void*)&retval,sizeof( ipv4_settings ));
    write_ipv4_settings(retval);
  }
  return retval;
}

/**************************************************************************************************
 *    Function      : eepread_struct
 *    Description   : reads a given block from flash / eeprom 
 *    Input         : void* element, uint32_t e_size, uint32_t startaddr  
 *    Output        : bool ( true if read was okay )
 *    Remarks       : Reads a given datablock into flash and checks the the crc32 
 **************************************************************************************************/
bool eepread_struct( void* element, uint32_t e_size, uint32_t startaddr  ){
  
  bool done = true;
  CRC32 crc;
  //Serial.println("Read EEPROM");
  uint8_t* ret_ptr=(uint8_t*)(element);
  uint8_t data;
  
  for(uint32_t i=startaddr;i<(e_size+startaddr);i++){
      data = EEPROM.read(i);
      crc.update(data);
      *ret_ptr=data;
      ret_ptr++; 
  }
  /* Next is to read the crc32*/
  uint32_t data_crc = crc.finalize(); 
  uint32_t saved_crc=0;
  uint8_t* bytedata = (uint8_t*)&saved_crc;
  for(uint32_t z=e_size+startaddr;z<e_size+startaddr+sizeof(data_crc);z++){
    *bytedata=EEPROM.read(z);
    bytedata++;
  } 
  
  if(saved_crc!=data_crc){
    Serial.println("SAVED CONF");
    done = false;
  }

  return done;
}

/**************************************************************************************************
 *    Function      : eepwrite_struct
 *    Description   : writes the display settings
 *    Input         : void* data, uint32_t e_size, uint32_t address 
 *    Output        : bool
 *    Remarks       : Writes a given datablock into flash and adds a crc32 
 **************************************************************************************************/
void eepwrite_struct(void* data_in, uint32_t e_size, uint32_t address ){
  //Serial.println("Write EEPROM");
  uint8_t* data=(uint8_t*)(data_in);
  CRC32 crc;
  
 
  for(uint32_t i=address;i<e_size+address;i++){
      EEPROM.write(i,*data);
      crc.update(*data);
      data++;
  }
 /* the last thing to do is to calculate the crc32 for the data and write it to the end */
  uint32_t data_crc = crc.finalize(); 
  uint8_t* bytedata = (uint8_t*)&data_crc;
  for(uint32_t z=e_size+address;z<e_size+address+sizeof(data_crc);z++){
    EEPROM.write(z,*bytedata);
    bytedata++;
  } 
  EEPROM.commit();
  
}


/**************************************************************************************************
 *    Function      : eepwrite_notes
 *    Description   : writes the user notes 
 *    Input         : uint8_t* data, uint32_t size
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void eepwrite_notes(uint8_t* data, uint32_t size){
  for(uint32_t i=NOTES_START;i<512+NOTES_START;i++){
      EEPROM.write(i,0xFF);
  }
  EEPROM.commit();
  eepwrite_struct(data,size,NOTES_START);
}


/**************************************************************************************************
 *    Function      : eepread_notes
 *    Description   : reads the user notes 
 *    Input         : uint8_t* data, uint32_t size
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void eepread_notes(uint8_t* data, uint32_t size){
  if(size>501){
    size=501;
  }
  
  if( false == (eepread_struct( data, size, NOTES_START  ) ) ){
   Serial.println("Notes corrutp");
   bzero(data,size);
   eepwrite_notes(data,size);
  }
   
  return;
}

/**************************************************************************************************
 *    Function      : erase_eeprom
 *    Description   : writes the whole EEPROM with 0xFF  
 *    Input         : none
 *    Output        : none
 *    Remarks       : This will invalidate all user data 
 **************************************************************************************************/
void erase_eeprom( void ){
  
 
  for(uint32_t i=0;i<4096;i++){
     EEPROM.write(i,0xFF);
  }
  EEPROM.commit();
 
}
