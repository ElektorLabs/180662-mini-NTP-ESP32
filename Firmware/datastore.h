/*
    This file is part of Firmware for Elektorproject 180662.

    Firmware for Elektorproject 180662 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Firmware for Elektorproject 180662.  If not, see <https://www.gnu.org/licenses/>.

*/

#ifndef DATASTORE_H_
 #define DATASTORE_H_
 
#include "timecore.h"

typedef struct {
  char ssid[128];
  char pass[128];
} credentials_t;
/* 256 byte */

typedef struct {
  bool use_static;
  uint32_t address;
  uint32_t subnet;
  uint32_t gateway;
  uint32_t dns0;
  uint32_t dns1;
} ipv4_settings;


typedef struct{
  bool sync_on_gps;
// uint16_t baudrate;
  uint8_t rollover_cnt;
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
 *    Function      : write_credentials
 *    Description   : writes the wifi ipv4 settings
 *    Input         : ipv4_settings
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void write_ipv4_settings(ipv4_settings c);

/**************************************************************************************************
 *    Function      : read_credentials
 *    Description   : reads the ipv4 settings
 *    Input         : none
 *    Output        : ipv4_settings
 *    Remarks       : none
 **************************************************************************************************/
ipv4_settings read_ipv4_settings( void );

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
