#ifndef MQTT_H
#define MQTT_H

#pragma once

#include <PubSubClient.h>

extern const char *MQTT_USERNAME; // Login utilisateur
extern const char *MQTT_PASSWORD; // Login mot de passe
extern const char *MQTT_SERVER;   // Adresse du broker
extern const int MQTT_PORT;       // Port du broker

extern const unsigned short TpsDefCom; // Temps avant défaut com. (sec)
extern unsigned short TempoCom;        // Tempo défaut de communication
extern char Life[2];                   // Mot de vie
extern char LifeOld[2];                // Mot de vie old
extern bool DefCom;                    // Défaut de com

extern PubSubClient MqttClient;

/**
 * @brief Tente de reconnecter le client MQTT au broker si la connexion est perdue.
 *
 * Cette fonction vérifie l'état de connexion du client MQTT. Si déconnecté, elle
 * tente de se reconnecter en souscrivant à une liste prédéfinie de topics.
 * Affiche des messages d'état et gère les erreurs éventuelles.
 */
void reconnectToMqttBroker();

/**
 * @brief Gère la boucle MQTT pour vérifier les messages entrants et maintenir la connexion.
 *
 * Si le client est connecté, cette fonction exécute la boucle MQTT pour traiter les messages.
 * Si déconnecté et si un intervalle de temps est écoulé, elle tente de reconnecter le client.
 *
 * @param ft10s Indique si 10 secondes se sont écoulées pour déclencher une tentative de reconnexion.
 */
void processMqtt(const bool ft10s);

/**
 * @brief Callback pour gérer les messages MQTT reçus.
 *
 * Cette fonction est appelée chaque fois qu'un message MQTT est reçu. Elle analyse
 * le topic et le message, puis applique une logique spécifique en fonction du topic.
 *
 * @param topic Le nom du topic sur lequel le message a été reçu.
 * @param payload Les données du message en tant que tableau d'octets.
 * @param length La longueur du message reçu.
 */
void onMqttMessageReceived(char *topic, byte *payload, unsigned int length);

/**
 * @brief Envoie un battement de cœur (heartbeat) périodique au broker MQTT pour indiquer l'état du système.
 *
 * Cette fonction publie un signal de vie sur un topic MQTT dédié à des intervalles réguliers.
 * Elle surveille également le temps écoulé depuis la dernière communication pour détecter des défauts
 * de communication (DefCom) et agit en conséquence.
 *
 * @param ft1s Indique si une seconde s'est écoulée pour synchroniser l'envoi du heartbeat.
 */
void sendHeartbeat(const bool ft1s);

#endif