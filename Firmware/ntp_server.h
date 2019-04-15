#include "Arduino.h"

class NTP_Server {
    
public:
    NTP_Server( );
    ~NTP_Server();
    
    bool begin(uint16_t port , uint32_t(*fnc_getutc_time)(void) );
      
};
