#ifndef NTP_S_H
#define NTP_S_H

#pragma once

#include <NtpServer.h>
#include <DateTimeToITimeSource.h>
#include "RTC\RTC.h"

extern DateTime_To_ITimeSource source;
extern NtpServer Ntp;

/**
 * @brief Gère les requêtes NTP entrantes pour synchroniser l'heure.
 *
 * Cette fonction vérifie si une requête NTP a été reçue et la traite. Si une requête est détectée,
 * elle affiche l'adresse IP du client ayant initié la requête (si la sortie de débogage est activée).
 */
void handleNtpRequests();

#endif