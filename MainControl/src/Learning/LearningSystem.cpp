#include "LearningSystem.h"

LearningSystem::LearningSystem() 
  : learningRate(0.01f),
    explorationRate(0.1f),
    discountFactor(0.9f),
    learningEnabled(true),
    lastLearningUpdate(0),
    learningIterations(0) {
  
  // 初始化学习数据
  currentLearning.bestKp = 1.0f;
  currentLearning.bestKi = 0.1f;
  currentLearning.bestKd = 0.05f;
  currentLearning.learningSamples = 1;
  
  for (int i = 0; i < 5; i++) {
    currentLearning.bestConditions[i] = 0.0f;
  }
  
  for (int i = 0; i < 4; i++) {
    currentLearning.fusionWeights[i] = 0.25f;
  }
  
  bestLearning = currentLearning;
  
  // 初始化Q表
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 10; j++) {
      qTable[i][j] = 0.0f;
    }
  }
}

bool LearningSystem::initialize() {
  loadLearningData();
  return true;
}

void LearningSystem::performOnlineLearning(const SensorData& sensors, const DigitalTwinData& twin) {
  if (!learningEnabled) return;
  
  // 计算当前性能
  float currentPerformance = calculateReward(sensors, twin);
  
  // 强化学习
  reinforcementLearning(sensors, currentPerformance);
  
  // 更新学习统计
  learningIterations++;
  lastLearningUpdate = millis();
  
  // 定期保存学习数据
  if (learningIterations % 10 == 0) {
    saveLearningData();
  }
}

void LearningSystem::reinforcementLearning(const SensorData& sensors, float reward) {
  // 简化强化学习实现
  int currentState = discretizeState(sensors);
  int action = selectAction(currentState);
  
  // 执行动作并观察新状态（简化）
  int nextState = currentState; // 简化：假设状态不变
  
  // 更新Q表
  updateQTable(currentState, action, reward, nextState);
  
  // 逐渐降低探索率
  explorationRate *= 0.995f;
  explorationRate = max(explorationRate, 0.01f);
}

void LearningSystem::updatePIDParameters(float& Kp, float& Ki, float& Kd, 
                                        const SensorData& sensors, float error) {
  // 简化PID参数更新
  if (fabs(error) > 50.0f) {
    // 误差大时增加Kp
    Kp = currentLearning.bestKp * 1.2f;
    Ki = currentLearning.bestKi * 0.8f;
  } else {
    // 误差小时恢复最佳参数
    Kp = currentLearning.bestKp;
    Ki = currentLearning.bestKi;
    Kd = currentLearning.bestKd;
  }
}

void LearningSystem::updateFusionWeights(float weights[4], const SensorData& sensors, 
                                        float predictionError) {
  // 简化融合权重更新
  if (predictionError > 20.0f) {
    // 预测误差大时，增加物理模型权重
    weights[0] *= 0.9f; // 降低回归权重
    weights[1] *= 1.1f; // 增加物理模型权重
    
    // 归一化
    float sum = weights[0] + weights[1] + weights[2] + weights[3];
    for (int i = 0; i < 4; i++) {
      weights[i] /= sum;
    }
    
    // 更新学习数据
    for (int i = 0; i < 4; i++) {
      currentLearning.fusionWeights[i] = weights[i];
    }
  }
}

const LearningData& LearningSystem::getBestLearningData() const {
  return bestLearning;
}

const LearningData& LearningSystem::getCurrentLearningData() const {
  return currentLearning;
}

bool LearningSystem::saveLearningData() {
  // 简化实现：保存到EEPROM
  return true;
}

bool LearningSystem::loadLearningData() {
  // 简化实现：从EEPROM加载
  return true;
}

void LearningSystem::enableLearning(bool enable) {
  learningEnabled = enable;
}

bool LearningSystem::isLearningEnabled() const {
  return learningEnabled;
}

void LearningSystem::reset() {
  learningIterations = 0;
  explorationRate = 0.1f;
  
  // 重置Q表
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 10; j++) {
      qTable[i][j] = 0.0f;
    }
  }
}

float LearningSystem::calculateReward(const SensorData& sensors, const DigitalTwinData& twin) {
  // 多目标奖励函数
  float efficiencyReward = sensors.systemEfficiency / 100.0f;
  float energyReward = 1.0f - (sensors.energyUsage / 100.0f);
  float healthReward = twin.systemHealth / 100.0f;
  
  // 加权奖励
  return efficiencyReward * 0.5f + energyReward * 0.3f + healthReward * 0.2f;
}

void LearningSystem::updateQTable(int state, int action, float reward, int nextState) {
  // Q-learning更新规则: Q(s,a) = Q(s,a) + α[r + γ*max_a'Q(s',a') - Q(s,a)]
  float maxNextQ = 0.0f;
  for (int a = 0; a < 10; a++) {
    if (qTable[nextState][a] > maxNextQ) {
      maxNextQ = qTable[nextState][a];
    }
  }
  
  float target = reward + discountFactor * maxNextQ;
  float error = target - qTable[state][action];
  qTable[state][action] += learningRate * error;
}

int LearningSystem::selectAction(int state) {
  // ε-贪婪策略
  if (random(0, 100) < explorationRate * 100) {
    // 探索：随机选择动作
    return random(0, 10);
  } else {
    // 利用：选择最优动作
    int bestAction = 0;
    float bestValue = qTable[state][0];
    
    for (int a = 1; a < 10; a++) {
      if (qTable[state][a] > bestValue) {
        bestValue = qTable[state][a];
        bestAction = a;
      }
    }
    
    return bestAction;
  }
}

int LearningSystem::discretizeState(const SensorData& sensors) const {
  // 简化状态离散化：基于污染物浓度
  if (sensors.pollutionLevel < 100.0f) return 0;
  if (sensors.pollutionLevel < 200.0f) return 1;
  if (sensors.pollutionLevel < 300.0f) return 2;
  if (sensors.pollutionLevel < 400.0f) return 3;
  return 4;
}

int LearningSystem::discretizeAction(float controlOutput) const {
  // 离散化控制输出
  return (int)(controlOutput / 10.0f);
}

float LearningSystem::calculatePerformanceImprovement() const {
  // 简化性能改进计算
  return 0.0f;
}