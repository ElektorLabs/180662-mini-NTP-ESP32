#ifndef DATASTORE_H_
 #define DATASTORE_H_
 
#include "timecore.h"

typedef struct {
  char ssid[128];
  char pass[128];
} credentials_t;
/* 256 byte */

typedef struct{
  bool sync_on_gps;
}gps_settings_t;

typedef struct {
  bool swap_display;
} display_settings_t;

/**************************************************************************************************
 *    Function      : datastoresetup
 *    Description   : Gets the EEPROM Emulation set up
 *    Input         : none 
 *    Output        : none
 *    Remarks       : We use 4096 byte for EEPROM 
 **************************************************************************************************/
void datastoresetup();


/**************************************************************************************************
 *    Function      : write_display_config
 *    Description   : writes the display config
 *    Input         : display_settings_t
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void write_display_config(display_settings_t c);

/**************************************************************************************************
 *    Function      : read_display_config
 *    Description   : reads the display config
 *    Input         : display_settings_t
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
display_settings_t read_display_config( void );

/**************************************************************************************************
 *    Function      : write_gps_config
 *    Description   : writes the gps config
 *    Input         : gps_settings_t
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void write_gps_config(gps_settings_t c);

/**************************************************************************************************
 *    Function      : read_gps_config
 *    Description   : reads the gops config
 *    Input         : gps_settings_t
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
gps_settings_t read_gps_config( void );

/**************************************************************************************************
 *    Function      : write_timecoreconf
 *    Description   : writes the time core config
 *    Input         : timecoreconf_t
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void write_timecoreconf(timecoreconf_t c);


/**************************************************************************************************
 *    Function      : read_timecoreconf
 *    Description   : reads the time core config
 *    Input         : none
 *    Output        : timecoreconf_t
 *    Remarks       : none
 **************************************************************************************************/
timecoreconf_t read_timecoreconf( void );


/**************************************************************************************************
 *    Function      : write_credentials
 *    Description   : writes the wifi credentials
 *    Input         : credentials_t
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void write_credentials(credentials_t c);


/**************************************************************************************************
 *    Function      : read_credentials
 *    Description   : reads the wifi credentials
 *    Input         : none
 *    Output        : credentials_t
 *    Remarks       : none
 **************************************************************************************************/
credentials_t read_credentials( void );

/**************************************************************************************************
 *    Function      : eepwrite_notes
 *    Description   : writes the user notes 
 *    Input         : uint8_t* data, uint32_t size
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void eepwrite_notes(uint8_t* data, uint32_t size);

/**************************************************************************************************
 *    Function      : eepread_notes
 *    Description   : reads the user notes 
 *    Input         : uint8_t* data, uint32_t size
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void eepread_notes(uint8_t* data, uint32_t size);

/**************************************************************************************************
 *    Function      : erase_eeprom
 *    Description   : writes the whole EEPROM with 0xFF  
 *    Input         : none
 *    Output        : none
 *    Remarks       : This will invalidate all user data 
 **************************************************************************************************/
void erase_eeprom( void );


#endif
