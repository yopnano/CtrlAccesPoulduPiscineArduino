#include <Mapping.h>
#include <Params.h>

void setup()
{
  // Forçage état initial des relais
  RelaisPorte.turnOff();
  RelaisOndule.turnOff();

  Modsetup();
}


void loop()
{  
  // 1. Rafraîchissement des instances (Automate)
  ModMain();
  
  // 2. Caresse au Watchdog pour signaler que l'Arduino n'est pas planté
  wdt_reset();

  // 3. Ecoute continue et non-bloquante de Node-RED
  EcouterNodeRED();

  // 4. Exécution cyclique à 1Hz (Bit de vie)
  if (sys.ft1Hz)
  {
    GestionBitVie();
  }  

  // Pilotage du témoin lumineux du clavier
  // Vert dans un des cas suivants :
  // - Une saisie est en cours
  // - Le portail est débloqué
  LedClavier.write(LecteurEntree.hasData() || TON_RelaisPortail.IN);

  // 5. Exécution cyclique rapide (Logique métier physique)
  GestionControleAcces();
  GestionBoutonSortie();
  GestionVentousePortail();

  // 6. Gestion du maintien d'alimentation UPS
  TOF_RelaisOndule.IN = analogRead(PIN_5VSB_RASPBERRY) > 500;
  RelaisOndule.write(TOF_RelaisOndule.Q());

}

// ==========================================
//          IMPLEMENTATION METIER
// ==========================================

/// @brief Ecoute non-bloquante du port Série (Trames envoyés par Node-RED)
void EcouterNodeRED()
{
  while (Serial.available() > 0) {
    char c = Serial.read();
    
    if (c == '\n' || c == '\r') {
      if (indexReception > 0) {
        bufferReception[indexReception] = '\0'; // Fin de chaîne
        ExecuterOrdre(bufferReception);
        indexReception = 0; 
      }
    } 
    else if (indexReception < sizeof(bufferReception) - 1) {
      bufferReception[indexReception++] = c;
    }
  }
}


/// @brief Interprète les ordres du Raspberry (Ex: "OPEN:5", "PONG")
void ExecuterOrdre(char* ordre)
{
  // Maintien en vie
  if (strncmp(ordre, "PONG", 4) == 0) {
    CompteurDefautCom = 0; 
  }
  // Action Ouverture
  else if (strncmp(ordre, "OPEN:", 5) == 0) {
    // Le Rasp a répondu ! On annule le timeout et on vide le code en attente
    attenteReponseRaspberry = false; 
    codeEnAttente[0] = '\0';         
    
    int temps = atoi(ordre + 5); 
    if(temps < 1) temps = TpsOuvertureEntree;
    Deverrouillage(temps);
  }
  // Actions IHM manuelles
  else if (strncmp(ordre, "BUZZ:1", 6) == 0) BuzzerClavier.turnOn();
  else if (strncmp(ordre, "BUZ:0", 6) == 0) BuzzerClavier.turnOff();
  else if (strncmp(ordre, "LED:1", 5) == 0) LedClavier.turnOn();
  else if (strncmp(ordre, "LED:0", 5) == 0) LedClavier.turnOff();
}


/// @brief Gestion de la perte de communication (Ping-Pong)
void GestionBitVie()
{
  Serial.println(F("DATA:PING"));

  if (CompteurDefautCom < TPS_DEFAUT_COM) {
    CompteurDefautCom++;
  }

  DefCom = (CompteurDefautCom >= TPS_DEFAUT_COM);

  // Sur front de changement d'état
  if (DefCom != DefComOld) 
  {
    if (DefCom) {
      Serial.println(F("LOG:ALERTE - Perte de com USB ! Passage en mode autonome."));
    } 
    else {
      Serial.println(F("LOG:RETOUR NORMAL - USB retabli."));
    }
    DefComOld = DefCom;
  }
}


