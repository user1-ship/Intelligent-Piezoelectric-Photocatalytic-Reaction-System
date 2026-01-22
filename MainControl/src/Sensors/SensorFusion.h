#ifndef SENSOR_FUSION_H
#define SENSOR_FUSION_H

#include <Arduino.h>
#include "../Core/CommonTypes.h"
#include "../Core/SystemConfig.h"
#include "../Utilities/CircularBuffer.h"

class SensorFusion {
private:
  // 卡尔曼滤波器状态
  struct KalmanState {
    float estimate;
    float estimateError;
    float processNoise;
    float measurementNoise;
  };
  
  // 传感器权重
  float sensorWeights[5];
  float fusionConfidence;
  
  // 历史数据
  CircularBuffer<float, 20> fusedHistory;
  
  // 卡尔曼滤波器
  KalmanState kalmanStates[5];
  
  // 回归模型参数
  float regressionWeights[4]; // 多元回归权重
  
public:
  SensorFusion();
  
  // 初始化融合算法
  void initialize();
  
  // 多传感器融合
  float fuseSensorData(const SensorData& sensorData);
  
  // 卡尔曼滤波
  float applyKalmanFilter(uint8_t sensorIndex, float measurement);
  void updateKalmanParameters(uint8_t sensorIndex, float processNoise, float measurementNoise);
  
  // 传感器故障补偿
  float compensateFaultySensor(uint8_t faultySensor, const SensorData& sensorData);
  
  // 回归模型估计
  float estimateByRegression(const SensorData& sensorData, uint8_t targetSensor);
  
  // 物理模型估计
  float estimateByPhysicalModel(const SensorData& sensorData, float reactionRate, float degradation);
  
  // 自适应权重调整
  void adjustWeightsBasedOnQuality(const SensorData& sensorData);
  
  // 获取融合置信度
  float getFusionConfidence() const;
  
  // 重置融合算法
  void reset();
  
private:
  // 计算协方差
  float calculateCovariance(uint8_t sensor1, uint8_t sensor2, const SensorData& sensorData);
  
  // 更新回归模型
  void updateRegressionModel(const SensorData& sensorData, float actualValue);
  
  // 计算残差
  float calculateResidual(float estimated, float actual);
};

#endif // SENSOR_FUSION_H