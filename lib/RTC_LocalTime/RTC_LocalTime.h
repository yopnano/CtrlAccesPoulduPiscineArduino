#ifndef RTC_LOCALTIME_H
#define RTC_LOCALTIME_H

#include <Arduino.h>
#include <RTClib.h>
#include <Timezone.h>
#include "SerialDebug.h" // Optionnel, pour le logging

/**
 * @class RTC_LocalTime
 * @brief Gestion du RTC matériel avec prise en charge de l'heure locale.
 *
 * Cette classe encapsule un module RTC (DS3231, DS1307, etc.) et utilise
 * la librairie Timezone pour fournir une heure locale tenant compte de l'heure
 * d'été/hiver.
 */
class RTC_LocalTime
{
public:
    /**
     * @brief Constructeur.
     * @param logger Optionnel, pour activer les logs via SerialDebug.
     */
    RTC_LocalTime(SerialDebug *logger = nullptr)
        : rtc(), tz(nullptr), debug(logger)  {}

    /**
     * @brief Initialise le RTC et configure le fuseau horaire.
     *
     * @param tzone Référence vers un objet Timezone (ex: Europe/Paris).
     * @return true si l'initialisation réussit, false sinon.
     */
    bool begin(Timezone &tzone)
    {
        tz = &tzone;
        if (!rtc.begin())
        {
            log(LogLevel::Error, F("RTC non détecté !"));
            return false;
        }

        if (rtc.lostPower())
        {
            log(LogLevel::Warning, F("RTC désynchronisé vérifier la pile bouton"));

            String msg = F("RTC initialisé à la date de compilation, soit le : ");
            msg += (__DATE__);
            msg += "à";
            msg += F(__TIME__);
            
            log(LogLevel::Warning, msg);

            // On initialise le RTC avec la date/heure de compilation
            rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        }

        log(LogLevel::Info, F("RTC initialisé avec succès"));
        return true;
    }

    /**
     * @brief Retourne l'heure UTC actuelle du RTC.
     * @return DateTime en UTC.
     */
    DateTime nowUTC()
    {
        return rtc.now();
    }

    /**
     * @brief Retourne l'heure locale (RTC stocké en UTC).
     */
    DateTime nowLocal()
    {
        if (!tz) {
            log(LogLevel::Error, F("Fuseau horaire non configuré"));
            return rtc.now();
        }

        DateTime utcDT = rtc.now();                    // UTC depuis le RTC
        time_t utc = static_cast<time_t>(utcDT.unixtime());
        time_t local = tz->toLocal(utc);               // conversion vers local
        return DateTime(static_cast<uint32_t>(local)); // retour en DateTime
    }

    /**
     * @brief Définit l'heure du RTC depuis une heure locale.
     *        (convertit d'abord local -> UTC, puis ajuste le RTC)
     */
    void setLocalDateTime(const DateTime &localDT)
    {
        if (!tz) {
            log(LogLevel::Warning, F("TZ non configuré, ajustement direct (supposé UTC)"));
            rtc.adjust(localDT);
            return;
        }
        time_t local = static_cast<time_t>(localDT.unixtime());
        time_t utc   = tz->toUTC(local);                     // conversion vers UTC
        
        log(LogLevel::Info, F("Heure locale mis à jour manuellement"));
        rtc.adjust(DateTime(static_cast<uint32_t>(utc)));    // on stocke l'UTC dans le RTC
    }

    /**
     * @brief Définit manuellement la date/heure du RTC (UTC).
     * @param dt Objet DateTime (UTC).
     */
    void setDateTime(const DateTime &dt)
    {
        rtc.adjust(dt);
        log(LogLevel::Info, F("RTC mis à jour manuellement"));
    }

private:
    RTC_DS3231 rtc;     ///< Instance du module RTC
    Timezone *tz;       ///< Fuseau horaire local
    SerialDebug *debug; ///< Logger optionnel

    void log(LogLevel level, const String &msg)
    {
        if (debug)
        {
            debug->log(level, msg);
        }
    }
};

#endif
