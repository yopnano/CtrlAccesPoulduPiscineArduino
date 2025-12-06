#ifndef Mapping_h
#define Mapping_h

#include <BeOutil.h>
#include <Ethernet.h>

#include <ClavierRFID.h>
#include <SerialDebug.h>
#include <SerialShell.h>
#include <MQTTManager.h>
// #include <RTC_LocalTime.h>

#include "Params.h"

// Inputs (Entrées)
#define PIN_D0_KEYPAD 2       ///< Broche pour l'entrée Wiegand D0 (fil vert).
#define PIN_D1_KEYPAD 3       ///< Broche pour l'entrée Wiegand D1 (fil blanc).
#define PIN_EXIT_BUTTON 7     ///< Broche pour le bouton de sortie.
#define PIN_5VSB_RASPBERRY A5 ///< Broche pour le retour d'alimentation raspberry pi.

// Outputs (Sorties)
#define PIN_BUZZER_KEYPAD 5 ///< Broche pour le buzzer du clavier.
#define PIN_LED_KEYPAD 6    ///< Broche pour la LED du clavier.
#define PIN_LOCK_RELAY 8    ///< Broche pour le relais de la ventouse.
#define PIN_UPS_RELAY 9     ///< Broche pour le relais de l'ups.

// INSTANCES

// Bibliothèque BeOutils
LioTorIn BpSortie(PIN_EXIT_BUTTON, INPUT_PULLUP, true);
Trigger TrigBpSortie;

LioTorOut BuzzerClavier(PIN_BUZZER_KEYPAD, true);
ledTor LedClavier(PIN_LED_KEYPAD, true);

relayTor RelaisPorte(PIN_LOCK_RELAY, true);
relayTor RelaisOndule(PIN_UPS_RELAY, true);

// Bibliothèques projet
SerialDebug Debug;
SerialShell Shell;

// Classes projet
ClavierRFID LecteurEntree(&Debug);
MQTTManager Mqtt(&Debug);

// RTC_LocalTime RtcLocal(&Debug);
EthernetClient EthClient;

/**
 * Protoypes de fonction
 */

void MqqtSubscription();
void GestionBitVie();
void GestionControleAcces();
void GestionBoutonSortie();
void GestionVentousePortail();
bool ValiderCodeMQTT(const String code);
bool ValiderCodeLocal(const String code);
void Deverrouillage(const unsigned int);
void ParseParametres(const String &param);
// void ParseDateString(const String &dateStr, DateTime &dt);
// void ParseTimeString(const String &dateStr, DateTime &dt);


void ShellDefineAvailableCommands()
{

    // Commande InfoProjet
    Shell.addCommand("-i", "--info", F("Info du projet"), [](int argc, char *argv[])
                     { ProjectInfos(); });

    // Commande Déverrouillage
    Shell.addCommand("", "--ulock", F("Déverrouillage du portail"), [](int argc, char *argv[])
                     {
                         int time_s = 0;

                         if (argc == 0)
                            time_s = 10;
                        else
                            time_s = atoi(argv[0]);

                         Debug.concat(F("Ouverture du portail par le shell pendant :"));
                         Debug.concat(time_s);
                         Debug.log(LogLevel::Info, F("secondes")); 
                        
                        
                         Deverrouillage(time_s);
                        });

    // Commande Mode de fonctionnement portail
    Shell.addCommand("", "--modefct", F("Mode de fonctionnement du portail"), [](int argc, char *argv[])
                     {
                        int modeFct = 3;

                        if (argc == 0)
                            modeFct = 3;
                        else
                            modeFct = atoi(argv[0]);

                        String modeStr;
                        switch (modeFct)
                        {
                        case 0:
                            modeStr = "Forcé Fermé";
                            break;
                        
                        case 1:
                            modeStr = "Forcé Ouvert";
                            break;                        
                        
                        default:
                            modeStr = "Automatique";
                            modeFct = 3;
                            break;
                        }

                        Debug.concat(F("Mode de fonctionnement du portail définit sur"));
                        Debug.log(LogLevel::Info, modeStr); 
                        ModeFctRelaisPortail = modeFct;
                        });
}

void Modsetup()
{
    BuzzerClavier.turnOff();
    LedClavier.turnOff();

#ifdef UsingLib_Serial
    Serial.begin(9600);
    ProjectInfos();
#endif

    // Initialisation du debug et du shell
    Debug.begin(Serial, LogLevel::Info);
    Shell.begin(Serial, &Debug);
    ShellDefineAvailableCommands();

    // Setup instances
    // for (auto &&iMoteur : moteur) iMoteur.setup();
    LecteurEntree.begin(PIN_D0_KEYPAD, PIN_D1_KEYPAD);

    // Gestion du RTC avec heure locale
    // RtcLocal.begin(tz);
    // DateTime localNow = RtcLocal.nowLocal();
    // Debug.log(LogLevel::Info, "Heure locale actuelle : " + String(localNow.timestamp().c_str()));

    // Ethernet
    Ethernet.begin(MAC, IP);
    Mqtt.begin(EthClient, "10.42.0.1", 1883, "CtrlAcces_Arduino", "ctrlAccess", "Copoul29360");
}

void ModMain()
{
    sys.main();
    Shell.update();

    // Main instances//for (auto &&iMoteur : moteur) iMoteur.main();
}

#endif