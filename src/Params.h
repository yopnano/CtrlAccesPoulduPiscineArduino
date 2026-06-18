#ifndef Params_h
#define Params_h

#include <BeOutil.h>

// --- Paramètres fixes ---
const unsigned int TPS_DEFAUT_COM = 10;                  ///< Temps avant défaut de communication série
const char* BADGES_SECOURS[] = {"23B0991C"};             ///< Badges de déverrouillage d'urgence.
const char* CODES_SECOURS[] = {"454129", "363844"};      ///< Code de déverrouillage d'urgence.

// --- Paramètres modifiables ---
unsigned int TpsOuvertureEntree = 5;        ///< Tempo de déverrouillage par badge ou code en secondes.
unsigned int TpsOuvertureSortie = 10;      ///< Tempo de déverrouillage par bouton en secondes.

// --- Temporisations BeOutils ---
TON TON_RelaisPortail;                      ///< Tempo de maintien du relais portail.
TOF TOF_RelaisOndule(30 * TIME::Secondes);  ///< Tempo de maintien du relais ondulé.

// --- Variables globales pour la communication Série ---
char bufferReception[64];                  // Tampon léger pour lire les ordres de Node-RED
byte indexReception = 0;
unsigned int CompteurDefautCom = 0;
bool DefCom = false;
bool DefComOld = false;

// --- Variables pour le Timeout Raspberry ---
char codeEnAttente[24] = "";         // Stocke le badge lu pendant l'attente
enum EtatControle {
  IDLE,               // Repos
  WAIT_NATIVE_BEEP,   // Attente 800ms (bip natif du clavier)
  BEEP_CONFIRM,       // Bip 50ms de l'Arduino
  WAIT_RASPBERRY,     // Attente 2s max (Validation Raspberry)
  WAIT_BEFORE_LOCAL   // Attente 200ms (Avant test local)
};
EtatControle etatCtrl = IDLE;
unsigned long chronoCtrl = 0;

#endif