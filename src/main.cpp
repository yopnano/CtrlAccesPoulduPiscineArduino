// Local libs
#include "Ether\Ethernet.h"
#include "RTC\RTC.h"
#include "RTC\NTP.h"
#include "MQTT\Topic.h"
#include "MQTT\MQTT.h"
#include "AccessControl\AccessControl.h"
#include "Utils.h"

Tempo tempo1s(1000); // Tempo 1 seconde
void setup()
{
  // I/O Inputs
  pinMode(PIN_EXIT_BUTTON, INPUT_PULLUP);

  // I/O Outputs
  pinMode(PIN_BUZZER_KEYPAD, OUTPUT);
  pinMode(PIN_LED_KEYPAD, OUTPUT);
  pinMode(PIN_LOCK_RELAY, OUTPUT);

  // I/O Initial state
  digitalWrite(PIN_BUZZER_KEYPAD, HIGH);
  digitalWrite(PIN_LED_KEYPAD, HIGH);
  digitalWrite(PIN_LOCK_RELAY, HIGH);

  // Init Objects
  Serial.begin(9600);
  while (!Serial)
    ;

  initializeRtc();
  initializeEthernet();

  Ntp.beginListening();                          // Listen request paquet for NTP
  MqttClient.setServer(MQTT_SERVER, MQTT_PORT);  // Configure MQTT server
  MqttClient.setCallback(onMqttMessageReceived); // Callback function for subcribed topics

  Wg.begin(PIN_D0_KEYPAD, PIN_D1_KEYPAD); // Init Wiguand keypad
}

void loop()
{

  // Gestion de la tempo 1 seconde
  tempo1s.run();

  // Toutes les secondes
  if (tempo1s.ft())
  {
    // Lecture de l'heure locale
    UTC = Rtc.now();
    synchronizeLocalTime();
  }
  
  // Toutes les minutes
  if (tempo1s.ft(60))
  {
    // Affichage de l'heure locale
    DEBUGLN(Local.timestamp());

    // Génération du code de secours
    generateBackupCode();
  }

  // Gestion des requêtes NTP
  handleNtpRequests();

  // Gestion de l'échange MQTT + Bit de vie
  processMqtt(tempo1s.ft(10));
  sendHeartbeat(tempo1s.ft());

  // Gestion des appareils (clavier, bouton de sortie, relais du portail)
  handleKeypadAndRfidInput();
  monitorExitButton();
  controlDoorLock(tempo1s.ft());

}