#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include <Arduino.h>

// ========== 基本数据类型定义 ==========
struct SensorData {
  float flowRate;          // 流量 (cm/s)
  float pollutionLevel;    // 污染物浓度 (ppm)
  float lightIntensity;    // 光照强度 (lux)
  float pH;                // pH值
  float temperature;       // 温度 (°C)
  float energyUsage;       // 能耗 (%)
  float systemEfficiency;  // 系统效率 (%)
  bool sensorFaults[5];    // 传感器故障标志
  float dataQuality[5];    // 数据质量指标 [0-1]
};

struct ControlDecision {
  float controlOutput;     // 控制输出 (0-100%)
  uint8_t mode;           // 控制模式
  String reasoning;       // 决策理由
};

struct DigitalTwinData {
  float predictedPollution;    // 预测污染物浓度
  float predictedEfficiency;   // 预测系统效率
  float remainingLife;         // 剩余寿命 (%)
  float optimalSetpoint;       // 最优设定点
  float systemHealth;          // 系统健康度 (%)
  float performanceTrend;      // 性能趋势
};

// 系统模型参数
struct SystemModel {
  float efficiency;         // 系统效率
  float energyFactor;       // 能量因子
  float degradation;        // 催化剂降解程度
  float reactionRate;       // 化学反应速率参数
  float massTransfer;       // 质量传递系数
  unsigned long lastUpdate; // 最后更新时间
};

// MPC参数
struct MPCParameters {
  float predictionHorizon;  // 预测时域
  float controlHorizon;     // 控制时域
  float weights[3];         // 权重系数
};

// 学习数据
struct LearningData {
  float bestKp, bestKi, bestKd; // 最优PID参数
  float bestConditions[5];       // 最优工况参数
  float fusionWeights[4];        // 传感器融合权重
  int learningSamples;           // 学习样本数
};

// 控制模式枚举
enum ControlMode : uint8_t {
  ENERGY_SAVING = 0,    // 节能模式
  STANDARD,             // 标准模式  
  HIGH_EFFICIENCY,      // 高效模式
  SHOCK_LOAD,           // 冲击负荷模式
  MAINTENANCE           // 维护模式
};

// 模糊等级枚举
enum FuzzyLevel : uint8_t {
  FUZZY_VERY_LOW = 0,
  FUZZY_LOW,
  FUZZY_MEDIUM,
  FUZZY_HIGH,
  FUZZY_VERY_HIGH
};

#endif // COMMON_TYPES_H