#ifndef MQTTMANAGER_H
#define MQTTMANAGER_H

#include <Arduino.h>
#include <PubSubClient.h>
#include "SerialDebug.h" // Optionnel, pour le logging

#define MAX_MQTT_TOPICS 10 // Nombre max de topics souscrits

typedef void (*MQTTCallback)(const String &payload);

struct MQTTTopicHandler
{
    const char *topic;
    MQTTCallback callback;
};

class MQTTManager
{
public:
    /**
     * @brief Constructeur par défaut de la classe MQTTManager.
     *
     * Initialise l’objet MQTTManager en mettant à zéro les pointeurs internes
     * et les buffers d’authentification. À ce stade, aucune connexion au broker
     * n’est encore configurée.
     *
     * - `mqttClient` est initialisé à nullptr.
     * - `debug` est initialisé à nullptr.
     * - `topicCount` est mis à 0 (aucun topic souscrit).
     * - `username` et `password` sont initialisés en chaînes vides.
     *
     * @note Après l’appel de ce constructeur, il est nécessaire d’appeler
     *       la méthode begin() pour configurer et établir une connexion MQTT.
     */
    MQTTManager(SerialDebug *logger = nullptr) : mqttClient(nullptr), debug(logger), topicCount(0)
    {
        username[0] = '\0';
        password[0] = '\0';
    }

    /**
     * @brief Initialise le gestionnaire MQTT avec les paramètres de connexion au broker.
     *
     * Cette fonction configure la connexion au broker MQTT, enregistre le client réseau
     * et (optionnellement) un debug pour le debug. Elle permet également de spécifier
     * un identifiant client ainsi que des identifiants d’authentification (username/password).
     *
     * @param netClient Référence à un objet réseau compatible (par ex. EthernetClient, WiFiClient).
     * @param broker Adresse du broker MQTT (nom de domaine ou IP).
     * @param port Port du broker MQTT (généralement 1883 ou 8883 pour TLS).
     * @param clientId Identifiant unique du client MQTT.
     * @param dbg (Optionnel) Pointeur vers une instance de SerialDebug pour le logging.
     * @param user (Optionnel) Nom d’utilisateur MQTT pour l’authentification.
     * @param pass (Optionnel) Mot de passe MQTT pour l’authentification.
     *
     * @note Cette fonction doit être appelée avant toute tentative de publish/subscribe.
     * @note Si user et pass ne sont pas fournis, la connexion MQTT se fera sans authentification.
     */
    void begin(Client &netClient, const char *broker, uint16_t port,
               const char *clientId,
               const char *user = nullptr, const char *pass = nullptr)
    {
        mqttClient = new PubSubClient(netClient);
        mqttClient->setServer(broker, port);
        mqttClient->setCallback(MQTTManager::mqttCallbackStatic);
        strncpy(this->clientId, clientId, sizeof(this->clientId) - 1);

        if (user)
            strncpy(this->username, user, sizeof(this->username) - 1);
        if (pass)
            strncpy(this->password, pass, sizeof(this->password) - 1);

        mqttInstance = this;
    }

    /**
     * @brief Boucle principale de gestion MQTT.
     *
     * Cette fonction doit être appelée régulièrement dans la boucle principale (`loop()` d’Arduino).
     *
     * - Si le client MQTT est déconnecté, elle tente automatiquement une reconnexion
     *   via la fonction interne reconnect().
     * - Si le client est connecté, elle exécute `mqttClient->loop()` afin de :
     *    - gérer la communication avec le broker,
     *    - traiter les messages entrants,
     *    - maintenir la connexion active.
     *
     * @note Cette fonction ne doit pas être bloquante et doit être appelée aussi souvent
     *       que possible pour assurer une bonne réactivité aux messages MQTT.
     */
    void loop()
    {
        if (!mqttClient->connected())
        {
            reconnect();
        }

        if (mqttClient->connected())
        {
            mqttClient->loop();
        }        
    }

    /// @brief Retourne l'etat de la connexion
    /// @return True si le client est connecté au brocker
    boolean connected()
    {
        return mqttClient->connected();
    }

