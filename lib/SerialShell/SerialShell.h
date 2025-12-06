#ifndef SERIALSHELL_H
#define SERIALSHELL_H

#include <Arduino.h>
#include "SerialDebug.h" // pour LogLevel et DebugLogger*

#define MAX_COMMANDS 10 // Nombre maximum de commandes

typedef void (*CommandCallback)(int argc, char *argv[]);

struct Command
{
    const char *shortName;
    const char *longName;
    String description;
    CommandCallback callback;
};

class SerialShell
{
public:
    /// @brief Serial Shell for program interraction
    SerialShell() : serialPort(nullptr), logger(nullptr), commandCount(0) {}

    /// @brief Initialize shell session
    /// @param serial Stream ex: Serial, Serial1, softwareSerial
    /// @param dbg Debug instance address ex: &Debug
    void begin(Stream &serial, SerialDebug *dbg = nullptr)
    {
        serialPort = &serial;
        logger = dbg;
        shellInstance = this;
        registerNativeCommands();

        serialPort->println(F("Shell Initilalisé."));
        serialPort->println(F("Tapper -h ou --help pour afficher la liste des commandes."));
    }

    void addCommand(const char *shortName, const char *longName,
                    const char *description,
                    CommandCallback cb)
    {
        if (commandCount < MAX_COMMANDS)
        {
            commands[commandCount++] = {shortName, longName, String(description), cb};
        }
    }

    void addCommand(const char *shortName, const char *longName,
                    const __FlashStringHelper *description,
                    CommandCallback cb)
    {
        if (commandCount < MAX_COMMANDS)
        {
            commands[commandCount++] = {shortName, longName, String(description), cb};
        }
    }

    void update()
    {
        if (!serialPort)
            return;

        if (serialPort->available())
        {
            String line = serialPort->readStringUntil('\n');
            line.trim();
            serialPort->println(line);
            if (line.length() > 0)
                executeCommand(line);
        }
    }

private:
    Stream *serialPort;
    SerialDebug *logger;
    Command commands[MAX_COMMANDS];
    uint8_t commandCount;

    void registerNativeCommands()
    {

        // Add help command
        addCommand("-h", "--help", F("List all commands"), [](int argc, char *argv[])
                   {
            Serial.print(shellInstance->commandCount);
            Serial.println(F(" Available commands:"));
            for (int i = 0; i < shellInstance->commandCount; i++) {
                Serial.print("  ");
                if (shellInstance->commands[i].shortName[0]) {
                    Serial.print(shellInstance->commands[i].shortName);
                    Serial.print(", ");
                }
                Serial.print(shellInstance->commands[i].longName);
                Serial.print(" : ");
                Serial.println(shellInstance->commands[i].description);
            } });

        // Add Logguer command
        if (logger != nullptr)
        {
            addCommand("", "--loglevel", F("Get or Set current log level (0=Error..4=Verbose)"), [](int argc, char *argv[])
                       {
                if (!shellInstance->logger) {
                    Serial.println(F("No SerialDebug attached"));
                    return;
                }
                if (argc == 0) {
                    Serial.print(F("Current log level: "));
                    Serial.println((int)shellInstance->logger->getLevel());
                    return;
                }
                int lvl = atoi(argv[0]);
                if (lvl < 0) lvl = 0;
                if (lvl > 4) lvl = 4;
                shellInstance->logger->setLevel((LogLevel)lvl);
                Serial.print(F("Log level set to "));
                Serial.println(lvl); });
        }
    }

    void executeCommand(const String &line)
    {
        char buf[64];
        line.toCharArray(buf, sizeof(buf));

        // Découpage en tokens
        char *argv[5];
        int argc = 0;
        char *token = strtok(buf, " ");
        while (token && argc < 5)
        {
            argv[argc++] = token;
            token = strtok(nullptr, " ");
        }

        if (argc == 0)
            return;

        for (int i = 0; i < commandCount; i++)
        {
            if ((commands[i].shortName[0] && strcmp(argv[0], commands[i].shortName) == 0) ||
                strcmp(argv[0], commands[i].longName) == 0)
            {
                commands[i].callback(argc - 1, &argv[1]);
                return;
            }
        }
        Serial.println(F("Unknown command, type --help"));
    }

    // Trick pour que les lambdas puissent accéder à la classe
    static SerialShell *shellInstance;
};

SerialShell *SerialShell::shellInstance = nullptr;

#endif
