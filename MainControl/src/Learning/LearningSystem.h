#ifndef LEARNING_SYSTEM_H
#define LEARNING_SYSTEM_H

#include <Arduino.h>
#include "../Core/CommonTypes.h"
#include "../Utilities/CircularBuffer.h"

class LearningSystem {
private:
  // 学习数据
  LearningData currentLearning;
  LearningData bestLearning;
  
  // 经验回放缓冲区
  CircularBuffer<float, 50> performanceBuffer;
  CircularBuffer<float, 50> stateBuffer[5]; // 5个状态变量
  
  // 学习参数
  float learningRate;
  float explorationRate;
  float discountFactor;
  
  // Q-learning参数
  float qTable[5][10]; // 简化的Q表：状态×动作
  
  // 学习状态
  bool learningEnabled;
  unsigned long lastLearningUpdate;
  int learningIterations;
  
public:
  LearningSystem();
  
  // 初始化学习系统
  bool initialize();
  
  // 执行在线学习
  void performOnlineLearning(const SensorData& sensors, const DigitalTwinData& twin);
  
  // 强化学习
  void reinforcementLearning(const SensorData& sensors, float reward);
  
  // 更新PID参数
  void updatePIDParameters(float& Kp, float& Ki, float& Kd, 
                          const SensorData& sensors, float error);
  
  // 更新传感器融合权重
  void updateFusionWeights(float weights[4], const SensorData& sensors, 
                          float predictionError);
  
  // 获取最佳学习数据
  const LearningData& getBestLearningData() const;
  const LearningData& getCurrentLearningData() const;
  
  // 保存学习数据
  bool saveLearningData();
  
  // 加载学习数据
  bool loadLearningData();
  
  // 启用/禁用学习
  void enableLearning(bool enable);
  bool isLearningEnabled() const;
  
  // 重置学习系统
  void reset();
  
private:
  // 计算奖励函数
  float calculateReward(const SensorData& sensors, const DigitalTwinData& twin);
  
  // 更新Q表
  void updateQTable(int state, int action, float reward, int nextState);
  
  // 选择动作（ε-贪婪策略）
  int selectAction(int state);
  
  // 状态离散化
  int discretizeState(const SensorData& sensors) const;
  
  // 动作离散化
  int discretizeAction(float controlOutput) const;
  
  // 计算性能改进
  float calculatePerformanceImprovement() const;
};

#endif // LEARNING_SYSTEM_H