    /**
     * @brief Souscrit à un topic MQTT et associe une fonction de rappel.
     *
     * Cette méthode permet d’enregistrer un handler pour un topic donné.
     * Chaque fois qu’un message est reçu sur ce topic, la fonction callback
     * associée est exécutée avec la charge utile en paramètre.
     *
     * @param topic Nom du topic MQTT auquel s’abonner.
     * @param cb    Fonction de rappel (callback) appelée lors de la réception
     *              d’un message sur ce topic. Elle reçoit la payload sous
     *              forme de String.
     *
     * @note
     * - Le nombre maximum de topics souscrits est limité à MAX_MQTT_TOPICS.
     * - Si le client est déjà connecté au broker, l’abonnement est
     *   immédiatement envoyé au broker. Sinon, l’abonnement sera effectué
     *   automatiquement après reconnexion.
     * - Si la limite de topics est atteinte, l’appel est ignoré.
     *
     * @see publish()
     */
    void subscribe(const char *topic, MQTTCallback cb)
    {
        if (topicCount < MAX_MQTT_TOPICS)
        {
            topicHandlers[topicCount++] = {topic, cb};
            if (mqttClient->connected())
            {
                mqttClient->subscribe(topic);
                log(LogLevel::Info, String("Subscribed to: ") + topic);
            }
        }
    }

    /**
     * @brief Publie un message sur un topic MQTT.
     *
     * Envoie la chaîne de caractères `payload` au topic spécifié.
     * Si le client MQTT n'est pas connecté au broker, le message
     * n'est pas envoyé.
     *
     * @param topic   Nom du topic sur lequel publier le message.
     * @param payload Contenu du message à publier.
     *
     * @note
     * - La publication n’est effectuée que si le client est connecté.
     * - Un log de niveau Verbose est généré si un objet SerialDebug
     *   est attaché.
     *
     * @see subscribe()
     */
    void publish(const char *topic, const String &payload)
    {
        if (mqttClient->connected())
        {
            mqttClient->publish(topic, payload.c_str());
            log(LogLevel::Verbose, String("Published to ") + topic + ": " + payload);
        }
    }

private:
    PubSubClient *mqttClient;
    SerialDebug *debug;
    char clientId[32];
    char username[32];
    char password[32];

    MQTTTopicHandler topicHandlers[MAX_MQTT_TOPICS];
    uint8_t topicCount;

    static MQTTManager *mqttInstance;

    /**
 * @brief Tente de (re)connecter le client MQTT au broker de manière non bloquante.
 *
 * Cette méthode vérifie si le client MQTT est connecté. Si ce n'est pas le cas,
 * elle tente de se reconnecter en utilisant les informations configurées via begin().
 *
 * - Si un nom d'utilisateur et un mot de passe sont fournis, ils sont utilisés.
 * - Sinon, la connexion se fait sans authentification.
 *
 * Après une connexion réussie, tous les topics précédemment souscrits
 * sont automatiquement resouscrits.
 *
 * En cas d'échec, une nouvelle tentative est programmée après un délai
 * défini (par défaut 10 secondes).
 *
 * @note Cette version est non bloquante : elle ne fait qu'une tentative par appel
 *       et programme la prochaine en fonction de millis().
 */
    void reconnect()
    {
        static unsigned long lastAttempt = 0;
        const unsigned long retryInterval = 60000; // 60 secondes

        if (mqttClient->connected())
            return;

        unsigned long now = millis();
        if (now - lastAttempt < retryInterval && lastAttempt != 0)
            return; // attendre avant de retenter

        lastAttempt = now;

        log(LogLevel::Info, F("Tentative de connexion MQTT..."));

        bool connected;
        if (username[0] != '\0')
        {
            connected = mqttClient->connect(clientId, username, password);
        }
        else
        {
            connected = mqttClient->connect(clientId);
        }

        if (connected)
        {
            log(LogLevel::Info, F("Connecté au broker MQTT"));
            // Resubscribe to all topics
            for (uint8_t i = 0; i < topicCount; i++)
            {
                mqttClient->subscribe(topicHandlers[i].topic);
            }

            // Une fois soucrit, on indique au broker qu'on est en ligne
            mqttClient->publish("CtrlAccess/state/Online", "");

        }
        else
        {
            String msg = F("Échec de connexion MQTT, nouvelle tentative dans ");
            msg += (const String) (retryInterval / 1000);
            msg += F(" secondes");
            log(LogLevel::Error, msg);
        }
    }


