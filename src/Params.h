#ifndef Params_h
#define Params_h

#include <BeOutil.h>
// #include <Timezone.h>

// Parameters (Paramètres)

byte MAC[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}; ///< Adresse MAC Ethernet.
IPAddress IP(10, 42, 0, 5);                        ///< Adresse IP Ethernet

const unsigned int TpsDefCom = 10;                        ///< Temps avant défaut de communication MQTT
const String BADGES_SECOURS[] = {"23B0991C"}; ///< Badges de déverrouillage d'urgence.
const String CODES_SECOURS[] = {"454129", "363844"};      ///< Code de déverrouillage d'urgence.

unsigned int TpsOuvertureEntree = 5;       ///< Tempo de déverrouillage par badge ou code en secondes.
unsigned int TpsOuvertureSortie = 10;      ///< Tempo de déverrouillage par bouton en secondes.
unsigned int ModeFctRelaisPortail = 3;      ///< Mode de fonctionnement portail 0:Fermé, 1:Ouvert, 3:Auto.
TON TON_RelaisPortail;                     ///< Tempo de maintien du relais portail.
TOF TOF_RelaisOndule(30 * TIME::Secondes); ///< Tempo de maintien du relais ondulé.

// Définition du fuseau pour la France (CET/CEST)
// TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, +120}; // heure d'été
// TimeChangeRule CET  = {"CET", Last, Sun, Oct, 3, +60};   // heure d'hiver
// Timezone tz(CEST, CET);

// DateTime DebutAcces = {2025, 6, 15, 9, 00, 00}; ///< Date et heure de début de l'accès autorisé au format yyyy, MM, dd, hh, mm, ss.
// DateTime FinAcces = {2025, 9, 15, 20, 00, 00};  ///< Date et heure de fin de l'accès autorisé au format yyyy, MM, dd, hh, mm, ss.

#endif