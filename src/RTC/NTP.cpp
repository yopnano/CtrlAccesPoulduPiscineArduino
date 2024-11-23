#include "NTP.h"

DateTime_To_ITimeSource source(UTC);
NtpServer Ntp(source);

void handleNtpRequests()
{
  // Check if NTP request
  if (Ntp.processOneRequest())
  {
    DEBUG(F("NTP Request from : "));
    #ifdef DEBUG_OUT
      Ntp.getLastClientIP().printTo(DEBUG_OUT);
    #endif
    DEBUGLN(NULL);
  }
}