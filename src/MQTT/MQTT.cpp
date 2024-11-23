#include "MQTT.h"

#include <Ether\Ethernet.h>
#include "AccessControl\AccessControl.h"
#include "RTC\RTC.h"
#include "Topic.h"
#include "Utils.h"
#include "Debug.hpp"

const char *MQTT_USERNAME = "ctrlAccess";
const char *MQTT_PASSWORD = "Copoul29360";
const char *MQTT_SERVER = "10.42.0.1";
const int MQTT_PORT = 1883;

const unsigned short TpsDefCom = 5U;
unsigned short TempoCom = 0U;
char Life[2] = "0";
char LifeOld[2] = "0";
bool DefCom = false;

PubSubClient MqttClient(EthClient);

void reconnectToMqttBroker()
{
    if (!MqttClient.connected())
    {
        DEBUGLN(F("DEBUG > Tentative de connexion MQTT..."));

        // Tenter la connexion
        if (MqttClient.connect("CtrlAcces_Arduino", MQTT_USERNAME, MQTT_PASSWORD))
        {
            DEBUGLN(F("INFO > Connecté au serveur MQTT"));

            // Liste des topics à souscrire
            const char *topics[] PROGMEM = {ST_Deverrouillage,
                                            ST_Buzzer,
                                            ST_Led,
                                            ST_Param,
                                            ST_Life,
                                            ST_Rtc};

            DEBUGLN(F("INFO > Topics souscrits : "));

            // Boucle pour souscrire aux topics
            for (const char *topic : topics)
            {
                if (MqttClient.subscribe(topic))
                    DEBUG(F("INFO >   - "));
                else
                    DEBUG(F("WARNING > Échec de la souscription au topic : "));

                DEBUGLN(topic); // Afficher le nom du topic
            }

            // Indiquer le succès de la connexion via un topic dédié
            MqttClient.publish(PT_Online, "");
        }
        else
        {
            DEBUGLN(F("ERROR > Échec de la connexion MQTT."));
            DEBUG(F("ERROR >  Code d'erreur : "));
            DEBUGLN(MqttClient.state());

            // Gestion alternative en cas d'erreur (si nécessaire)
            // Exemple : Tentatives de reconnexion ou autre logique d'attente
        }
    }
}

void processMqtt(const bool ft10s)
{
    if (MqttClient.connected())
    {
        MqttClient.loop();
        {
        }
    }
    else if (ft10s)
    {
        reconnectToMqttBroker(); // Tenter de se reconnecter au serveur MQTT
    }
}

void onMqttMessageReceived(char *topic, byte *payload, unsigned int length)
{
    // Limiter la taille maximale du message pour éviter les débordements
    const size_t maxMessageLength = 50;
    char message[maxMessageLength] = {0};

    // Copier le contenu du payload dans le tableau `message`
    if (length < maxMessageLength - 1)
    {
        memcpy(message, payload, length);
        message[length] = '\0'; // Terminer la chaîne avec '\0'
    }
    else
    {
        DEBUGLN(F("WARNING > Message MQTT trop long, tronqué."));
        memcpy(message, payload, maxMessageLength - 1);
        message[maxMessageLength - 1] = '\0';
    }

    // Convertir en booléen si le message est "0" ou "1"
    bool state = (strcmp(message, "1") == 0);

    // Liste des topics souscrits
    const char *topics[] PROGMEM = {
        ST_Deverrouillage,
        ST_Buzzer,
        ST_Led,
        ST_Param,
        ST_Life,
        ST_Rtc};

    // Recherche de l'index correspondant au topic reçu
    int topicIndex = -1;
    for (size_t i = 0; i < sizeof(topics) / sizeof(topics[0]); ++i)
    {
        if (strcmp(topic, topics[i]) == 0)
        {
            topicIndex = i;
            break;
        }
    }

    // Gestion du bit de vie
    if (topicIndex == 4)
    {
        Bool_To_Char(!state, Life); // Inversion du bit de vie
        return;                     // Quitte la fonction
    }

    // Gérer le message reçu en fonction du topic
    switch (topicIndex)
    {
    case 0: // ST_Deverrouillage
    {
        unsigned int time = atoi(message);
        if (time > 0)
        {
            DEBUG(F("INFO > Déverrouillage de "));
            DEBUG(time);
            DEBUGLN(F(" secondes par MQTT"));
            OpenDelayCode = time;
            TempoOuverture = OpenDelayCode;
        }
        break;
    }
    case 1: // ST_Buzzer
        DEBUG(F("VERBOSE > Buzzer "));
        DEBUGLN((state ? F("ON") : F("OFF")));
        digitalWrite(PIN_BUZZER_KEYPAD, !state);
        break;

    case 2: // ST_Led
        DEBUG(F("VERBOSE > Led "));
        DEBUGLN((state ? F("Vert") : F("Rouge")));
        digitalWrite(PIN_LED_KEYPAD, !state);
        break;

    case 3: // ST_Param
        parseParam(message);
        break;

    case 5: // ST_Rtc
        DEBUG(F("INFO > Réglage du Rtc à "));
        DEBUGLN(message);
        Rtc.adjust(DateTime(message));
        rtcFail = false;
        break;

    default:
        DEBUG(F("warning > Topic inconnu :"));
        // DEBUGLN(topic);

        // Affichage du message en debug
        DEBUG(F("INFO > Message Reçu ["));
        DEBUG(topic);
        DEBUG(F("] : "));
        DEBUGLN(message);

        break;
    }
}

void sendHeartbeat(const bool ft1s)
{
    if (ft1s)
    {
        // Gestion du défaut de communication
        if (TempoCom < TpsDefCom)
            TempoCom++;

        if (Life != LifeOld)
            TempoCom = 0;

        DefCom = TempoCom >= TpsDefCom;

        // Gestion du mot de vie
        MqttClient.publish(PT_Life, Life);
        strncpy(LifeOld, Life, 2);
    }

    // if (DefCom)
    // Ouvrir le portail sur la page horaire de secours pour tous les badges
    // Activer les codes admin de secours
    // Générer un code provisoire ou code = Date du jour AMSAMS
}