    /**
     * @brief Callback statique appelé par la bibliothèque PubSubClient lors de la réception d'un message MQTT.
     *
     * Cette fonction est statique car PubSubClient nécessite une fonction de callback globale ou statique.
     * Elle redirige ensuite l'appel vers la méthode d'instance `mqttCallback` de l'objet MQTTManager actif.
     *
     * @param topic   Le topic sur lequel le message a été reçu.
     * @param payload Le tableau d'octets contenant le message reçu.
     * @param length  La longueur du message en octets.
     *
     * @note Utilise le pointeur statique `mqttInstance` pour accéder à l'objet MQTTManager.
     *       Assurez-vous qu'une instance a été assignée avant la réception de messages.
     */
    static void mqttCallbackStatic(char *topic, byte *payload, unsigned int length)
    {
        if (mqttInstance)
        {
            mqttInstance->mqttCallback(topic, payload, length);
        }
    }

    /**
     * @brief Callback interne appelé par la bibliothèque PubSubClient lors de la réception d’un message MQTT.
     *
     * Cette méthode est responsable de :
     *  - Transformer le payload reçu (tableau de bytes) en une chaîne Arduino `String`.
     *  - Chercher un handler associé au topic reçu.
     *  - Logger systématiquement la réception du message sur les topics abonnés.
     *  - Appeler la fonction callback associée au topic si elle existe.
     *  - Logger un warning si le message est reçu sur un topic non géré.
     *
     * @param topic Le nom du topic sur lequel le message a été reçu.
     * @param payload Tableau de bytes contenant le message reçu.
     * @param length Taille du payload en bytes.
     *
     * @note Cette fonction est appelée uniquement par la fonction statique `mqttCallbackStatic`.
     *       Elle ne doit pas être appelée directement par l’utilisateur.
     *
     * @details
     * 1. Convertit le payload en `String` et supprime les espaces superflus (trim).
     * 2. Parcourt la liste des handlers souscrits (`topicHandlers`) pour trouver une correspondance.
     * 3. Si un handler est trouvé :
     *    - Log le message reçu avec le niveau `Debug`.
     *    - Appelle la callback associée avec le message sous forme de `String`.
     * 4. Si aucun handler n’est trouvé :
     *    - Log un warning indiquant que le topic n’est pas géré.
     */
    void mqttCallback(char *topic, byte *payload, unsigned int length)
    {
        String msg;
        for (unsigned int i = 0; i < length; i++)
        {
            msg += (char)payload[i];
        }
        msg.trim();

        // 🔹 Recherche d’un handler associé au topic
        for (uint8_t i = 0; i < topicCount; i++)
        {
            if (strcmp(topic, topicHandlers[i].topic) == 0)
            {
                // 🔹 Log systématique de chaque réception sur les topic abonné
                log(LogLevel::Debug, String("Réception MQTT [") + topic + "] : " + msg);

                if (topicHandlers[i].callback)
                {
                    topicHandlers[i].callback(msg);
                }
                return;
            }
        }

        // 🔹 Si aucun handler trouvé → warning
        log(LogLevel::Warning, String("Message reçu sur un topic non géré: ") + topic);
    }

    /**
     * @brief Log un message via l'objet SerialDebug attaché.
     *
     * Si un objet SerialDebug a été passé à la classe MQTTManager via begin(),
     * cette méthode transmet le message à cet objet pour affichage sur le port série
     * selon le niveau de log fourni.
     *
     * @param level Niveau de log du message (LogLevel::Error, Warning, Info, Debug, Verbose).
     * @param msg   Message à afficher (type String).
     *
     * @note Si aucun SerialDebug n'est attaché (logger == nullptr), la fonction ne fait rien.
     */
    void log(LogLevel level, const String &msg)
    {
        if (debug)
        {
            debug->log(level, msg.c_str());
        }
    }
};

MQTTManager *MQTTManager::mqttInstance = nullptr;

#endif
