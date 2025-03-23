#include "RTC.h"

bool rtcFail = false;
RTC_DS3231 Rtc;
DateTime UTC;
DateTime Local;

TimeChangeRule myDST = {"ETE", Last, Sun, Mar, 2, +120};
TimeChangeRule mySTD = {"HIVER", Last, Sun, Oct, 2, +60};
Timezone myTZ(myDST, mySTD);
TimeChangeRule *tcr;

void initializeRtc()
{
    if (!Rtc.begin())
    {
        DEBUGLN(F("Couldn't find RTC"));
        rtcFail = true;
    }

    if (Rtc.lostPower())
    {
        rtcFail = true;
        DEBUGLN(F("RTC is NOT initialized, let's set the time!"));
        Rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    UTC = Rtc.now();
    setTime(UTC.hour(),
            UTC.minute(),
            UTC.second(),
            UTC.day(),
            UTC.month(),
            UTC.year());
    
    DEBUG("RTC is initialized, value : ");
    DEBUGLN(UTC.timestamp(UTC.TIMESTAMP_FULL));

}

void synchronizeLocalTime()
{
    time_t utc = now();
    Local = myTZ.toLocal(utc, &tcr);
}
