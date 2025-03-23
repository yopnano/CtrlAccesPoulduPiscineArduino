/*
 * File: DateTime_To_ITimeSource.h
 * Description:
 *   An Interface DateTime to MyNtpServer library.
 * Authors: Herledan Yann <yh@gmail.com>
 * License: New BSD License
 */

#ifndef DateTimeToITime_Source_H
#define DateTimeToITime_Source_H

#include <RTClib.h>
#include <ITimeSource.h>

class DateTime_To_ITimeSource : public MyITimeSource
{
private:
    DateTime &_dt;

public:
    DateTime_To_ITimeSource(DateTime &dt);
    // ~DateTime_To_ITimeSource();

    void now(uint32_t *secs, uint32_t *fract);

};

#endif