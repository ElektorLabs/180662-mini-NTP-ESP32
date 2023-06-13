#include "Arduino.h"
#include "AsyncUDP.h"

class NTP_Server {
    
public:
    NTP_Server( );
    ~NTP_Server();

    bool begin(uint16_t port , uint32_t(*fnc_getutc_time)(void) , uint32_t(*fnc_get_subsecond)(void) );
private:    
    static void processUDPPacket(AsyncUDPPacket& packet);
      
};
