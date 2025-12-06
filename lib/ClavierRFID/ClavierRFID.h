#ifndef ClavierRFID_h
#define ClavierRFID_h

#include <Arduino.h>
#include <Wiegand.h>
#include "SerialDebug.h" // Optionnel, pour le logging

class ClavierRFID
{

public:
    ClavierRFID(SerialDebug *logger = nullptr) : debug(logger),
                                                 rfidcount(0),
                                                 pincount(0),
                                                 lastKey(0),
                                                 lastPin(0),
                                                 lastRfid(0),
                                                 data(""),
                                                 wg() {}

    ~ClavierRFID() = default;

    void begin(int pinD0, int pinD1);

    /// @brief Traitement des données
    /// @return True si des données sont disponible
    bool available();

    /// @brief Renvoie les données lues
    /// @return String des données
    const String read() noexcept
    {
        const String d = data;
        clearData();
        return d;
    }

    /// @brief Indique si des données sont présentes
    /// @return bool données présente ou non
    const bool hasData() const noexcept
    {
        return data.length() > 0;
    }

private:
    // Delais anti-bruteforce
    static constexpr unsigned int RFIDLIMIT = 2000;  ///< Limite de délai entre deux lectures de badge RFID (en millisecondes).
    static constexpr unsigned int PINLIMIT = 2000;   ///< Limite de délai entre deux saisies de code PIN (en millisecondes).
    static constexpr unsigned int PINTIMEOUT = 8000; ///< Temps limite pour saisir un code PIN (en millisecondes).

    /// @brief Mapping des codes du clavier
    enum class wKey : uint8_t
    {
        ENTER = 13,
        ESCAPE = 27
    };

    /// @brief Type d'info (Lecture tag RFID ou saisie au clavier)
    enum class wType : uint8_t
    {
        KEYPAD = 8,
        RFID_26 = 26,
        RFID_34 = 34
    };

    // Compteur et Timestamps
    SerialDebug *debug;
    unsigned long rfidcount; ///< Compteur des lectures RFID.
    unsigned long pincount;  ///< Compteur des saisies de code PIN.
    unsigned long lastKey;   ///< Temps du dernier appui sur une touche.
    unsigned long lastPin;   ///< Temps de la dernière saisie de code PIN.
    unsigned long lastRfid;  ///< Temps de la dernière lecture RFID.

    String data; ///< Chaîne contenant la saisie courante sur le clavier.

    WIEGAND wg;

    /// @brief Traitement des données du tag RFID
    /// @param wcode ID du tag
    /// @return Bool si le traitement à eu lieu
    bool processRFID(const unsigned long wcode);

    /// @brief Traitement des données saisies au clavier
    /// @param wkey Code de la touche
    /// @return Bool si le traitement à eu lieu
    bool processKeyboard(const unsigned long wkey);

    /// @brief Fonction utilitaire de purge du tampon de données
    void clearData() noexcept
    {
        // L'onjet String n'a pas de méthode clear(), j'ai vérifieé dans la documentation Arduino officiel
        data = "";
    }

    void log(LogLevel level, const String &msg)
    {
        if (debug)
        {
            debug->log(level, msg);
        }
    }
};

#endif