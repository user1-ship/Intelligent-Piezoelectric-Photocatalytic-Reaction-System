#include "DataStorage.h"

DataStorage::DataStorage() 
  : bufferSize(0),
    maxBufferSize(256),
    totalDataPoints(0),
    storedDataPoints(0) {
  
  config.eepromSize = EEPROM.length();
  config.sdCardAvailable = false;
  config.sdCardSize = 0;
  config.maxDataPoints = 10000;
  
  dataBuffer.reserve(maxBufferSize);
}

bool DataStorage::initialize() {
  // 初始化EEPROM
  if (!checkEEPROM()) {
    lastError = "EEPROM初始化失败";
    return false;
  }
  
  return true;
}

template<typename T>
bool DataStorage::saveToEEPROM(uint16_t address, const T& data) {
  if (address + sizeof(T) > config.eepromSize) {
    lastError = "EEPROM地址超出范围";
    return false;
  }
  
  EEPROM.put(address, data);
  return true;
}

template<typename T>
bool DataStorage::loadFromEEPROM(uint16_t address, T& data) {
  if (address + sizeof(T) > config.eepromSize) {
    lastError = "EEPROM地址超出范围";
    return false;
  }
  
  EEPROM.get(address, data);
  return true;
}

bool DataStorage::beginSDCard(uint8_t csPin) {
  // 简化实现
  config.sdCardAvailable = false;
  return false;
}

bool DataStorage::createDataFile(const String& filename) {
  fileName = filename;
  return true;
}

bool DataStorage::appendData(const String& data) {
  if (!addToBuffer(data)) {
    return false;
  }
  
  // 模拟写入数据
  storedDataPoints++;
  
  // 如果缓冲区满了，刷新
  if (bufferSize >= maxBufferSize) {
    return flushBuffer();
  }
  
  return true;
}

bool DataStorage::flushBuffer() {
  // 简化实现：清空缓冲区
  dataBuffer = "";
  bufferSize = 0;
  return true;
}

bool DataStorage::logSensorData(const SensorData& data, uint32_t timestamp) {
  String csvData = formatCSV(data, timestamp);
  return appendData(csvData);
}

bool DataStorage::logControlData(const ControlDecision& decision, uint32_t timestamp) {
  // 简化实现
  String logEntry = String(timestamp) + ",Control," + 
                   String(decision.mode) + "," + 
                   String(decision.controlOutput, 2) + "," +
                   decision.reasoning;
  return appendData(logEntry);
}

bool DataStorage::logSystemStatus(const DigitalTwinData& twin, uint32_t timestamp) {
  // 简化实现
  String logEntry = String(timestamp) + ",System," + 
                   String(twin.systemHealth, 1) + "," +
                   String(twin.remainingLife, 1);
  return appendData(logEntry);
}

bool DataStorage::readHistoricalData(uint32_t startTime, uint32_t endTime, 
                                    void (*callback)(const String& data)) {
  // 简化实现
  return false;
}

bool DataStorage::clearOldData(uint32_t olderThan) {
  // 简化实现
  return true;
}

uint32_t DataStorage::getStoredDataPoints() const {
  return storedDataPoints;
}

uint32_t DataStorage::getTotalStorageSpace() const {
  return config.eepromSize;
}

uint32_t DataStorage::getFreeStorageSpace() const {
  return config.eepromSize - (storedDataPoints * 50); // 简化估算
}

String DataStorage::getLastError() const {
  return lastError;
}

void DataStorage::clearError() {
  lastError = "";
}

bool DataStorage::ensureFileOpen() {
  // 简化实现
  return true;
}

bool DataStorage::checkSDCard() {
  return config.sdCardAvailable;
}

bool DataStorage::checkEEPROM() {
  return config.eepromSize > 0;
}

String DataStorage::formatCSV(const SensorData& data, uint32_t timestamp) {
  String csv = String(timestamp) + "," +
               String(data.flowRate, 2) + "," +
               String(data.pollutionLevel, 2) + "," +
               String(data.lightIntensity, 2) + "," +
               String(data.pH, 2) + "," +
               String(data.temperature, 2) + "," +
               String(data.energyUsage, 2) + "," +
               String(data.systemEfficiency, 2);
  
  return csv;
}

String DataStorage::formatJSON(const SensorData& data, uint32_t timestamp) {
  // 简化实现
  return "{}";
}

String DataStorage::formatBinary(const SensorData& data, uint32_t timestamp) {
  // 简化实现
  return "";
}

bool DataStorage::addToBuffer(const String& data) {
  if (dataBuffer.length() + data.length() + 2 > maxBufferSize) {
    return false;
  }
  
  dataBuffer += data + "\n";
  bufferSize = dataBuffer.length();
  
  return true;
}

void DataStorage::clearBuffer() {
  dataBuffer = "";
  bufferSize = 0;
}

// 显式实例化模板函数
template bool DataStorage::saveToEEPROM<float>(uint16_t, const float&);
template bool DataStorage::saveToEEPROM<int>(uint16_t, const int&);
template bool DataStorage::loadFromEEPROM<float>(uint16_t, float&);
template bool DataStorage::loadFromEEPROM<int>(uint16_t, int&);