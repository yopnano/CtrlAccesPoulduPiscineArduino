#include <Mapping.h>
#include <Params.h>

int tempo1s = 0;
int g7 = 0;

bool BitVie = false;
bool BitVieOld = false;
bool DefCom = false;
unsigned int TempoCom = 0;

/// @brief Fonction de souscription au topic MQTT, avec leur fonctions de callback
void MqqtSubscription()
{
  // Gestion du bit de vie par MQTT
  Mqtt.subscribe("CtrlAccess/Life", [](const String &payload)
                 {
                   bool VieState = (payload == "1"); // Lecture du bit de vie
                   BitVie = !VieState;               // Inversion du bit de vie reçu, pour rémission
                 });

  // Gestion de la led du clavier par MQTT
  Mqtt.subscribe("CtrlAccess/Led", [](const String &payload)
                 {
        bool ledState = (payload == "1");
        LedClavier.write(ledState); });

  // Gestion du déverrouillage du portail par MQTT
  Mqtt.subscribe("CtrlAccess/Deverrouillage", [](const String &payload)
                 {
                   Debug.log(LogLevel::Info, ("Déverrouillage: " + payload + " secondes"));

                   unsigned int time_s = payload.toInt();

                   time_s = max(time_s, 5); // Temps d'ouverture limité à 5 secondes minimum
                   Deverrouillage(time_s); });

  // Gestion du buzzer du clavier par MQTT
  Mqtt.subscribe("CtrlAccess/Buzzer", [](const String &payload)
                 {
        bool buzzerState = (payload == "1");
        BuzzerClavier.write(buzzerState); });

  Mqtt.subscribe("CtrlAccess/Param", [](const String &payload)
                 { ParseParametres(payload); });

  Mqtt.subscribe("CtrlAccess/Rtc", [](const String &payload)
                 { Debug.log(LogLevel::Info, ("RTC Update: " + payload)); });
}

/// @brief Setup Arduino
void setup()
{
  // Forçage état initial des relais
  RelaisPorte.turnOff();
  RelaisOndule.turnOff();

  Modsetup();

  MqqtSubscription();
}

/// @brief Boucle Arduino
void loop()
{
  // fonction main des méthodes
  ModMain();

  // Mqtt fonctionne en veille
  if (!LecteurEntree.hasData())
  {    
    Mqtt.loop();
  }

  // Cadence 1 seconde
  if (sys.ft1Hz)
  {
    tempo1s++;

    // Gestion et envoi du bit de vie vers MQTT
    GestionBitVie();
  }
  

  // Pilotage du témoin lumineux du clavier
  // Vert dans un des cas suivants :
  // - Une saisie est en cours
  // - Le portail est débloqué
  LedClavier.write(LecteurEntree.hasData() or TON_RelaisPortail.IN);
  
  // Gestion du traitement des codes et du lecteur
  GestionControleAcces();

  // Gestion du bouton de sortie
  GestionBoutonSortie();

  // Gestion de la tempo et pilotage du relais
  GestionVentousePortail();
  
  /*
  if (g7 == 0)
  {

    if (sys.ft1Hz)
    {

      Serial.print("Now = ");
      Serial.println(now.toStringHMS());

    }

    //BP cycle manuel
    if (digitalRead(2) == LOW)
    {
        tone(A2, 700, 1000);
        delay(1000);
        tempo1s = 0;
        g7 = 1;
    }
  }


  // Etape 0 Attente cdt fct

  // Etape 1
  if (g7 == 1 && tempo1s >= 2)
  {
    tempo1s = 0;
    g7 = 2;
  }

// Etape 2
  if (g7 == 2 && tempo1s >= 4)
  {
    tempo1s = 0;
    g7 = 3;
  }
*/
  // ACTIONS G7

  // Gestion du relais alimentaion ondulée
  TOF_RelaisOndule.IN = analogRead(PIN_5VSB_RASPBERRY) > 500;
  RelaisOndule.write(TOF_RelaisOndule.Q());
}

/// @brief Gestion de l'alternance du bit de vie envoyé par MQTT
void GestionBitVie()
{
  // Bit OK
  if (BitVie != BitVieOld)
  {
    TempoCom = 0;
  }

  // Bit identique au dernier cycle (inc. tempo def com)
  else if (TempoCom < TpsDefCom)
  {
    TempoCom++;
  }

  // Affectation défaut de com, et mémorisation bit de vie actuel
  DefCom = TempoCom >= TpsDefCom;
  BitVieOld = BitVie;

  // Publication du bit de vie, on passe d'un booléen à une String à l'aide d'une expression ternaire
  Mqtt.publish("CtrlAccess/state/Life", BitVie ? "1" : "0");
}

