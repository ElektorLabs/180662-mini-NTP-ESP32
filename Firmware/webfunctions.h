#ifndef WEBFUNCTION_H_
#define WEBFUNCTION_H_

/**************************************************************************************************
*    Function      : response_settings
*    Description   : none
*    Input         : none
*    Output        : none
*    Remarks       : none
**************************************************************************************************/     
void response_settings( void );

/**************************************************************************************************
*    Function      : timezone_update
*    Description   : none
*    Input         : none
*    Output        : none
*    Remarks       : none
**************************************************************************************************/ 
void timezone_update( void );

/**************************************************************************************************
*    Function      : timezone_overrides_update
*    Description   : none
*    Input         : none
*    Output        : none
*    Remarks       : none
**************************************************************************************************/ 
void timezone_overrides_update( void );

/**************************************************************************************************
*    Function      : settime_update
*    Description   : none
*    Input         : none
*    Output        : none
*    Remarks       : none
**************************************************************************************************/ 
void settime_update( void );

/**************************************************************************************************
*    Function      : update_ledactivespan
*    Description   : none
*    Input         : none
*    Output        : none
*    Remarks       : none
**************************************************************************************************/ 
void update_ledactivespan( void );

/**************************************************************************************************
*    Function      : update_notes
*    Description   : none
*    Input         : none
*    Output        : none
*    Remarks       : none
**************************************************************************************************/ 
void update_notes();

/**************************************************************************************************
*    Function      : read_notes
*    Description   : none
*    Input         : none
*    Output        : none
*    Remarks       : none
**************************************************************************************************/ 
void read_notes();

/**************************************************************************************************
*    Function      : update_gps_syncclock
*    Description   : set or unset form web is gps will be used to sync clock
*    Input         : none
*    Output        : none
*    Remarks       : none
**************************************************************************************************/
void update_gps_syncclock( void );

/**************************************************************************************************
*    Function      : update_display_settings
*    Description   : set or unset form web if the display will be swapped
*    Input         : none
*    Output        : none
*    Remarks       : none
**************************************************************************************************/
void update_display_settings( void );

/**************************************************************************************************
*    Function      : send_display_settings
*    Description   : set or unset form web if the display will be swapped
*    Input         : none
*    Output        : none
*    Remarks       : none
**************************************************************************************************/
void send_display_settings( void );

/**************************************************************************************************
*    Function      : getGPS_Location
*    Description   : none
*    Input         : none
*    Output        : none
*    Remarks       : Retunrs the GPS location
**************************************************************************************************/ 
void getGPS_Location( void );

/**************************************************************************************************
*    Function      : getipv4settings_settings
*    Description   : Sets the ipv4 settings via json 
*    Input         : non
*    Output        : none
*    Remarks       : none
**************************************************************************************************/ 
void update_ipv4_settings( void );

/**************************************************************************************************
*    Function      : getipv4settings_settings
*    Description   : Sends the ipv4 settings as json 
*    Input         : non
*    Output        : none
*    Remarks       : none
**************************************************************************************************/ 
void getipv4settings_settings( void );

#endif
