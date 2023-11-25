#ifndef __DATALOGGER_H__
#define __DATALOGGER_H__
#include <Arduino.h>
#include "DataTable.h"

#define DEBUG_LOG(message) DataLogger::getInstance()->debug(message)
#define INFO_LOG(message) DataLogger::getInstance()->info(message)
#define WARNING_LOG(message) DataLogger::getInstance()->warning(message)
#define ERROR_LOG(message) DataLogger::getInstance()->error(message)

class DataLogger {
public:
    typedef enum {
        NONE = -1,
        DEBUG = 0,
        INFO = 1,
        WARNING = 2,
        ERROR = 3
    } LogType;

private:
    char _buffer[256];
    char _commonBuffer[256];

    int _logSequenceNumber;
    char _logFileName[20];
    LogType _logLevel;


protected:
    static DataLogger* _instance;

    const String& get_log_prefix(LogType logType) const;
public:
    static void init(LogType log_level = DEBUG)         { new DataLogger(log_level); }
    static DataLogger* getInstance()                    { return _instance; }

    static char* commonBuffer()                         { return _instance->_commonBuffer; }

    DataLogger(LogType log_level);
    virtual ~DataLogger();

    void loop();
    void log(LogType, const char* message);
    void log(LogType, const __FlashStringHelper* message);
    void log(LogType logType, const String& message);
    void log_data_table(const DataTable<double>& dataTable, DataTable<double>::FieldFormatter formatter = [](double value, int col_num) -> String {
        return String(value);
    });
    void log_data_table(const DataTable<int>& dataTable, DataTable<int>::FieldFormatter formatter = [](int value, int col_num) -> String {
        return String(value);
    });

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