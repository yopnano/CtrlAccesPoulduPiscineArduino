// Local libs
#include "Ether\Ethernet.h"
#include "RTC\RTC.h"
#include "RTC\NTP.h"
#include "MQTT\Topic.h"
#include "MQTT\MQTT.h"
#include "AccessControl\AccessControl.h"
#include "Utils.h"

Tempo tempo1s(1000);           // Tempo 1 seconde
unsigned int tempo1sRelaisUPS; // Tempo de comptage relais ups

void setup()
{
  // I/O Inputs
  pinMode(PIN_EXIT_BUTTON, INPUT_PULLUP);
  pinMode(PIN_5VSB_RASPBERRY, INPUT);

  // I/O Outputs
  pinMode(PIN_BUZZER_KEYPAD, OUTPUT);
  pinMode(PIN_LED_KEYPAD, OUTPUT);
  pinMode(PIN_LOCK_RELAY, OUTPUT);
  pinMode(PIN_UPS_RELAY, OUTPUT);

  // I/O Initial state
  digitalWrite(PIN_BUZZER_KEYPAD, HIGH);
  digitalWrite(PIN_LED_KEYPAD, HIGH);
  digitalWrite(PIN_LOCK_RELAY, HIGH);
  digitalWrite(PIN_UPS_RELAY, LOW);

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

  // Lecture de l'entrée analogique de tension raspberry
  int Rpi_Voltage = analogRead(PIN_5VSB_RASPBERRY);

  // Gestion de la tempo 1 seconde
  tempo1s.run();

  // Toutes les secondes
  if (tempo1s.ft())
  {
    // Lecture de l'heure locale
    UTC = Rtc.now();
    synchronizeLocalTime();

    if (Rpi_Voltage < 500 && tempo1sRelaisUPS < TempoRelaisOnduleur)
      tempo1sRelaisUPS++;
    else
      tempo1sRelaisUPS = 0;
  }

  // Toutes les 10 secondes
  if (tempo1s.ft(10))
  {
    // Affichage de l'heure locale
    DEBUG(F("Heure locale : "));
    DEBUGLN(Local.timestamp());

    // Génération du code de secours
    generateBackupCode();

    // Affichage de la tension du raspberry
    DEBUG(F("Rpi_Voltage "));
    DEBUGLN(Rpi_Voltage);
  }

  // Maintiens du relais UPS, tant que l'alimentation fonctionne
  digitalWrite(PIN_UPS_RELAY, tempo1sRelaisUPS < TempoRelaisOnduleur);

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