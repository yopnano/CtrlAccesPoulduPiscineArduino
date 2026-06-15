
#include "ClavierRFID.h"

void ClavierRFID::begin(int pinD0, int pinD1)
{
    wg.begin(pinD0, pinD1); // Init Wiguand keypad
}

bool ClavierRFID::available()
{

    // Gestion du TimeOut de saisie
    if (hasData() && millis() - lastKey >= PINTIMEOUT)
    {
        Serial.println(F("LOG:Timeout de saisie au clavier"));
        clearData();
    }

    // Detection de données sur le bus Wiegand
    if (wg.available())
    {
        const unsigned long wcode = wg.getCode();
        const int wtype = wg.getWiegandType();

        switch (static_cast<wType>(wtype))
        {
        case wType::RFID_26:
        case wType::RFID_34:
            /* Tag RFID détecté */
            return processRFID(wcode);

        case wType::KEYPAD:
            /* Saisie au clavier détectée */
            return processKeyboard(wcode);

        default:
            clearData();
            break;
        }
    }
    return false;
}


bool ClavierRFID::processRFID(const unsigned long wcode)
{
    // Anti bruteforce
    if (millis() - lastRfid > RFIDLIMIT)
    {
        rfidcount++;
        lastRfid = millis();

        // Conversion directe du code en Hexadécimal majuscule (sans allouer de String)
        snprintf(data, sizeof(data), "%lX", wcode);
        dataIndex = strlen(data); // Met à jour le curseur

        Serial.print(F("LOG:Tag RFID detecte UID = "));
        Serial.println(data);

        return true;
    }

    return false;
}

bool ClavierRFID::processKeyboard(const unsigned long wkey)
{
    switch (static_cast<wKey>(wkey))
    {
    case wKey::ESCAPE:
        if (hasData())
        {
            Serial.println(F("LOG:Touche ESC : Effacement saisie"));
            clearData();
        }
        break;

    case wKey::ENTER:
        // Anti bruteforce
        if (millis() - lastPin > PINLIMIT)
        {
            pincount++;
            lastPin = millis();

            Serial.print(F("LOG:Touche Enter : Validation Code = "));
            Serial.println(data);

            return true;
        }
        break;

    default:
        // 'wkey' étant de type unsigned, wkey >= 0 est toujours vrai, donc vérification wkey <= 9 suffit
        if (wkey <= 9)
        {
            // Protection contre les débordements de tampon (Buffer Overflow)
            if (dataIndex < sizeof(data) - 1) 
            {
                data[dataIndex] = '0' + wkey; // Conversion mathématique du chiffre en char ASCII
                dataIndex++;
                data[dataIndex] = '\0'; // Clôture systématique de la chaîne
            }
            lastKey = millis();
        }
        break;
    }

    return false;
}