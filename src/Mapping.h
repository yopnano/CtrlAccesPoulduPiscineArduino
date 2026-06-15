#ifndef Mapping_h
#define Mapping_h

#include <avr/wdt.h> // Watchdog matériel
#include <BeOutil.h>
#include <ClavierRFID.h>
#include "Params.h"

// --- Inputs (Entrées) ---
#define PIN_D0_KEYPAD 2         ///< Broche pour l'entrée Wiegand D0 (fil vert).
#define PIN_D1_KEYPAD 3         ///< Broche pour l'entrée Wiegand D1 (fil blanc).
#define PIN_EXIT_BUTTON 7       ///< Broche pour le bouton de sortie.
#define PIN_5VSB_RASPBERRY A5   ///< Broche pour le retour d'alimentation raspberry pi.

// --- Outputs (Sorties) ---
#define PIN_BUZZER_KEYPAD 5     ///< Broche pour le buzzer du clavier.
#define PIN_LED_KEYPAD 6        ///< Broche pour la LED du clavier.
#define PIN_LOCK_RELAY 8        ///< Broche pour le relais de la ventouse.
#define PIN_UPS_RELAY 9         ///< Broche pour le relais de l'ups.


// --- INSTANCES BE OUTILS ---
LioTorIn BpSortie(PIN_EXIT_BUTTON, INPUT_PULLUP, true);
Trigger TrigBpSortie;

LioTorOut BuzzerClavier(PIN_BUZZER_KEYPAD, true);
ledTor LedClavier(PIN_LED_KEYPAD, true);

relayTor RelaisPorte(PIN_LOCK_RELAY, true);
relayTor RelaisOndule(PIN_UPS_RELAY, true);

ClavierRFID LecteurEntree;


// --- Prototypes des fonctions (définies dans main.cpp) ---
void EcouterNodeRED();
void ExecuterOrdre(char* ordre);
void GestionBitVie();
void GestionControleAcces();
void GestionBoutonSortie();
void GestionVentousePortail();
void Deverrouillage(const unsigned int time_s);

// --- Fonctions de démarrage globales ---
void Modsetup()
{
    BuzzerClavier.turnOff();
    LedClavier.turnOff();

    // Initialisation port Série universel pour Node-RED
    Serial.begin(9600);
    
    #ifdef UsingLib_Serial
        ProjectInfos();
    #endif
    
    // Initialisation Matérielle
    LecteurEntree.begin(PIN_D0_KEYPAD, PIN_D1_KEYPAD);

    Serial.println(F("LOG:Arduino Mega demarre. Prets pour Node-RED."));
    
    // Activation du chien de garde (2 secondes max de freeze toléré)
    wdt_enable(WDTO_2S); 
}

void ModMain()
{
    sys.main();
}

#endif