/*
 * File: DateTime_To_ITimeSource.cpp
 * Description:
 *   An Interface DateTime to NtpServer library.
 * Authors: Herledan Yann <yh@gmail.com>
 * License: New BSD License
 */

#include "DateTimeToITimeSource.h"

DateTime_To_ITimeSource::DateTime_To_ITimeSource(DateTime &dt)
    : _dt(dt)
{
    
}

void DateTime_To_ITimeSource::now(uint32_t *secs, uint32_t *fract)
{
    const uint32_t seventyYears = 2208988800UL;    // to convert Unix time to NTP 
    uint32_t ts = _dt.unixtime() + seventyYears;
    uint32_t f = 0;

    *secs = ts;
    *fract = f;
}

// DateTime_To_ITimeSource::~DateTime_To_ITimeSource()
// {
// }