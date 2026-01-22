#include "DigitalTwin.h"
#include "../Core/SystemConfig.h"

DigitalTwin::DigitalTwin() {
  initialize();
}

bool DigitalTwin::initialize() {
  // 初始化系统模型
  systemModel.efficiency = 1.0f;
  systemModel.energyFactor = 1.0f;
  systemModel.degradation = 1.0f;
  systemModel.reactionRate = 0.1f;
  systemModel.massTransfer = 0.05f;
  systemModel.lastUpdate = millis();
  
  // 初始化预测权重
  for (int i = 0; i < 3; i++) {
    predictionWeights[i] = 0.333f; // 平均权重
    modelAccuracy[i] = 1.0f;
  }
  
  trendCoefficient = 0.0f;
  seasonalComponent = 0.0f;
  modelUpdated = false;
  lastModelUpdate = millis();
  
  // 清空历史数据
  pollutionHistory.clear();
  efficiencyHistory.clear();
  energyHistory.clear();
  
  return true;
}

DigitalTwinData DigitalTwin::simulate(const SensorData& sensors) {
  DigitalTwinData result;
  
  // 预测污染物浓度
  result.predictedPollution = predictPollution(sensors);
  
  // 预测系统效率
  result.predictedEfficiency = predictEfficiency(sensors);
  
  // 预测剩余寿命
  result.remainingLife = predictRemainingLife(sensors);
  
  // 计算最优设定点
  result.optimalSetpoint = calculateOptimalSetpoint(sensors, result);
  
  // 计算系统健康度
  result.systemHealth = calculateSystemHealth(sensors);
  
  // 计算性能趋势
  result.performanceTrend = calculatePerformanceTrend();
  
  // 更新当前状态
  currentState = result;
  
  // 更新历史数据
  pollutionHistory.push(sensors.pollutionLevel);
  efficiencyHistory.push(sensors.systemEfficiency);
  energyHistory.push(sensors.energyUsage);
  
  // 更新系统模型
  updateSystemModel(sensors, result);
  
  return result;
}

void DigitalTwin::updateSystemModel(const SensorData& sensors, const DigitalTwinData& twin) {
  // 简化实现：基于预测误差更新模型
  float predictionError = fabs(sensors.pollutionLevel - twin.predictedPollution);
  
  if (predictionError > 10.0f) {
    // 调整反应速率
    systemModel.reactionRate *= 0.95f;
    systemModel.reactionRate = max(systemModel.reactionRate, 0.01f);
  }
  
  // 更新催化剂衰减
  systemModel.degradation *= 0.999f;
  systemModel.degradation = max(systemModel.degradation, 0.5f);
  
  systemModel.lastUpdate = millis();
  modelUpdated = true;
}

float DigitalTwin::predictPollution(const SensorData& sensors) {
  // 使用三种方法预测
  float predictions[3];
  predictions[0] = arimaPrediction();
  predictions[1] = physicalModelPrediction(sensors);
  predictions[2] = machineLearningPrediction(sensors);
  
  // 融合预测结果
  return fusePredictions(predictions);
}

float DigitalTwin::predictEfficiency(const SensorData& sensors) {
  // 简化预测：基于当前效率和历史趋势
  float baseEfficiency = sensors.systemEfficiency;
  float trend = trendCoefficient * 0.1f; // 考虑趋势
  
  return constrain(baseEfficiency * (1.0f + trend), 0.0f, 100.0f);
}

float DigitalTwin::predictRemainingLife(const SensorData& sensors) {
  // 基于催化剂衰减和操作条件预测寿命
  float baseLife = 100.0f; // 初始寿命百分比
  
  // 考虑温度影响
  float temperatureFactor = 1.0f - 0.01f * max(0.0f, sensors.temperature - 25.0f);
  
  // 考虑污染物浓度影响
  float pollutionFactor = 1.0f - 0.0005f * sensors.pollutionLevel;
  
  float remainingLife = baseLife * systemModel.degradation * temperatureFactor * pollutionFactor;
  
  return constrain(remainingLife, 0.0f, 100.0f);
}

float DigitalTwin::calculateOptimalSetpoint(const SensorData& sensors, const DigitalTwinData& twin) {
  // 基于系统状态动态调整设定点
  float baseSetpoint = TARGET_POLLUTION;
  
  // 根据系统健康度调整
  if (twin.systemHealth < 70.0f) {
    baseSetpoint *= 1.2f; // 健康度低时放宽要求
  }
  
  // 根据性能趋势调整
  if (twin.performanceTrend > 0.1f) {
    baseSetpoint *= 0.9f; // 性能改善时提高要求
  }
  
  return baseSetpoint;
}

