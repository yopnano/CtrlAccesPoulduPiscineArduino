#ifndef DEBUG_LOGGER_H
#define DEBUG_LOGGER_H

#include <Arduino.h>

enum class LogLevel : uint8_t
{
    Error = 0,
    Warning = 1,
    Info = 2,
    Debug = 3,
    Verbose = 4
};

class SerialDebug
{
public:
    SerialDebug() : serialPort(nullptr),
                    filterLevel(LogLevel::Info) {}

    /// @brief Initialise le logger avec un port série et un niveau de filtre
    /// @param serial Pointeur vers l'objet Stream
    /// @param level Niveau de debogage initial
    void begin(Stream &serial, LogLevel level = LogLevel::Info)
    {
        serialPort = &serial;
        filterLevel = level;
    }

    /// @brief Change le niveau de filtrage
    void setLevel(LogLevel level)
    {
        filterLevel = level;
    }

    LogLevel getLevel() const { return filterLevel; }


    /// @brief Afficher un message de log
    /// @tparam T Accepte tout type compatible avec String::operator+=
    /// @param msg Message à concaténer au log
    template <typename T>
    void log(LogLevel level, const T &msg)
    {
        // Condition d'affichage du log
        if (serialPort && level <= filterLevel)
        {
            printPrefix(level);            
            concat(msg);
            message += ".";
            serialPort->println(message);
            message = "";
        }
    }

    /// @brief Concaténer message de log
    /// @tparam T Accepte tout type compatible avec String::operator+=
    /// @param msg Message à concaténer au log
    template <typename T>
    void concat(const T &msg)
    {
        if (message.length() > 0)
            message += " ";

        message += msg; // Concatène directement
    }

private:
    Stream *serialPort;
    LogLevel filterLevel;
    String message;

    void printPrefix(LogLevel level)
    {
        if (!serialPort)
            return;
        serialPort->print('[');
        switch (level)
        {
        case LogLevel::Error:
            serialPort->print(F("Error"));
            break;
        case LogLevel::Warning:
            serialPort->print(F("Warning"));
            break;
        case LogLevel::Info:
            serialPort->print(F("Info"));
            break;
        case LogLevel::Debug:
            serialPort->print(F("Debug"));
            break;
        case LogLevel::Verbose:
            serialPort->print(F("Verbose"));
            break;
        }
        serialPort->print(F("] >> "));
    }
};

#endif
