#include "AccessControl.h"

#include "MQTT\MQTT.h"
#include "MQTT\Topic.h"
#include "RTC\RTC.h"
#include "Utils.h"

// Parameters
const char EMERGENCY_BADGES[][10] PROGMEM = {
    "23B0991C",
    "23B0991C"};
const char EMERGENCY_CODE[] PROGMEM = "29360";
char BackupCode[] = "123456";
unsigned int OpenDelayCode = 10;
unsigned int OpenDelayButton = 10;
unsigned int TempoOuverture = 0U;
DateTime DebutAcces = {0000, 6, 15, 9, 00, 00};
DateTime FinAcces = {0000, 9, 15, 20, 00, 00};

// Variable stuff
const int PINTIMEOUT = 2000;
unsigned long rfidcount = 0, pincount = 0;
unsigned long lastKey, lastPin, lastRfid;
bool LastExitButtonState = false;
String Saisie = "";

WIEGAND Wg;

bool isAccessTimeValid()
{
  return true;
  // Vérifier si la date actuelle est dans la plage d'accès
  bool inDateRange = (Local.month() > DebutAcces.month() ||
                      (Local.month() == DebutAcces.month() && Local.day() >= DebutAcces.day())) &&
                     (Local.month() < FinAcces.month() ||
                      (Local.month() == FinAcces.month() && Local.day() <= FinAcces.day()));

  // Vérifier si l'heure actuelle est dans la plage d'accès
  bool inTimeRange = (Local.hour() > DebutAcces.hour() ||
                      (Local.hour() == DebutAcces.hour() && Local.minute() >= DebutAcces.minute())) &&
                     (Local.hour() < FinAcces.hour() ||
                      (Local.hour() == FinAcces.hour() && Local.minute() <= FinAcces.minute()));

  DEBUGLN(F("Trigger plage d'accès"));
  DEBUG(F("Date : "));
  DEBUG(inDateRange);
  DEBUG(F("Heure : "));
  DEBUG(inTimeRange);

  // Retourner vrai si les deux conditions sont remplies
  return inDateRange && inTimeRange;
}

void generateBackupCode()
{

  // Extraire l'année et le mois
  const int year = Local.year() % 100; // Année sur 2 chiffres
  const int month = Local.month();

  // Calculer A = abs(YY - MM)
  const int A = abs(year - month);

  // Construire les chaînes de caractères
  char Y[3];     // Année (2 chiffres)
  char M[3];     // Mois (2 chiffres)
  char A_str[3]; // Différence (2 chiffres)

  // Formater les valeurs avec `snprintf`
  snprintf(Y, sizeof(Y), "%02d", year);      // Format YY
  snprintf(M, sizeof(M), "%02d", month);     // Format MM
  snprintf(A_str, sizeof(A_str), "%02d", A); // Format A

  // Construire le code final
  char code[7] = {
      Y[0],
      A_str[0],
      M[0],
      Y[1],
      A_str[1],
      M[1],
      '\0'};

  // Afficher le code dans le moniteur série
  DEBUG(F("Code de secours mensuel : "));
  DEBUGLN(code);

  strncpy(BackupCode, code, sizeof(code));
}

void parseDate(const char *date, DateTime &dt)
{
  // Format attendu : DD/MM
  int day, month;
  sscanf(date, "%d/%d", &day, &month);

  dt = DateTime{dt.year(),
                (uint8_t)month,
                (uint8_t)day,
                dt.hour(),
                dt.minute(),
                dt.second()};
}

void parseTime(const char *time, DateTime &dt)
{
  // Format attendu : HH:MM
  int hour, minute;
  sscanf(time, "%d:%d", &hour, &minute);

  dt = DateTime{dt.year(),
                dt.month(),
                dt.day(),
                (uint8_t)hour,
                (uint8_t)minute,
                dt.second()};
}

void parseParam(const char *param)
{
  // Copie locale pour éviter de modifier la trame originale
  char buffer[50];
  strncpy(buffer, param, sizeof(buffer));
  buffer[sizeof(buffer) - 1] = '\0'; // Sécuriser le buffer

  // Découpage de la trame par lignes
  char *line = strtok(buffer, "\n");
  while (line != NULL)
  {
    // Découper chaque ligne en clé=valeur
    char *delimiter = strchr(line, '=');
    if (delimiter != NULL)
    {
      *delimiter = '\0'; // Séparer la clé et la valeur
      char *key = line;
      char *value = delimiter + 1;

      DEBUG(F("Param : "));
      DEBUG(key);
      DEBUG(F(" = "));
      DEBUGLN(value);

      // Mapper les clés aux variables
      if (strcmp(key, "temps_ouverture/entree") == 0)
      {
        OpenDelayCode = atoi(value);
      }
      else if (strcmp(key, "temps_ouverture/sortie") == 0)
      {
        OpenDelayButton = atoi(value);
      }
      else if (strcmp(key, "acces_piscine/date_debut") == 0)
      {
        parseDate(value, DebutAcces);
      }
      else if (strcmp(key, "acces_piscine/date_fin") == 0)
      {
        parseDate(value, FinAcces);
      }
      else if (strcmp(key, "acces_piscine/heure_debut") == 0)
      {
        parseTime(value, DebutAcces);
      }
      else if (strcmp(key, "acces_piscine/heure_fin") == 0)
      {
        parseTime(value, FinAcces);
      }
    }

    // Passer à la ligne suivante
    line = strtok(NULL, "\n");
  }
}

