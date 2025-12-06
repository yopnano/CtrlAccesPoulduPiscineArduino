
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
        log(LogLevel::Verbose, F("Timeout de saisie"));
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
        {
            /* Tag RFID détecté */
            return processRFID(wcode);
        }

        case wType::KEYPAD:
        {
            /* Saisie au clavier détectée */
            return processKeyboard(wcode);
        }

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

        data = String(wcode, HEX);
        data.trim();
        data.toUpperCase();

        String msg = F("Tag RFID détecté ");
        msg += F("UID = ");
        msg += data;
        log(LogLevel::Info, msg);

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
            log(LogLevel::Info, F("Touche ESC : Effacement saisie"));
            clearData();
        }
        break;

    case wKey::ENTER:

        // Anti bruteforce
        if (millis() - lastPin > PINLIMIT)
        {
            pincount++;
            lastPin = millis();

            data.trim();
            data.toUpperCase();

            String msg = F("Touche Enter : Validation ");
            msg += F("Code = ");
            msg += data;
            log(LogLevel::Info, msg);

            return true;
        }
        break;

    default:

        if ((wkey >= 0) && (wkey <= 9))
        {
            data += wkey;
            lastKey = millis();

            log(LogLevel::Verbose, "Saisie : " + data);
        }

        break;
    }

    return false;
}