/// @brief Séquence de contrôle d'accès (Raspberry ou local en secours)
void GestionControleAcces()
{
  // 1. Un badge ou code vient d'être saisi
  if (LecteurEntree.available())
  {
    // Lecture du code en attente de traitement
    LecteurEntree.read(codeEnAttente, sizeof(codeEnAttente));
    
    // Envoie de l'info à Node-RED AVANT le bip de confirmation
    Serial.print(F("DATA:BADGE:"));
    Serial.println(codeEnAttente);
    
    // On lance le chrono de 1,5 seconde
    chronoAttenteRaspberry = millis();
    attenteReponseRaspberry = true;

    // Attente de confort PUIS bip court de confirmation de réception
    // (Node-RED a déjà reçu le code et travaille pendant cette demi-seconde)
    delay(500); 
    BuzzerClavier.turnOn(); delay(100); BuzzerClavier.turnOff(); 
  }

  // 2. Surveillance non-bloquante du Timeout
  if (attenteReponseRaspberry)
  {
    // Limite de temps passée à 1500 ms
    if (millis() - chronoAttenteRaspberry >= 1500) 
    {
      // --- TIMEOUT ATTEINT (Le Raspberry n'a pas répondu OPEN:) ---
      attenteReponseRaspberry = false; 
      
      BuzzerClavier.turnOn(); delay(100); BuzzerClavier.turnOff(); // Bip court signalant le Timeout
      
      // On teste les codes de secours locaux
      bool valideLocalement = false;
      for (byte i = 0; i < sizeof(CODES_SECOURS)/sizeof(CODES_SECOURS[0]); i++) {
        if (strcmp(codeEnAttente, CODES_SECOURS[i]) == 0) valideLocalement = true;
      }
      for (byte i = 0; i < sizeof(BADGES_SECOURS)/sizeof(BADGES_SECOURS[0]); i++) {
        if (strcmp(codeEnAttente, BADGES_SECOURS[i]) == 0) valideLocalement = true;
      }

      if (valideLocalement) {
        Serial.print(F("LOG:Ouverture SECOURS par code : "));
        Serial.println(codeEnAttente);
        Deverrouillage(TpsOuvertureEntree);
      } 
      else {
        Serial.println(F("LOG:Code inconnu, acces refuse."));
        BuzzerClavier.turnOn(); delay(500); BuzzerClavier.turnOff(); // Bip long de refus
      }
      
      // Nettoyage du code en attente
      codeEnAttente[0] = '\0';
    }
  }
}

/// @brief Traitement du bouton poussoir local
void GestionBoutonSortie()
{
  TrigBpSortie.Analyse(BpSortie.read());
  if (TrigBpSortie.F_trig())
  {
    Serial.println(F("LOG:Bouton sortie presse"));
    Serial.println(F("DATA:BOUTON:ON"));
    Deverrouillage(TpsOuvertureSortie);
  }
}


/// @brief Pilotage de la tempo de déverrouillage (Universel)
void Deverrouillage(const unsigned int time_s)
{
  Serial.print(F("LOG:Deverrouillage pour "));
  Serial.print(time_s);
  Serial.println(F(" sec"));

  // L'animation "Bip Bip" est maintenant universelle (Node-RED, Secours, Bouton Sortie)
  delay(20);
  BuzzerClavier.turnOn();  delay(120);
  BuzzerClavier.turnOff(); delay(120);
  BuzzerClavier.turnOn();  delay(100);
  BuzzerClavier.turnOff();

  TON_RelaisPortail.PT = time_s * TIME::Secondes;
  TON_RelaisPortail.IN = false;
}


/// @brief Basculement du relais
void GestionVentousePortail()
{
  // Fin de la temporisation
  if (TON_RelaisPortail.Q())
  {
    TON_RelaisPortail.PT = 0;
    Serial.println(F("LOG:Verrouillage du portail"));
  }
  
  // Commande du relais de la ventouse
  RelaisPorte.write(TON_RelaisPortail.IN);
  
  // Maintien de l'impulsion de la fonction TON
  TON_RelaisPortail.IN = TON_RelaisPortail.PT > 0;
}