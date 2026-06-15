#ifndef ClavierRFID_h
#define ClavierRFID_h

#include <Arduino.h>
#include <Wiegand.h>
#include "SerialDebug.h" // Optionnel, pour le logging

class ClavierRFID
{

public:

    ClavierRFID() : rfidcount(0),
                    pincount(0),
                    lastKey(0),
                    lastPin(0),
                    lastRfid(0),
                    dataIndex(0),
                    wg() 
    {
        clearData();
    }

    ~ClavierRFID() = default;

    void begin(int pinD0, int pinD1);

    /// @brief Traitement des données
    /// @return True si des données sont disponible
    bool available();

    /// @brief Remplit le buffer fourni avec les données lues et purge le lecteur
    /// @param buffer Tableau de destination (ex: codeEnAttente)
    /// @param maxLength Taille maximale du tableau pour éviter les débordements
    void read(char* buffer, size_t maxLength) noexcept
    {
        strncpy(buffer, data, maxLength);
        buffer[maxLength - 1] = '\0'; // Sécurité absolue de fin de chaîne
        clearData();
    }

    /// @brief Indique si des données sont présentes
    /// @return bool données présente ou non
    const bool hasData() const noexcept
    {
        return dataIndex > 0;
    }

private:
    // Delais anti-bruteforce
    static constexpr unsigned int RFIDLIMIT = 2000;  ///< Limite de délai entre deux lectures de badge RFID
    static constexpr unsigned int PINLIMIT = 2000;   ///< Limite de délai entre deux saisies de code PIN
    static constexpr unsigned int PINTIMEOUT = 8000; ///< Temps limite pour saisir un code PIN

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
    unsigned long rfidcount; ///< Compteur des lectures RFID.
    unsigned long pincount;  ///< Compteur des saisies de code PIN.
    unsigned long lastKey;   ///< Temps du dernier appui sur une touche.
    unsigned long lastPin;   ///< Temps de la dernière saisie de code PIN.
    unsigned long lastRfid;  ///< Temps de la dernière lecture RFID.

    char data[24]; ///< Buffer interne ultra-léger remplaçant l'objet String
    byte dataIndex; ///< Curseur de remplissage du buffer

    WIEGAND wg;

    /// @brief Traitement des données du tag RFID
    bool processRFID(const unsigned long wcode);

    /// @brief Traitement des données saisies au clavier
    bool processKeyboard(const unsigned long wkey);

    /// @brief Fonction utilitaire de purge du tampon de données
    void clearData() noexcept
    {
        data[0] = '\0'; // On met le caractère de fin en première position
        dataIndex = 0;  // On remet le curseur à zéro
    }
};

#endif