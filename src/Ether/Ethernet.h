#ifndef ETHER_H
#define ETHER_H

#pragma once

#include <Ethernet.h>
#include "Debug.hpp"

extern const byte ETH_ADDR_MAC[];     // Adresse MAC du shield Ethernet
extern const IPAddress ETH_ADDR_IPV4; // Adresse IP sur le réseau

extern EthernetClient EthClient;

/**
 * @brief Initialise la connexion Ethernet et attend une adresse IP valide.
 * Configure l'Ethernet avec une adresse MAC et une adresse IP statique.
 * Affiche un message de débogage si la connexion échoue ou réussit.
 */
void initializeEthernet();


#endif