/// @brief Gestion globale du contrôle d'accès
void GestionControleAcces()
{
  // Vérifie si le lecteur de badge à des données à transmettre
  if (LecteurEntree.available())
  {
    // Passage tu témoin lumineux en rouge
    LedClavier.turnOff();
    
    // Récupération de la (saisie / lecture RFID)
    const String code = LecteurEntree.read();

    // Si le code est validé par MQTT (Node Red)
    // Le dévérouillage est effectif, on peut quitter la fonction
    if (ValiderCodeMQTT(code) == true)
      return;

    BuzzerClavier.turnOn();
    delay(300);
    BuzzerClavier.turnOff();
    delay(300);
    
    // Si le code est validé localement
    // Le dévérouillage est effectif, on peut quitter la fonction
    if (ValiderCodeLocal(code) == true)
    {
      // delay(700);
      Deverrouillage(TpsOuvertureEntree);
      return;
    }
    
    // Dans les autres cas on, fait un bip long (accès refusé) 
    BuzzerClavier.turnOn();
    delay(800);
    BuzzerClavier.turnOff();
  }
}

/// @brief Surveillance du bouton de sortie
void GestionBoutonSortie()
{
  // Detection du front montant / descendant
  TrigBpSortie.Analyse(BpSortie.read());

  // Front montant (Apuuis sur le bouton)
  if (TrigBpSortie.R_trig())
  {
    Debug.log(LogLevel::Info, F("Bouton de sortie enfoncé"));
  }
  // Front descendant (Relâchement du bouton)
  else if (TrigBpSortie.F_trig())
  {
    Debug.log(LogLevel::Info, F("Bouton de sortie relaché"));

    unsigned int time_s = max(10, TpsOuvertureSortie);

    Deverrouillage(time_s);
  }
}

/// @brief Gestion de l'ouverture temporisée du portail
void GestionVentousePortail()
{

  // RAZ si la tempo à atteint le temps maximum d'ouverture
  if (TON_RelaisPortail.Q())
  {
    TON_RelaisPortail.PT = 0;
    Debug.log(LogLevel::Info, F("Verrouillage du portail"));
  }

  // Si la tempo cours, on affiche les messages de debug
  else if (TON_RelaisPortail.IN and sys.ft1Hz)
  {
    unsigned int remainingTime = TON_RelaisPortail.PT.totalSecond() - TON_RelaisPortail.ET().totalSecond();
    Debug.concat(F("Déverrouillage du portail pendant"));
    Debug.concat((const String)remainingTime);
    Debug.log(LogLevel::Info, F("secondes"));
  }

  // Pilotage du relais du portail (tant que la tempo cours)
  RelaisPorte.write(ModeFctRelaisPortail == 1 or (ModeFctRelaisPortail == 3 and TON_RelaisPortail.IN));

  // Déclenchement de la tempo si le preset est supérieur à 0
  TON_RelaisPortail.IN = TON_RelaisPortail.PT > 0;

}


/// @brief Validation des codes par MQTT
/// @param code Le code saisi au clavier ou tag RFID
/// @return true si le code correspond à un code/badge autorisé, false sinon
bool ValiderCodeMQTT (const String code)
{

  // Si Mqtt n'est pas connecter, on ne va pas plus loin et on passe à la vérifcation des codes locaux
  if (!Mqtt.connected())
    return false;
  
  // Envoi du code vers MQTT

  // Préparation de la tramme
  String payload;
  payload = "{\"code\": \"";
  payload += code;
  payload += "\" ,\"type\": \"Saisie\"}";
  
  // Publication sur MQTT
  Mqtt.publish("CtrlAccess/state/Wiegand", payload);


  // Attente le temps de recevoir une réponse du rasberry
  unsigned long startTime = millis(); // Temps de début
    
  // On laisse une seconde de delais
  while (millis() - startTime < 1000) {
    Mqtt.loop();

    // Detection de la demande d'ouverture
    if (TON_RelaisPortail.PT > 0)
    {
      Debug.concat(F("Code : "));
      Debug.concat(code);      
      Debug.log(LogLevel::Info, F("validé par MQTT"));
      return true;
    }

  }
  
  Debug.concat(F("Code : "));
  Debug.concat(code);      
  Debug.log(LogLevel::Info, F("refusé par MQTT"));

  return false;
  
}

/// @brief Validation des codes locaux
/// @param code Le code saisi au clavier ou tag RFID
/// @return true si le code correspond à un code/badge autorisé, false sinon
bool ValiderCodeLocal (const String code)
{

  // Vérification des badges de secours
  for (const auto &badge : BADGES_SECOURS)
  {
    if (code.equals(badge))
    {
      Debug.concat(F("Code valide : Badge de secours"));
      Debug.log(LogLevel::Info, code);
      return true;
    }
  }

  // Vérification du code de secours fixe
  for (const auto &codeS : CODES_SECOURS)
  {
    if (code.equals(codeS))
    {
      Debug.concat(F("Code valide : Code de secours fixe"));
      Debug.log(LogLevel::Info, code);
      return true;
    }
  }

  // Aucun match
  Debug.concat(F("Code invalide : "));
  Debug.log(LogLevel::Warning, code);
  return false;
}

