#ifndef MYUTILS_H
#define MYUTILS_H

#pragma once

#include <Arduino.h>

/**
 * @brief Convertit une valeur booléenne en une chaîne de caractères.
 *
 * Cette fonction retourne une chaîne constante représentant la valeur booléenne.
 * "1" pour `true`, et "0" pour `false`.
 *
 * @param value La valeur booléenne à convertir.
 * @return const char* Une chaîne constante représentant la valeur ("1" ou "0").
 */
const char *Bool_To_Char(const bool value);

/**
 * @brief Convertit une valeur booléenne en une chaîne de caractères et la stocke dans un buffer.
 *
 * Cette fonction écrit la représentation en chaîne de caractères ("1" ou "0") de la valeur booléenne
 * dans le buffer spécifié. Le buffer doit être suffisamment grand pour contenir au moins deux caractères.
 *
 * @param value La valeur booléenne à convertir.
 * @param buffer Un pointeur vers le buffer où stocker la chaîne de caractères résultante.
 */
void Bool_To_Char(const bool value, char *buffer);

// Gestion de temporisation
struct Tempo
{
private:
    bool _ft;                 // Front montant (généré chaque période)
    bool _clock;              // Clock qui change état à chaque front
    unsigned long _cpt;       // Compteur du nombre de periodes effectuées
    unsigned long _oldMillis; // Ancienne valeur de millis() pour la gestion du temps
    void (*callback)();       // Fonction de rappel

public:
    /// @brief Période de la tempo en millisecondes
    const unsigned long period;

    /// @brief Signal de front, pulsation sur un cycle
    const bool ft() const { return _ft; }

    /// @brief Signal de front, pulsation sur n cycle
    /// @param mod Diviseur (par exemple, 10 pour 1 front toutes les 10 périodes)
    /// @return true si le nombre de front est un multiple de mod et front actif
    const bool ft(unsigned long mod) const { return _ft && _cpt % mod == 0; }

    /// @brief Signal d'horloge inversion à chaque front
    const bool clock() const { return _clock; }

    /// @brief Signal d'horloge toutes les n périodes
    /// @param mod Diviseur (par exemple, 10 pour 1 période sur 10 à true)
    /// @return true tant que le nombre de front est un multiple de mod
    const bool clock(unsigned long mod) const { return _cpt % mod == 0; }

    /// @brief Compteur du nombre de périodes écoulées
    const unsigned long cpt() const { return _cpt; }

    /// @brief Temporisation
    /// @param p Periode de la temporisation (ms)
    /// @param cb Fonction de rappel (optionnel)
    Tempo(unsigned long p, void (*cb)() = nullptr)
        : _ft(false), _clock(false), _cpt(0U), _oldMillis(0U), callback(cb), period(p) {}

    // Gestion de la tempo
    void run()
    {
        unsigned long currentMillis = millis();

        // Vérifier si la période est écoulée
        if (currentMillis - _oldMillis >= period)
        {
            _cpt++;                     // Incrémentation du compteur de période
            _ft = true;                 // Générer le front montant chaque période
            _clock = !_clock;           // Inverser le clock à chaque front
            _oldMillis = currentMillis; // Réinitialiser l'ancienne valeur de millis pour commencer un nouveau cycle

            // Appeler la fonction de rappel si elle est définie
            if (callback)
            {
                callback();
            }
        }
        else
            _ft = false;
    }

    // Méthode pour définir un callback
    void setCallback(void (*cb)())
    {
        callback = cb;
    }
};

#endif