void clearSaisie()
{
  // const size_t sizeOfSaisie = sizeof(Saisie);
  // strncpy(Saisie, "", sizeOfSaisie);
  Saisie = "";
}

void handleKeypadAndRfidInput()
{

  const size_t sizeOfSaisie = sizeof(Saisie);

  // Gestion du TimeOut de saisie
  if (millis() - lastKey >= PINTIMEOUT)
  {
    if (sizeOfSaisie > 0)
    {
      clearSaisie();
    }
  }

  // Gestion de la saisie / lecture de badge
  if (Wg.available())
  {

    const unsigned long wcode = Wg.getCode();
    const int wtype = Wg.getWiegandType();

    // Emergency code
    if (Saisie == EMERGENCY_CODE)
    {
      DEBUGLN(F("INFO > Déverrouillage d'urgence du portail"));
      TempoOuverture = OpenDelayCode;
      clearSaisie();
      return;
    }

    // Backup code
    if (Saisie == BackupCode || (rtcFail && Saisie == F("123456")))
    {
      DEBUGLN(F("INFO > Déverrouillage du portail code de secours"));
      TempoOuverture = OpenDelayCode;
      clearSaisie();
      return;
    }

    // Keypad
    if (wtype == 8)
    {

      // Numkeys
      if ((wcode >= 0) && (wcode <= 9))
      {
        Saisie += wcode;
        lastKey = millis();
        DEBUG(F(" | Saisie = "));
        DEBUG(Saisie);

        // Enter Key
      }
      else if (wcode == 13)
      {
        lastKey = millis();
        String msg;
        msg = "{\"code\": \"";
        msg += Saisie;
        msg += "\" ,\"type\": \"Saisie\"}";
        DEBUG(F(" | Saisie = "));
        DEBUG(Saisie);
        if (millis() - lastPin > PINLIMIT)
        {
          pincount++;
          MqttClient.publish(PT_Wiegand, msg.c_str());
          DEBUG(F("  -> MQTT sent"));
          clearSaisie();
          lastPin = millis();
        }
        else
        {
          MqttClient.publish(PT_Wiegand, "{ \"type\": \"pinratelimit\" }");
          DEBUG(F("  -> RATELIMITED"));
          clearSaisie();
        }

        // Escape Key
      }
      else if (wcode == 27)
      {
        clearSaisie();
        lastKey = millis();
        DEBUG(F(" | Saisie = "));
        DEBUG(Saisie);
      }

      // RFID card
    }
    else if (wtype == 26 || wtype == 34)
    {

      String msg;
      String HEXcode = String(wcode, HEX);
      HEXcode.toUpperCase();

      msg = "{\"code\":\"";
      msg += HEXcode;
      msg += "\",\"type\": \"rfid\"}";
      if (millis() - lastRfid > RFIDLIMIT)
      {
        rfidcount++;
        MqttClient.publish(PT_Wiegand, msg.c_str());
        DEBUG(F("  -> MQTT sent"));
        DEBUG(msg);
        lastRfid = millis();
      }
      else
      {
        MqttClient.publish(PT_Wiegand, "{ \"type\": \"rfidratelimit\" }");
        DEBUG(F("  -> RATELIMITED"));
      }
    }
    Serial.println();
  }
}

void monitorExitButton()
{

  bool pushed = digitalRead(PIN_EXIT_BUTTON) == LOW;

  // Trigger
  if (LastExitButtonState != pushed)
  {
    if (pushed)
    {
      Serial.println(F("INFO > Bouton de sortie enfoncé"));
      // MqttClient.publish(PT_Button, "1");
      TempoOuverture = OpenDelayButton;
    }
    else
    {
      Serial.println(F("INFO > Bouton de sortie relâché"));
      // MqttClient.publish(PT_Button, "0");
    }
    LastExitButtonState = pushed;
  }
  delay(100); // Debounce time
}

void controlDoorLock(const bool ft1s)
{
  if (TempoOuverture > 0 && ft1s)
    TempoOuverture--;

  if (TempoOuverture >= OpenDelayCode && digitalRead(PIN_LOCK_RELAY) == !LOW)
  {
    digitalWrite(PIN_LED_KEYPAD, LOW);
    digitalWrite(PIN_LOCK_RELAY, LOW);
    Serial.println(F("INFO > Déverrouillage du portail"));
  }
  else if (TempoOuverture <= 0 && digitalRead(PIN_LOCK_RELAY) == !HIGH)
  {
    digitalWrite(PIN_LED_KEYPAD, HIGH);
    digitalWrite(PIN_LOCK_RELAY, HIGH);
    Serial.println(F("INFO > Verrouillage du portail"));
  }
}
