#ifndef DATA_STORAGE_H
#define DATA_STORAGE_H

#include <Arduino.h>
#include <EEPROM.h>
#include <SD.h>
#include "../Core/CommonTypes.h"

class DataStorage {
private:
  // 存储配置
  struct StorageConfig {
    uint32_t eepromSize;
    bool sdCardAvailable;
    uint32_t sdCardSize;
    uint32_t maxDataPoints;
  };
  
  StorageConfig config;
  
  // 文件管理
  File dataFile;
  String fileName;
  
  // 数据缓冲区
  String dataBuffer;
  uint16_t bufferSize;
  uint16_t maxBufferSize;
  
  // 存储统计
  uint32_t totalDataPoints;
  uint32_t storedDataPoints;
  
public:
  DataStorage();
  
  // 初始化存储系统
  bool initialize();
  
  // EEPROM操作
  template<typename T>
  bool saveToEEPROM(uint16_t address, const T& data);
  
  template<typename T>
  bool loadFromEEPROM(uint16_t address, T& data);
  
  // SD卡操作
  bool beginSDCard(uint8_t csPin = 4);
  bool createDataFile(const String& filename);
  bool appendData(const String& data);
  bool flushBuffer();
  
  // 数据记录
  bool logSensorData(const SensorData& data, uint32_t timestamp);
  bool logControlData(const ControlDecision& decision, uint32_t timestamp);
  bool logSystemStatus(const DigitalTwinData& twin, uint32_t timestamp);
  
  // 数据检索
  bool readHistoricalData(uint32_t startTime, uint32_t endTime, 
                         void (*callback)(const String& data));
  
  // 存储管理
  bool clearOldData(uint32_t olderThan);
  uint32_t getStoredDataPoints() const;
  uint32_t getTotalStorageSpace() const;
  uint32_t getFreeStorageSpace() const;
  
  // 错误处理
  String getLastError() const;
  void clearError();
  
private:
  // 错误状态
  String lastError;
  
  // 内部方法
  bool ensureFileOpen();
  bool checkSDCard();
  bool checkEEPROM();
  
  // 数据格式转换
  String formatCSV(const SensorData& data, uint32_t timestamp);
  String formatJSON(const SensorData& data, uint32_t timestamp);
  String formatBinary(const SensorData& data, uint32_t timestamp);
  
  // 缓冲区管理
  bool addToBuffer(const String& data);
  void clearBuffer();
};

#endif // DATA_STORAGE_H