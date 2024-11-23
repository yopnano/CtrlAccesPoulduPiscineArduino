#ifndef RTC_H
#define RTC_H

#pragma once

#include <Wire.h>
#include <RTClib.h>
#include <Timezone.h>
#include "Debug.hpp"

extern bool rtcFail;   // If RTC lost power or cannot init()
extern RTC_DS3231 Rtc; // RTC object
extern DateTime UTC;   // Current date and time UTC
extern DateTime Local; // Current local date and time

extern TimeChangeRule myDST; // Daylight time = UTC + 2 hours
extern TimeChangeRule mySTD; // Standard time = UTC + 1 hour
extern Timezone myTZ;
extern TimeChangeRule *tcr; // pointer to the time change rule, use to get TZ abbrev

/**
 * @brief Initialise le module RTC (Real-Time Clock).
 * 
 * Cette fonction configure et vérifie l'état du module RTC. Si le RTC est absent ou a perdu son alimentation,
 * elle indique un échec et réinitialise l'heure en fonction de la date et l'heure de compilation.
 * Elle initialise également l'heure UTC à partir du module RTC.
 */
void initializeRtc();

/**
 * @brief Synchronise l'heure locale avec l'heure UTC.
 * 
 * Cette fonction convertit l'heure UTC actuelle en heure locale selon le fuseau horaire spécifié
 * et met à jour la variable globale représentant l'heure locale.
 */
void synchronizeLocalTime();


#endif