/// @brief Deverrouillage momentané du portail
/// @param time_s Temps de déverrouillage en secondes
void Deverrouillage(const unsigned int time_s)
{
  LedClavier.turnOn();
  BuzzerClavier.turnOn();
  delay(150);
  BuzzerClavier.turnOff();
  delay(120);
  BuzzerClavier.turnOn();
  delay(150);
  BuzzerClavier.turnOff();

  TON_RelaisPortail.PT = time_s * TIME::Secondes;
  TON_RelaisPortail.IN = false;
}

/**
 * @brief Parse une chaîne de paramètres (clé=valeur) séparés par des retours à la ligne
 *        et met à jour les variables associées.
 *
 * @param param Chaîne contenant les paramètres au format :
 *              "cle=valeur\ncle=valeur\n..."
 */
void ParseParametres(const String &param)
{
  unsigned int start = 0;
  while (start < param.length())
  {
    int end = param.indexOf('\n', start);
    if (end == -1)
      end = param.length();

    // Extraire la ligne complète
    String line = param.substring(start, end);
    line.trim(); // Supprimer espaces ou retours chariot

    if (line.length() > 0)
    {
      int delimiter = line.indexOf('=');
      if (delimiter != -1)
      {
        String key = line.substring(0, delimiter);
        String value = line.substring(delimiter + 1);

        key.trim();
        value.trim();

        // Log de debug
        Debug.log(LogLevel::Info, "[Param] " + key + " = " + value);

        // Mapper les clés aux variables
        if (key == F("temps_ouverture/entree"))
        {
          TpsOuvertureEntree = value.toInt();
        }
        else if (key == F("temps_ouverture/sortie"))
        {
          TpsOuvertureSortie = value.toInt();
        }
        else if (key == F("acces_piscine/date_debut"))
        {
          // ParseDateString(value, DebutAcces);
        }
        else if (key == F("acces_piscine/date_fin"))
        {
          // ParseDateString(value, FinAcces);
        }
        else if (key == F("acces_piscine/heure_debut"))
        {
          // ParseTimeString(value, DebutAcces);
        }
        else if (key == F("acces_piscine/heure_fin"))
        {
          // ParseTimeString(value, FinAcces);
        }
      }
    }

    start = end + 1; // passer à la prochaine ligne
  }
}

/**
 * @brief Met à jour la date (jour/mois) d'une instance DateTime existante
 *        à partir d'une chaîne "DD/MM".
 *
 * @param dateStr Chaîne au format "DD/MM"
 * @param dt      Référence à l'objet DateTime à modifier
 */
/*
void ParseDateString(const String &dateStr, DateTime &dt)
{
  int sep = dateStr.indexOf('/');
  if (sep == -1)
  {
    Debug.concat(F("Format de date incorrect"));
    Debug.log(LogLevel::Error, dateStr);
    return;
  }

  const int day = dateStr.substring(0, sep).toInt();
  const int month = dateStr.substring(sep + 1).toInt();

  Debug.concat(F("date : jour = "));
  Debug.concat(day);
  Debug.concat(F(" mois = "));
  Debug.log(LogLevel::Verbose, month);

  // On reconstruit l'objet avec nouvelle date mais on conserve l'heure existante
  dt = DateTime(
      dt.year(),
      month,
      day,
      dt.hour(),
      dt.minute(),
      dt.second());
}
*/
/**
 * @brief Met à jour l'heure (HH:MM) d'une instance DateTime existante
 *        à partir d'une chaîne "HH:MM".
 *
 * @param timeStr Chaîne au format "HH:MM"
 * @param dt      Référence à l'objet DateTime à modifier
 */

/*
 void ParseTimeString(const String &timeStr, DateTime &dt)
{
  int sep = timeStr.indexOf(':');
  if (sep == -1)
  {
    Debug.concat(F("Format d'heure incorrect"));
    Debug.log(LogLevel::Error, timeStr);
    return;
  }

  const int hour = timeStr.substring(0, sep).toInt();
  const int minute = timeStr.substring(sep + 1).toInt();

  Debug.concat(F("time : heure = "));
  Debug.concat(hour);
  Debug.concat(F(" minute = "));
  Debug.log(LogLevel::Verbose, minute);

  // On reconstruit l'objet avec nouvelle heure mais on conserve la date existante
  dt = DateTime(
      dt.year(),
      dt.month(),
      dt.day(),
      hour,
      minute,
      0);
}
*/