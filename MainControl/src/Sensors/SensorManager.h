#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include "../Core/CommonTypes.h"
#include "../Core/SystemConfig.h"
#include "../Utilities/CircularBuffer.h"

class SensorManager {
private:
  // 传感器数据缓冲区
  CircularBuffer<float, FLOW_BUFFER_SIZE> flowBuffer;
  CircularBuffer<float, POLLUTION_BUFFER_SIZE> pollutionBuffer;
  CircularBuffer<float, LIGHT_BUFFER_SIZE> lightBuffer;
  CircularBuffer<float, PH_BUFFER_SIZE> pHBuffer;
  
  // 传感器校准参数
  float calibrationOffsets[5];
  float calibrationGains[5];
  
  // 传感器故障检测
  float previousReadings[5];
  unsigned long faultStartTime[5];
  bool persistentFaults[5];
  
  // 数据质量
  float dataStability[5];
  float dataVariance[5];
  
public:
  SensorManager();
  
  // 初始化传感器
  bool initialize();
  
  // 读取传感器数据
  SensorData readAllSensors();
  
  // 校准传感器
  bool calibrateSensor(uint8_t sensorIndex, float knownValue);
  void setCalibration(uint8_t sensorIndex, float offset, float gain);
  
  // 传感器故障检测
  bool detectFault(uint8_t sensorIndex, float rawValue);
  bool isSensorFaulty(uint8_t sensorIndex) const;
  float getSensorHealth(uint8_t sensorIndex) const;
  
  // 数据质量评估
  float calculateDataQuality(uint8_t sensorIndex, float rawValue, bool isFaulty);
  
  // 获取历史数据
  float getHistoricalAverage(uint8_t sensorType, size_t samples) const;
  float getHistoricalTrend(uint8_t sensorType, size_t samples) const;
  
  // 重置传感器
  void resetSensor(uint8_t sensorIndex);
  
private:
  // 读取单个传感器
  float readSensorRaw(uint8_t sensorPin);
  
  // 转换原始数据到物理量
  float convertToPhysical(uint8_t sensorIndex, float rawValue);
  
  // 更新数据稳定性
  void updateStability(uint8_t sensorIndex, float currentValue);
  
  // 应用数字滤波
  float applyFilter(uint8_t sensorIndex, float rawValue);
};

#endif // SENSOR_MANAGER_H