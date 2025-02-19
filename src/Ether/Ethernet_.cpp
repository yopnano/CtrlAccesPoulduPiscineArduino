#include "Ether\Ethernet.h"

const byte ETH_ADDR_MAC[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
const IPAddress ETH_ADDR_IPV4(10, 42, 0, 5);

EthernetClient EthClient;

void initializeEthernet()
{

  // Start Ethernet connexion
  // Ethernet.begin(ETH_ADDR_MAC, ETH_ADDR_IPV4);
  Ethernet.begin(ETH_ADDR_MAC, ETH_ADDR_IPV4);

  while (Ethernet.localIP() == IPAddress(0, 0, 0, 0))
  {
    DEBUGFN(F("ERROR > Échec de la connexion Ethernet"));
    delay(1000);
  }

  DEBUGLN(F("DEBUG > Connexion Ethernet établie"));
  DEBUG(F("DEBUG > Adresse : "));
  DEBUGLN(Ethernet.localIP());
}