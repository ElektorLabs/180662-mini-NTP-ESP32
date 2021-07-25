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
#ifndef TIMECORE_H_
 #define TIMECORE_H_
 
 /* 
    This is a generic Time core that can be feeded by 
    differnt timesources in polled or event driven modes
    The time is internally handeld as UTC time and converted
    to the appropriate locale time
 */
 
 
#include "Arduino.h"
#include <TimeLib.h>
#include "timezone_enums.h"


typedef struct{
  TIMEZONES_NAMES_t TimeZone;
  DLTS_OFFSET_t DLTS_OffsetIDX;
  bool ManualDLSEna;
  bool AutomaticDLTS_Ena;
  bool TimeZoneOverride;
  int32_t GMTOffset; 
}timecoreconf_t;

/* the higher the id the higher the priority */
typedef enum {
    NO_RTC = 0,
    RTC_CLOCK,
    NTP_CLOCK,
    GPS_CLOCK,
    USER_DEFINED,
    RTC_SRC_CNT
} source_t  ;  

/* The RTX Soruce must provice a unix timestamp in GMT*/
typedef struct {
   source_t type;
   void (*SecondTick)(void);
   void (*WriteTime)(uint32_t);
   uint32_t (*ReadTime)(bool* delayed_result);   
} rtc_source_t;


typedef struct {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t dow;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} datum_t;

typedef enum {
    OnSecondChanged=0,
    OnMinuteChanged,
    OnHourChanged,
    OnDayOfWeekChaned,
    OnDayChanged,
    OnMonthChanged,
    OnYearChanged,
    RTC_EVENT_CNT
} rtc_cb_t;

typedef enum {
 no_zone=0
} TimeZone_t;

class Timecore {
    
    public:
    /**************************************************************************************************
     *    Function      : Constructor
     *    Class         : Timecore
     *    Description   : none
     *    Input         : none
     *    Output        : none
     *    Remarks       : none
     **************************************************************************************************/
    Timecore();

    /**************************************************************************************************
     *    Function      : Destructor
     *    Class         : Timecore
     *    Description   : none
     *    Input         : none
     *    Output        : none
     *    Remarks       : none
     **************************************************************************************************/
    ~Timecore();

    /**************************************************************************************************
     *    Function      : SetUTC
     *    Class         : Timecore
     *    Description   : Sets the UTC Time
     *    Input         : uint32_t time, source_t source
     *    Output        : none
     *    Remarks       : Only sets the UTC Time if the source is equal or better than the last one
     **************************************************************************************************/
    void SetUTC( uint32_t time, source_t source );
    
    /**************************************************************************************************
     *    Function      : SetUTC
     *    Class         : Timecore
     *    Description   : Sets the UTC Time
     *    Input         : datum_t time, source_t source
     *    Output        : none
     *    Remarks       : Only sets the UTC Time if the source is equal or better than the last one
     **************************************************************************************************/
    void SetUTC( datum_t time, source_t source  );

    /**************************************************************************************************
     *    Function      : GetUTC
     *    Class         : Timecore
     *    Description   : Gets the UTC Time
     *    Input         : none
     *    Output        : uint32_t ( seconds since 1.1.1970)
     *    Remarks       : This is a unix timestamp
     **************************************************************************************************/
    uint32_t GetUTC( void );

    /**************************************************************************************************
     *    Function      : GetLocalTime
     *    Class         : Timecore
     *    Description   : Gets the UTC Time
     *    Input         : none
     *    Output        : time_t
     *    Remarks       : Returns the local time according to the settings
     **************************************************************************************************/
    time_t GetLocalTime( void );

    
    /**************************************************************************************************
     *    Function      : GetLocalTimeDate
     *    Class         : Timecore
     *    Description   : Gets the UTC Time
     *    Input         : none
     *    Output        : datum_t
     *    Remarks       : Returns the local time according to the settings
     **************************************************************************************************/
    datum_t GetLocalTimeDate( void );

    /**************************************************************************************************
     *    Function      : SetLocalTime
     *    Class         : Timecore
     *    Description   : Sets the Time based on the local settings
     *    Input         : none
     *    Output        : datum_t
     *    Remarks       : Will convert the time back to UTC
     **************************************************************************************************/  
    void SetLocalTime( datum_t d);

    /**************************************************************************************************
     *    Function      : ConvertToDatum
     *    Class         : Timecore
     *    Description   : Sets the Time based on the local settings
     *    Input         : timestamp ( uinixtime )
     *    Output        : datum_t
     *    Remarks       : Will convert the timestamp to a stuct with hours, minutes, seconds ......
     **************************************************************************************************/ 
    datum_t ConvertToDatum( uint32_t timestamp);

