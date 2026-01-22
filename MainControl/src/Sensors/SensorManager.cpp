#include "SensorManager.h"
#include <EEPROM.h>

// 简化数学函数，避免依赖 MathUtils.h
namespace LocalMath {
  float constrainFloat(float value, float minVal, float maxVal) {
    if (value < minVal) return minVal;
    if (value > maxVal) return maxVal;
    return value;
  }
  
  float mapFloat(float value, float inMin, float inMax, float outMin, float outMax) {
    return (value - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
  }
}

SensorManager::SensorManager() {
  // 初始化校准参数（默认值）
  for (int i = 0; i < 5; i++) {
    calibrationOffsets[i] = 0.0f;
    calibrationGains[i] = 1.0f;
    previousReadings[i] = 0.0f;
    faultStartTime[i] = 0;
    persistentFaults[i] = false;
    dataStability[i] = 1.0f;
    dataVariance[i] = 0.0f;
  }
}

bool SensorManager::initialize() {
  // 从EEPROM加载校准数据
  for (int i = 0; i < 5; i++) {
    int addr = i * sizeof(float) * 2;
    EEPROM.get(addr, calibrationOffsets[i]);
    EEPROM.get(addr + sizeof(float), calibrationGains[i]);
  }
  
  // 清除缓冲区
  flowBuffer.clear();
  pollutionBuffer.clear();
  lightBuffer.clear();
  pHBuffer.clear();
  
  // 初始读数，建立基准
  for (int i = 0; i < 10; i++) {
    readAllSensors();
    delay(100);
  }
  
  return true;
}

SensorData SensorManager::readAllSensors() {
  SensorData data;
  
  // 读取所有传感器原始值
  float rawReadings[5];
  rawReadings[0] = readSensorRaw(FLOW_SENSOR_PIN);
  rawReadings[1] = readSensorRaw(POLLUTION_SENSOR_PIN);
  rawReadings[2] = readSensorRaw(LIGHT_SENSOR_PIN);
  rawReadings[3] = readSensorRaw(PH_SENSOR_PIN);
  rawReadings[4] = readSensorRaw(TEMPERATURE_SENSOR_PIN);
  
  // 应用滤波
  for (int i = 0; i < 5; i++) {
    rawReadings[i] = applyFilter(i, rawReadings[i]);
  }
  
  // 转换到物理量
  data.flowRate = convertToPhysical(0, rawReadings[0]);
  data.pollutionLevel = convertToPhysical(1, rawReadings[1]);
  data.lightIntensity = convertToPhysical(2, rawReadings[2]);
  data.pH = convertToPhysical(3, rawReadings[3]);
  data.temperature = convertToPhysical(4, rawReadings[4]);
  
  // 检测故障
  for (int i = 0; i < 5; i++) {
    data.sensorFaults[i] = detectFault(i, rawReadings[i]);
  }
  
  // 计算数据质量
  for (int i = 0; i < 5; i++) {
    data.dataQuality[i] = calculateDataQuality(i, rawReadings[i], data.sensorFaults[i]);
  }
  
  // 更新历史缓冲区
  flowBuffer.push(data.flowRate);
  pollutionBuffer.push(data.pollutionLevel);
  lightBuffer.push(data.lightIntensity);
  pHBuffer.push(data.pH);
  
  // 更新稳定性数据
  for (int i = 0; i < 5; i++) {
    updateStability(i, rawReadings[i]);
  }
  
  // 这些值需要外部计算
  data.energyUsage = 0.0f;
  data.systemEfficiency = 0.0f;
  
  return data;
}

float SensorManager::readSensorRaw(uint8_t sensorPin) {
  // 多次采样取平均，减少噪声
  const int samples = 10;
  long sum = 0;
  
  for (int i = 0; i < samples; i++) {
    sum += analogRead(sensorPin);
    delayMicroseconds(100);
  }
  
  return static_cast<float>(sum) / samples;
}

float SensorManager::convertToPhysical(uint8_t sensorIndex, float rawValue) {
  // 应用校准参数
  float calibrated = (rawValue * calibrationGains[sensorIndex]) + calibrationOffsets[sensorIndex];
  
  // 映射到物理范围
  switch (sensorIndex) {
    case 0: // 流量
      return LocalMath::mapFloat(calibrated, 0.0f, 1023.0f, FLOW_MIN, FLOW_MAX);
    case 1: // 污染物
      return LocalMath::mapFloat(calibrated, 0.0f, 1023.0f, POLLUTION_MIN, POLLUTION_MAX);
    case 2: // 光照
      return LocalMath::mapFloat(calibrated, 0.0f, 1023.0f, LIGHT_MIN, LIGHT_MAX);
    case 3: // pH
      return LocalMath::mapFloat(calibrated, 0.0f, 1023.0f, PH_MIN, PH_MAX);
    case 4: // 温度
      return LocalMath::mapFloat(calibrated, 0.0f, 1023.0f, TEMP_MIN, TEMP_MAX);
    default:
      return calibrated;
  }
}

bool SensorManager::detectFault(uint8_t sensorIndex, float rawValue) {
  // 范围检查
  if (rawValue < 50.0f || rawValue > 1000.0f) {
    if (faultStartTime[sensorIndex] == 0) {
      faultStartTime[sensorIndex] = millis();
    } else if (millis() - faultStartTime[sensorIndex] > 5000) {
      persistentFaults[sensorIndex] = true;
    }
    return true;
  }
  
  // 突变检测（超过30%变化）
  if (previousReadings[sensorIndex] > 0.0f) {
    float change = fabs(rawValue - previousReadings[sensorIndex]) / previousReadings[sensorIndex];
    if (change > 0.3f) {
      if (faultStartTime[sensorIndex] == 0) {
        faultStartTime[sensorIndex] = millis();
      } else if (millis() - faultStartTime[sensorIndex] > 1000) {
        return true;
      }
    } else {
      faultStartTime[sensorIndex] = 0;
    }
  }
  
  previousReadings[sensorIndex] = rawValue;
  persistentFaults[sensorIndex] = false;
  return false;
}

float SensorManager::calculateDataQuality(uint8_t sensorIndex, float rawValue, bool isFaulty) {
  if (isFaulty) return 0.0f;
  
  float quality = dataStability[sensorIndex] * 0.7f + (1.0f - dataVariance[sensorIndex]) * 0.3f;
  return LocalMath::constrainFloat(quality, 0.0f, 1.0f);
}

void SensorManager::updateStability(uint8_t sensorIndex, float currentValue) {
  // 简化稳定性计算
  static float history[5][5] = {{0}};
  static uint8_t historyIndex[5] = {0};
  
  history[sensorIndex][historyIndex[sensorIndex]] = currentValue;
  historyIndex[sensorIndex] = (historyIndex[sensorIndex] + 1) % 5;
  
  // 计算平均值和方差
  float sum = 0.0f;
  for (int i = 0; i < 5; i++) {
    sum += history[sensorIndex][i];
  }
  float mean = sum / 5.0f;
  
  float variance = 0.0f;
  for (int i = 0; i < 5; i++) {
    float diff = history[sensorIndex][i] - mean;
    variance += diff * diff;
  }
  variance /= 5.0f;
  
  dataVariance[sensorIndex] = variance / (mean + 0.001f);
  dataStability[sensorIndex] = 1.0f - LocalMath::constrainFloat(dataVariance[sensorIndex], 0.0f, 0.5f);
}

float SensorManager::applyFilter(uint8_t sensorIndex, float rawValue) {
  // 一阶低通滤波器
  static float filtered[5] = {0};
  
  if (filtered[sensorIndex] == 0.0f) {
    filtered[sensorIndex] = rawValue;
  } else {
    float alpha = 0.3f; // 滤波系数
    filtered[sensorIndex] = alpha * rawValue + (1.0f - alpha) * filtered[sensorIndex];
  }
  
  return filtered[sensorIndex];
}

bool SensorManager::calibrateSensor(uint8_t sensorIndex, float knownValue) {
  if (sensorIndex >= 5) return false;
  
  // 获取传感器引脚
  uint8_t sensorPin;
  switch (sensorIndex) {
    case 0: sensorPin = FLOW_SENSOR_PIN; break;
    case 1: sensorPin = POLLUTION_SENSOR_PIN; break;
    case 2: sensorPin = LIGHT_SENSOR_PIN; break;
    case 3: sensorPin = PH_SENSOR_PIN; break;
    case 4: sensorPin = TEMPERATURE_SENSOR_PIN; break;
    default: sensorPin = A0; break;
  }
  
  // 读取当前原始值
  float rawValue = readSensorRaw(sensorPin);
  
  // 计算新的校准参数
  calibrationGains[sensorIndex] = knownValue / (rawValue + 0.001f);
  calibrationOffsets[sensorIndex] = 0.0f;
  
  // 保存到EEPROM
  int addr = sensorIndex * sizeof(float) * 2;
  EEPROM.put(addr, calibrationOffsets[sensorIndex]);
  EEPROM.put(addr + sizeof(float), calibrationGains[sensorIndex]);
  
  return true;
}

bool SensorManager::isSensorFaulty(uint8_t sensorIndex) const {
  if (sensorIndex >= 5) return false;
  return persistentFaults[sensorIndex];
}

float SensorManager::getSensorHealth(uint8_t sensorIndex) const {
  if (sensorIndex >= 5) return 0.0f;
  return dataStability[sensorIndex];
}

float SensorManager::getHistoricalAverage(uint8_t sensorType, size_t samples) const {
  // 简化实现
  return 0.0f;
}

float SensorManager::getHistoricalTrend(uint8_t sensorType, size_t samples) const {
  // 简化实现
  return 0.0f;
}

void SensorManager::resetSensor(uint8_t sensorIndex) {
  if (sensorIndex >= 5) return;
  
  calibrationOffsets[sensorIndex] = 0.0f;
  calibrationGains[sensorIndex] = 1.0f;
  previousReadings[sensorIndex] = 0.0f;
  faultStartTime[sensorIndex] = 0;
  persistentFaults[sensorIndex] = false;
  dataStability[sensorIndex] = 1.0f;
  dataVariance[sensorIndex] = 0.0f;
}

void SensorManager::setCalibration(uint8_t sensorIndex, float offset, float gain) {
  if (sensorIndex >= 5) return;
  
  calibrationOffsets[sensorIndex] = offset;
  calibrationGains[sensorIndex] = gain;
}