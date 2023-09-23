#include "DataLogger.h"
#include <SD.h>
#include <SPI.h>

const int SD_EN_PIN = 37;
const int SD_CD_PIN = 36;
const int SD_CS_PIN = 53;
const int SD_SCLK_PIN = 52;
const int SD_DO_PIN = 50;
const int SD_DI_PIN = 51;


DataLogger* DataLogger::_instance = nullptr;

DataLogger::DataLogger() {
    if (_instance == nullptr) {
        _instance = this;
    } else {
        ERROR_LOG(F("DataLogger::DataLogger: instance already exists"));
        return;
    }
    _logFileName[0] = '\0';
    _logSequenceNumber = 0;
    // initialize the SD card
    if (!SD.begin(SD_CS_PIN)) {
        ERROR_LOG(F("DataLogger::DataLogger: SD card initialization failed!"));
        return;
    }
    INFO_LOG(F("SD card initialized."));
    // detect if card is formatted for the robot data logger
    // get or create the file "log/sequence_number.txt" file. If it is present, read the sequence number from it. 
    // If it is not present, create it and write the sequence number 0 to it.
    if (!SD.exists("log")) {
        DEBUG_LOG(F("DataLogger::DataLogger: log directory does not exist, creating it"));
        if (!SD.mkdir("log")) {
            ERROR_LOG(F("DataLogger::DataLogger: could not create log directory"));
            return;
        }
    }
    if (!SD.exists("log/sequence_number.txt")) {
        DEBUG_LOG(F("DataLogger: sequence number file does not exist, creating it"));
        File sequenceNumberFile = SD.open("log/sequence.txt", FILE_WRITE);
        if (!sequenceNumberFile) {
            ERROR_LOG(F("DataLogger: could not create sequence number file"));
            return;
        }
        sequenceNumberFile.println("0");
        sequenceNumberFile.close();
        // double check the file was created
        if (!SD.exists("log/sequence.txt")) {
            ERROR_LOG(F("DataLogger: could not create sequence number file"));
            return;
        }   
    }
    File sequenceNumberFile = SD.open("log/sequence.txt", FILE_READ);
    if (sequenceNumberFile) {
        String sequenceNumberString = sequenceNumberFile.readStringUntil('\n');
        sequenceNumberFile.close();
        _logSequenceNumber = sequenceNumberString.toInt();
        sprintf(_buffer, "DataLogger: initial sequence number is %d, incrementing it", _logSequenceNumber);
        INFO_LOG(_buffer);
        _logSequenceNumber++;
        // roll over if we hit 1000
        if (_logSequenceNumber > 999) {
            _logSequenceNumber = 0;
        }
        SD.remove("log/sequence.txt");
        sequenceNumberFile = SD.open("log/sequence.txt", FILE_WRITE);
        if (!sequenceNumberFile) {
            ERROR_LOG(F("DataLogger: could not open sequence number file for writing"));
            return;
        }
        sequenceNumberFile.println(_logSequenceNumber);
        sequenceNumberFile.close();
    } else {
        ERROR_LOG(F("DataLogger: could not open sequence number file"));
        return;
    }
    // now that we have the sequence number, create the log file
    sprintf(_logFileName, "log/log_%d.txt", _logSequenceNumber);
    if (SD.exists(_logFileName)) {
        WARNING_LOG(F("DataLogger: log file already exists, removing it"));
        SD.remove(_logFileName);
    }
    File logFile = SD.open(_logFileName, FILE_WRITE);
    if (!logFile) {
        ERROR_LOG(F("DataLogger: could not create log file"));
    } else {
        logFile.println(F("DataLogger: log file created"));
        logFile.close();
    }
}

DataLogger::~DataLogger() {
    // TODO Auto-generated destructor stub
}

void DataLogger::loop() {

}

void DataLogger::log(LogType logType, const char* message) {
    char buffer[256];
    switch (logType) {
        case DEBUG:
            sprintf(buffer, "DEBUG: %s", message);
            break;
        case INFO:
            sprintf(buffer, "INFO: %s", message);
            break;
        case WARNING:
            sprintf(buffer, "WARNING: %s", message);
            break;
        case ERROR:
            sprintf(buffer, "ERROR: %s", message);
            break;
    }
    Serial.println(buffer);
    if (_logFileName[0] != '\0') {
        File logFile = SD.open(_logFileName, FILE_WRITE);
        if (logFile) {
            logFile.println(buffer);
            logFile.close();
        }
    } 
}

void DataLogger::log(LogType logType, const __FlashStringHelper* message) {
    strcpy_P(_buffer, (const char*)message);
    log(logType, _buffer);
}