    /**************************************************************************************************
     *    Function      : TimeStructToTimeStamp
     *    Class         : Timecore
     *    Description   : Helperfunction to get a unixtimestam from a datum_t
     *    Input         : datum_t
     *    Output        : uint32_t
     *    Remarks       : none
     **************************************************************************************************/
      uint32_t TimeStructToTimeStamp(datum_t time);
  
     /**************************************************************************************************
     *    Function      : SetTimeZone
     *    Class         : Timecore
     *    Description   : Sets the Timezone we are in 
     *    Input         : TIMEZONES_NAMES_t
     *    Output        : none
     *    Remarks       : See timezone_enums for valid values
     **************************************************************************************************/    
    void SetTimeZone(TIMEZONES_NAMES_t Zone );

    /**************************************************************************************************
     *    Function      : GetTimeZone
     *    Class         : Timecore
     *    Description   : Gets the Timezone we are in 
     *    Input         : none
     *    Output        : TIMEZONES_NAMES_t
     *    Remarks       : See timezone_enums for valid values
     **************************************************************************************************/   
    TIMEZONES_NAMES_t GetTimeZone( void );

    /**************************************************************************************************
     *    Function      : SetDLS_Offset
     *    Class         : Timecore
     *    Description   : Sets the DLST Offset
     *    Input         : DLTS_OFFSET_t
     *    Output        : none
     *    Remarks       : See timezone_enums for valid values
     **************************************************************************************************/  
    void SetDLS_Offset(DLTS_OFFSET_t offset );

    /**************************************************************************************************
     *    Function      : GetDLS_Offset
     *    Class         : Timecore
     *    Description   : Sets the DLST Offset
     *    Input         : none
     *    Output        : DLTS_OFFSET_t
     *    Remarks       : See timezone_enums for valid values
     **************************************************************************************************/  
    DLTS_OFFSET_t GetDLS_Offset( void );

    /**************************************************************************************************
     *    Function      : SetGMT_Offset
     *    Class         : Timecore
     *    Description   : Sets the DLST Offset
     *    Input         : int32_t
     *    Output        : none
     *    Remarks       : See the manual GMT offset in mintes
     **************************************************************************************************/  
    void SetGMT_Offset(int32_t offset );

    
    /**************************************************************************************************
     *    Function      : GetGMT_Offset
     *    Class         : Timecore
     *    Description   : Gets the DLST Offset
     *    Input         : none
     *    Output        : int32_t
     *    Remarks       : See the manual GMT offset in mintes
     **************************************************************************************************/
    int32_t GetGMT_Offset( void );

    /**************************************************************************************************
     *    Function      : SetAutomaticDLS
     *    Class         : Timecore
     *    Description   : Sets the DLST Mode
     *    Input         : bool
     *    Output        : none
     *    Remarks       : none
     **************************************************************************************************/
    void SetAutomaticDLS( bool ena);

    /**************************************************************************************************
     *    Function      : GetAutomacitDLS
     *    Class         : Timecore
     *    Description   : Gets the DLST Mode
     *    Input         : none
     *    Output        : bool
     *    Remarks       : none
     **************************************************************************************************/
    bool GetAutomacitDLS( void );

    /**************************************************************************************************
     *    Function      : GetDLSstatus
     *    Class         : Timecore
     *    Description   : Gets the current DLTS Status
     *    Input         : none
     *    Output        : bool
     *    Remarks       : returns true if DLTS is in use
     **************************************************************************************************/
    bool GetDLSstatus( void );

    /**************************************************************************************************
     *    Function      : GetManualDLSEna
     *    Class         : Timecore
     *    Description   : Gets the manual DLTS status
     *    Input         : none
     *    Output        : bool
     *    Remarks       : returns true if in use
     **************************************************************************************************/
    bool GetManualDLSEna( void );

    
    /**************************************************************************************************
     *    Function      : SetManualDLSEna
     *    Class         : Timecore
     *    Description   : Sets the manual DLTS status
     *    Input         : bool
     *    Output        : none
     *    Remarks       : none
     **************************************************************************************************/
    void SetManualDLSEna( bool ena);

    /**************************************************************************************************
     *    Function      : SetTimeZoneManual
     *    Class         : Timecore
     *    Description   : Sets the override for the timzone
     *    Input         : bool
     *    Output        : none
     *    Remarks       : none
     **************************************************************************************************/
    void SetTimeZoneManual( bool ena);
     
   /**************************************************************************************************
   *    Function      : GetZimeZoneManual
   *    Class         : Timecore
   *    Description   : Gets the override for the timzone
   *    Input         : bool
   *    Output        : none
   *    Remarks       : none
   **************************************************************************************************/
    bool GetTimeZoneManual( void );

