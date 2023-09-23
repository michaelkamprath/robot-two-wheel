#ifndef __DATALOGGER_H__
#define __DATALOGGER_H__
#include <Arduino.h>


typedef enum {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3
} LogType;


#define DEBUG_LOG(message) DataLogger::getInstance()->debug(message)
#define INFO_LOG(message) DataLogger::getInstance()->info(message)
#define WARNING_LOG(message) DataLogger::getInstance()->warning(message)
#define ERROR_LOG(message) DataLogger::getInstance()->error(message)

class DataLogger {
private:
    char _buffer[256];
    char _commonBuffer[256];

    int _logSequenceNumber;
    char _logFileName[20];

protected:
    void log(LogType, const char* message);
    void log(LogType, const __FlashStringHelper* message);

    static DataLogger* _instance;
public:
    static void init()                                  { new DataLogger(); }
    static DataLogger* getInstance()                    { return _instance; }

    static char* commonBuffer()                         { return _instance->_commonBuffer; }

    DataLogger();
    virtual ~DataLogger();

    void loop();

    void debug(const char* message)                     { log(DEBUG, message); }
    void info(const char* message)                      { log(INFO, message); }
    void warning(const char* message)                   { log(WARNING, message); }
    void error(const char* message)                     { log(ERROR, message); }
    void debug(const __FlashStringHelper* message)      { log(DEBUG, message); }
    void info(const __FlashStringHelper* message)       { log(INFO, message); }
    void warning(const __FlashStringHelper* message)    { log(WARNING, message); }
    void error(const __FlashStringHelper* message)      { log(ERROR, message); }

};

#endif // __DATALOGGER_H__