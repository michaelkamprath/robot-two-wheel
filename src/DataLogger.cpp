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
    
    if (!SD.begin(SD_CS_PIN)) {
        Serial.println(F("DataLogger::DataLogger: SD card initialization failed!"));
        return;
    }
    INFO_LOG(F("SD card initialized."));


}

DataLogger::~DataLogger() {
    // TODO Auto-generated destructor stub
}

void DataLogger::loop() {

}

void DataLogger::log(LogType logType, const char* message) {

    switch (logType) {
        case DEBUG:
            Serial.print(F("DEBUG: "));
            break;
        case INFO:
            Serial.print(F("INFO: "));
            break;
        case WARNING:
            Serial.print(F("WARNING: "));
            break;
        case ERROR:
            Serial.print(F("ERROR: "));
            break;
    }
    Serial.println(message);
}

void DataLogger::log(LogType logType, const __FlashStringHelper* message) {
    strcpy_P(_buffer, (const char*)message);
    log(logType, _buffer);
}