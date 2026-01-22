#ifndef DIGITAL_TWIN_H
#define DIGITAL_TWIN_H

#include <Arduino.h>
#include "../Core/CommonTypes.h"
#include "../Utilities/CircularBuffer.h"

class DigitalTwin {
private:
  // 系统模型
  SystemModel systemModel;
  
  // 历史数据
  CircularBuffer<float, 20> pollutionHistory;
  CircularBuffer<float, 20> efficiencyHistory;
  CircularBuffer<float, 20> energyHistory;
  
  // 预测模型参数
  float predictionWeights[3]; // ARIMA, 物理模型, 机器学习权重
  float modelAccuracy[3];
  
  // 性能指标
  DigitalTwinData currentState;
  
  // 时间序列分析
  float trendCoefficient;
  float seasonalComponent;
  
  // 模型更新标志
  bool modelUpdated;
  unsigned long lastModelUpdate;
  
public:
  DigitalTwin();
  
  // 初始化数字孪生
  bool initialize();
  
  // 运行仿真
  DigitalTwinData simulate(const SensorData& sensors);
  
  // 更新系统模型
  void updateSystemModel(const SensorData& sensors, const DigitalTwinData& twin);
  
  // 预测污染物浓度
  float predictPollution(const SensorData& sensors);
  
  // 预测系统效率
  float predictEfficiency(const SensorData& sensors);
  
  // 预测剩余寿命
  float predictRemainingLife(const SensorData& sensors);
  
  // 计算最优设定点
  float calculateOptimalSetpoint(const SensorData& sensors, const DigitalTwinData& twin);
  
  // 计算系统健康度
  float calculateSystemHealth(const SensorData& sensors);
  
  // 计算性能趋势
  float calculatePerformanceTrend();
  
  // 获取当前状态
  const DigitalTwinData& getCurrentState() const;
  
  // 获取系统模型
  const SystemModel& getSystemModel() const;
  
  // 重置数字孪生
  void reset();
  
  // 保存/加载模型参数
  bool saveModelParameters();
  bool loadModelParameters();
  
private:
  // 时间序列预测方法
  float arimaPrediction();
  float physicalModelPrediction(const SensorData& sensors);
  float machineLearningPrediction(const SensorData& sensors);
  
  // 模型融合
  float fusePredictions(const float predictions[3]);
  
  // 更新预测权重
  void updatePredictionWeights(float actualValue, const float predictions[3]);
  
  // 计算模型误差
  float calculatePredictionError(float predicted, float actual) const;
  
  // 更新趋势分析
  void updateTrendAnalysis(float currentValue);
};

#endif // DIGITAL_TWIN_H