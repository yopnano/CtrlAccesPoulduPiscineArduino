#ifndef ACCESS_CONTROL_H
#define ACCESS_CONTROL_H

#pragma once

#include <Wiegand.h>
class DateTime; // Déclaration avancée
class Tempo;    // Déclaration avancée

// Inputs (Entrées)
#define PIN_D0_KEYPAD 2   ///< Broche pour l'entrée Wiegand D0 (fil vert).
#define PIN_D1_KEYPAD 3   ///< Broche pour l'entrée Wiegand D1 (fil blanc).
#define PIN_EXIT_BUTTON 7 ///< Broche pour le bouton de sortie.
#define PIN_5VSB_RASPBERRY A5 ///< Broche pour le retour d'alimentation raspberry pi.

// Outputs (Sorties)
#define PIN_BUZZER_KEYPAD 5 ///< Broche pour le buzzer du clavier.
#define PIN_LED_KEYPAD 6    ///< Broche pour la LED du clavier.
#define PIN_LOCK_RELAY 8   ///< Broche pour le relais de la ventouse.
#define PIN_UPS_RELAY 9    ///< Broche pour le relais de l'ups.

// Parameters (Paramètres)
extern const char EMERGENCY_BADGES[][10]; ///< Badges de déverrouillage d'urgence.
extern const char EMERGENCY_CODE[];       ///< Code de déverrouillage d'urgence.
extern char BackupCode[];                 ///< Code de secours pour déverrouillage (format YMYM).
extern unsigned int OpenDelayCode;        ///< Temps de déverrouillage par badge ou code en secondes.
extern unsigned int OpenDelayButton;      ///< Temps de déverrouillage par bouton en secondes.
extern unsigned int TempoOuverture;       ///< Compteur temporaire pour contrôler la durée d'ouverture.
extern unsigned int TempoRelaisOnduleur;  ///< Tempo de maintien du relais onduleur
extern DateTime DebutAcces;               ///< Date et heure de début de l'accès autorisé au format yyyy, MM, dd, hh, mm, ss.
extern DateTime FinAcces;                 ///< Date et heure de fin de l'accès autorisé au format yyyy, MM, dd, hh, mm, ss.

// Variable stuff (Variables diverses)
#define PINLIMIT 2000            ///< Limite de délai entre deux saisies de code PIN (en millisecondes).
#define RFIDLIMIT 2000           ///< Limite de délai entre deux lectures de badge RFID (en millisecondes).
extern const int PINTIMEOUT;     ///< Temps limite pour saisir un code PIN (en millisecondes).
extern unsigned long rfidcount;  ///< Compteur des lectures RFID.
extern unsigned long pincount;   ///< Compteur des saisies de code PIN.
extern unsigned long lastKey;    ///< Temps du dernier appui sur une touche.
extern unsigned long lastPin;    ///< Temps de la dernière saisie de code PIN.
extern unsigned long lastRfid;   ///< Temps de la dernière lecture RFID.
extern bool LastExitButtonState; ///< État précédent du bouton de sortie.
extern String Saisie;            ///< Chaîne contenant la saisie courante sur le clavier.

// Wiegand Instance
extern WIEGAND Wg; ///< Instance pour gérer les communications Wiegand (RFID & clavier).

/**
 * @brief Vérifie si l'accès est valide en fonction des plages horaires et des dates configurées.
 * @return true si la date et l'heure actuelles sont dans les plages autorisées, false sinon.
 */
bool isAccessTimeValid();

/**
 * @brief Génère un code de secours basé sur l'année et le mois actuels.
 * Le code est enregistré dans la variable globale BackupCode.
 */
void generateBackupCode();

/**
 * @brief Analyse une date sous forme de chaîne (format "DD/MM") et met à jour un objet DateTime.
 * @param date La date sous forme de chaîne (par exemple "15/06").
 * @param dt L'objet DateTime à mettre à jour avec les informations de la date.
 */
void parseDate(const char *date, DateTime &dt);

/**
 * @brief Analyse une heure sous forme de chaîne (format "HH:MM") et met à jour un objet DateTime.
 * @param time L'heure sous forme de chaîne (par exemple "09:00").
 * @param dt L'objet DateTime à mettre à jour avec les informations de l'heure.
 */
void parseTime(const char *time, DateTime &dt);

/**
 * @brief Analyse une trame de paramètre sous forme clé=valeur et met à jour les variables correspondantes.
 * @param param La trame contenant les paramètres (par exemple "temps_ouverture/entree=5").
 */
void parseParam(const char *param);

/**
 * @brief Efface la saisie, du clavier Wiegand
 */
void clearSaisie();

/**
 * @brief Gère les entrées utilisateur provenant du clavier et des badges RFID.
 * Traite les délais, les codes d'urgence, et publie les événements via MQTT.
 */
void handleKeypadAndRfidInput();

/**
 * @brief Surveille l'état du bouton de sortie et déclenche les actions associées.
 * Comme le déverrouillage du portail lorsqu'il est pressé.
 */
void monitorExitButton();

/**
 * @brief Contrôle l'état du verrou de la porte en fonction du délai d'ouverture.
 * @param ft1s Flag indiquant si une seconde s'est écoulée.
 */
void controlDoorLock(const bool ft1s);

#endif