   /**************************************************************************************************
   *    Function      : SetConfig
   *    Class         : Timecore
   *    Description   : Applys the passed config 
   *    Input         : timecoreconf_t conf
   *    Output        : none
   *    Remarks       : none
   **************************************************************************************************/
    void SetConfig(timecoreconf_t conf);

  /**************************************************************************************************
   *    Function      : GetConfig
   *    Class         : Timecore
   *    Description   : Gets the current config 
   *    Input         : none
   *    Output        : timecoreconf_t
   *    Remarks       : none
   **************************************************************************************************/
    timecoreconf_t GetConfig( void );

  /**************************************************************************************************
   *    Function      : GetDefaultConfig
   *    Class         : Timecore
   *    Description   : Gets the default config 
   *    Input         : none
   *    Output        : timecoreconf_t
   *    Remarks       : none
   **************************************************************************************************/
    static timecoreconf_t GetDefaultConfig( void );
  
  /**************************************************************************************************
   *    Function      : RegisterTimeSource
   *    Class         : Timecore
   *    Description   : Registers a new timesource
   *    Input         : rtc_source_t source
   *    Output        : none
   *    Remarks       : none
   **************************************************************************************************/   
    void RegisterTimeSource(rtc_source_t source); 

  /**************************************************************************************************
   *    Function      : SaveConfig
   *    Class         : Timecore
   *    Description   : Saves config to EEPROM / FLASH
   *    Input         : none
   *    Output        : none
   *    Remarks       : none
   **************************************************************************************************/   
    void SaveConfig( void );

  /**************************************************************************************************
   *    Function      : RTC_Tick
   *    Class         : Timecore
   *    Description   : Needs to be called once a second 
   *    Input         : none
   *    Output        : none
   *    Remarks       : Keeps internal time counter running
   **************************************************************************************************/  
    void RTC_Tick( void ); /* Needs to be called once a second */

  /**************************************************************************************************
   *    Function      : GetTimeZoneName
   *    Class         : Timecore
   *    Description   : This will return the name of the current Timezone
   *    Input         : none
   *    Output        : none
   *    Remarks       : none
   **************************************************************************************************/ 
    const char* GetTimeZoneName(TIMEZONES_NAMES_t Zone); 
    
    private:
        timecoreconf_t local_config; 
        timezone_t TimeZoneRam;
        uint8_t dstYear;
        time_t dstStart;  // Start of DST in specific Year (seconds since 1970)
        time_t dstEnd;    // End of DST in listed Year (seconds since 1970)
        uint32_t local_softrtc_timestamp=0;
        source_t CurrentMasterSource=NO_RTC; /* If this is set to none we run from the internal rtc */
        rtc_source_t TimeSources [RTC_SRC_CNT]; 
        void* rtc_event_callback[RTC_EVENT_CNT]={NULL,}; /* Holds the callbacks for the RTC events */
        uint16_t DegradeTimer_Src=3600;
        
      /**************************************************************************************************
       *    Function      : calcYear
       *    Class         : Timecore
       *    Description   : Helperfunction to calculate the year
       *    Input         : time_t
       *    Output        : uint8_t
       *    Remarks       : none
       **************************************************************************************************/ 
        uint8_t calcYear(time_t time);

      /**************************************************************************************************
       *    Function      : calcTime
       *    Class         : Timecore
       *    Description   : Helperfunction to calculate the DLST-Rule for the current year
       *    Input         : struct dstRule * tr
       *    Output        : time_t
       *    Remarks       : none
       **************************************************************************************************/ 
        time_t calcTime(struct dstRule * tr);

      /**************************************************************************************************
       *    Function      : my_mktime
       *    Class         : Timecore
       *    Description   : Helperfunction to calculate the time_t form a given time struct
       *    Input         : struct dstRule * tr
       *    Output        : time_t
       *    Remarks       : none
       **************************************************************************************************/ 
        time_t my_mktime(struct tm *tmptr);

      /**************************************************************************************************
       *    Function      : SetTimeZone
       *    Class         : Timecore
       *    Description   : Helperfunction to set the DLTS Rules for the curren TimeZone
       *    Input         : struct dstRule * tr
       *    Output        : time_t
       *    Remarks       : none
       **************************************************************************************************/        
        void SetTimeZone(struct dstRule startRule, struct dstRule endRule );



      /**************************************************************************************************
       *    Function      : LoadTimezone
       *    Class         : Timecore
       *    Description   : Helperfunction load the current timezone from FLASH to RAM
       *    Input         : uint16_t index
       *    Output        : none
       *    Remarks       : none
       **************************************************************************************************/ 
       void LoadTimezone( uint16_t index);
     
};

#endif
