#include "SensorFusion.h"

SensorFusion::SensorFusion() {
  initialize();
}

void SensorFusion::initialize() {
  // 初始化传感器权重
  for (int i = 0; i < 5; i++) {
    sensorWeights[i] = 0.2f; // 平均权重
  }
  
  // 初始化卡尔曼滤波器
  for (int i = 0; i < 5; i++) {
    kalmanStates[i].estimate = 250.0f; // 初始估计值
    kalmanStates[i].estimateError = 1.0f;
    kalmanStates[i].processNoise = 0.1f;
    kalmanStates[i].measurementNoise = 0.5f;
  }
  
  // 初始化回归权重
  for (int i = 0; i < 4; i++) {
    regressionWeights[i] = 0.25f;
  }
  
  fusionConfidence = 1.0f;
  fusedHistory.clear();
}

float SensorFusion::fuseSensorData(const SensorData& sensorData) {
  // 简化实现：返回污染物浓度
  return sensorData.pollutionLevel;
}

float SensorFusion::applyKalmanFilter(uint8_t sensorIndex, float measurement) {
  if (sensorIndex >= 5) return measurement;
  
  KalmanState& state = kalmanStates[sensorIndex];
  
  // 预测步骤
  float predictedEstimate = state.estimate;
  float predictedError = state.estimateError + state.processNoise;
  
  // 更新步骤
  float kalmanGain = predictedError / (predictedError + state.measurementNoise);
  state.estimate = predictedEstimate + kalmanGain * (measurement - predictedEstimate);
  state.estimateError = (1 - kalmanGain) * predictedError;
  
  return state.estimate;
}

void SensorFusion::updateKalmanParameters(uint8_t sensorIndex, float processNoise, float measurementNoise) {
  if (sensorIndex < 5) {
    kalmanStates[sensorIndex].processNoise = processNoise;
    kalmanStates[sensorIndex].measurementNoise = measurementNoise;
  }
}

float SensorFusion::compensateFaultySensor(uint8_t faultySensor, const SensorData& sensorData) {
  // 简化实现：使用其他传感器的平均值
  float sum = 0.0f;
  int count = 0;
  
  for (int i = 0; i < 5; i++) {
    if (i != faultySensor) {
      switch (i) {
        case 0: sum += sensorData.flowRate; break;
        case 1: sum += sensorData.pollutionLevel; break;
        case 2: sum += sensorData.lightIntensity; break;
        case 3: sum += sensorData.pH; break;
        case 4: sum += sensorData.temperature; break;
      }
      count++;
    }
  }
  
  return count > 0 ? sum / count : 0.0f;
}

float SensorFusion::estimateByRegression(const SensorData& sensorData, uint8_t targetSensor) {
  // 简化实现：返回平均值
  return (sensorData.flowRate + sensorData.lightIntensity + sensorData.pH + sensorData.temperature) / 4.0f;
}

float SensorFusion::estimateByPhysicalModel(const SensorData& sensorData, float reactionRate, float degradation) {
  // 简化实现：基于物理模型估计
  float estimate = sensorData.pollutionLevel * (1.0f - reactionRate) * degradation;
  return estimate;
}

void SensorFusion::adjustWeightsBasedOnQuality(const SensorData& sensorData) {
  // 简化实现：根据数据质量调整权重
  float totalQuality = 0.0f;
  for (int i = 0; i < 5; i++) {
    totalQuality += sensorData.dataQuality[i];
  }
  
  if (totalQuality > 0.0f) {
    for (int i = 0; i < 5; i++) {
      sensorWeights[i] = sensorData.dataQuality[i] / totalQuality;
    }
  }
}

float SensorFusion::getFusionConfidence() const {
  return fusionConfidence;
}

void SensorFusion::reset() {
  initialize();
}

float SensorFusion::calculateCovariance(uint8_t sensor1, uint8_t sensor2, const SensorData& sensorData) {
  // 简化实现
  return 0.0f;
}

void SensorFusion::updateRegressionModel(const SensorData& sensorData, float actualValue) {
  // 简化实现
}

float SensorFusion::calculateResidual(float estimated, float actual) {
  return fabs(estimated - actual);
}