float DigitalTwin::calculateSystemHealth(const SensorData& sensors) {
  float health = 100.0f;
  
  // 传感器健康度
  int workingSensors = 0;
  for (int i = 0; i < 5; i++) {
    if (!sensors.sensorFaults[i]) {
      workingSensors++;
    }
  }
  health *= (workingSensors / 5.0f);
  
  // 性能健康度
  if (sensors.systemEfficiency < 60.0f) health *= 0.8f;
  if (sensors.energyUsage > MAX_ENERGY_USAGE * 0.8f) health *= 0.9f;
  
  return health;
}

float DigitalTwin::calculatePerformanceTrend() {
  if (pollutionHistory.size() < 3) return 0.0f;
  
  // 简化趋势计算：最近3个点的平均变化率
  float recentValues[3];
  for (int i = 0; i < 3 && i < pollutionHistory.size(); i++) {
    pollutionHistory.get(i, recentValues[i]);
  }
  
  float trend = 0.0f;
  for (int i = 1; i < 3; i++) {
    if (recentValues[i-1] > 0.0f) {
      trend += (recentValues[i] - recentValues[i-1]) / recentValues[i-1];
    }
  }
  
  return trend / 2.0f; // 平均趋势
}

const DigitalTwinData& DigitalTwin::getCurrentState() const {
  return currentState;
}

const SystemModel& DigitalTwin::getSystemModel() const {
  return systemModel;
}

void DigitalTwin::reset() {
  initialize();
}

bool DigitalTwin::saveModelParameters() {
  // 简化实现：保存到EEPROM
  return true;
}

bool DigitalTwin::loadModelParameters() {
  // 简化实现：从EEPROM加载
  return true;
}

float DigitalTwin::arimaPrediction() {
  if (pollutionHistory.size() < 2) return 250.0f; // 默认值
  
  float lastValue, secondLastValue;
  pollutionHistory.get(0, lastValue);
  pollutionHistory.get(1, secondLastValue);
  
  // 简化的AR(1)模型：y(t) = 0.8*y(t-1) + 0.2*y(t-2)
  return 0.8f * lastValue + 0.2f * secondLastValue;
}

float DigitalTwin::physicalModelPrediction(const SensorData& sensors) {
  // 基于反应动力学的物理模型
  float reactionRate = systemModel.reactionRate *
                     (1.0f + 0.1f * sensors.flowRate / 50.0f) *
                     (1.0f + 0.05f * sensors.lightIntensity / 500.0f);
  
  return sensors.pollutionLevel * exp(-reactionRate * 1.0f); // 预测1个时间单位后
}

float DigitalTwin::machineLearningPrediction(const SensorData& sensors) {
  // 简化机器学习预测：基于历史趋势
  if (pollutionHistory.size() < 3) return sensors.pollutionLevel;
  
  float trend = calculatePerformanceTrend();
  return sensors.pollutionLevel * (1.0f + trend);
}

float DigitalTwin::fusePredictions(const float predictions[3]) {
  // 加权融合
  float fused = 0.0f;
  for (int i = 0; i < 3; i++) {
    fused += predictions[i] * predictionWeights[i];
  }
  return fused;
}

void DigitalTwin::updatePredictionWeights(float actualValue, const float predictions[3]) {
  // 基于预测精度更新权重
  float totalError = 0.0f;
  float errors[3];
  
  for (int i = 0; i < 3; i++) {
    errors[i] = calculatePredictionError(predictions[i], actualValue);
    totalError += errors[i];
  }
  
  if (totalError > 0.0f) {
    for (int i = 0; i < 3; i++) {
      predictionWeights[i] = (totalError - errors[i]) / (2.0f * totalError);
    }
  }
}

float DigitalTwin::calculatePredictionError(float predicted, float actual) const {
  if (actual > 0.0f) {
    return fabs(predicted - actual) / actual;
  }
  return fabs(predicted - actual);
}

void DigitalTwin::updateTrendAnalysis(float currentValue) {
  // 简化趋势分析
  static float previousValue = 250.0f;
  
  if (previousValue > 0.0f) {
    trendCoefficient = (currentValue - previousValue) / previousValue;
  }
  
  previousValue = currentValue;
}