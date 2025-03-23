/*
 * File: NTPServer.h
 * Description:
 *   NTP server implementation.
 * Authors: Mooneer Salem <mooneer@gmail.com>
 *			Dan Quigley <danq@quigleys.us>
 * License: New BSD License
 */
 
 #ifndef NTP_SERVER_H
 #define NTP_SERVER_H
 
 #define NTP_PORT 123
 
 #include <MyITimeSource.h>
 
 #if defined(ARDUINO)
 
 #include <SPI.h>
 #include <Ethernet.h>
 #include <EthernetUdp.h>
 
 #include "NTPPacket.h"
 #include "NTPServer.h"
 #endif
 
 class MyNtpServer
 {
 public:
     MyNtpServer(MyITimeSource &source);
     
      // Begins listening for NTP requests.
     void beginListening(void);
     
      // Processes a single NTP request.
     bool processOneRequest(void);
 
     // Returns last NTP client IP address.
     IPAddress getLastClientIP();
 
 private:
     MyITimeSource &timeSource_;
     EthernetUDP timeServerPort_;
     IPAddress lastClient_;
 };
 
 #endif // NTP_SERVER_H
 