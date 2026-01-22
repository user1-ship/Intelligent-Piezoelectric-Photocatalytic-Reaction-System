#ifndef CONTROL_SYSTEM_H
#define CONTROL_SYSTEM_H

#include <Arduino.h>
#include <Servo.h>
#include "../Core/CommonTypes.h"
#include "../Core/SystemConfig.h"
#include "PIDController.h"
#include "FuzzyLogic.h"

class ControlSystem {
private:
  // 执行器
  Servo stressServo;
  
  // 控制器
  PIDController pidController;
  FuzzyLogicSystem fuzzySystem;
  
  // 控制模式
  ControlMode currentMode;
  ControlMode previousMode;
  
  // 控制参数
  float controlOutput;
  float previousOutput;
  
  // MPC参数
  float predictionHorizon;
  float controlHorizon;
  
  // 性能指标
  float controlEffort;
  float trackingError;
  float energyConsumption;
  
  // 状态变量
  unsigned long lastControlTime;
  bool initialized;
  
public:
  ControlSystem();
  
  // 初始化控制系统
  bool initialize();
  
  // 设置控制模式
  void setControlMode(ControlMode mode);
  ControlMode getCurrentMode() const;
  ControlMode getPreviousMode() const;
  
  // 计算控制输出
  float computeControl(const SensorData& sensors, const DigitalTwinData& twin);
  
  // 执行控制动作
  void executeControl(float output);
  
  // 各种控制模式的具体实现
  float standardControl(const SensorData& sensors, const DigitalTwinData& twin);
  float energySavingControl(const SensorData& sensors, const DigitalTwinData& twin);
  float highEfficiencyControl(const SensorData& sensors, const DigitalTwinData& twin);
  float shockLoadControl(const SensorData& sensors, const DigitalTwinData& twin);
  float maintenanceControl(const SensorData& sensors, const DigitalTwinData& twin);
  
  // MPC控制
  float modelPredictiveControl(const SensorData& sensors, const DigitalTwinData& twin);
  
  // 自适应模糊PID控制
  float adaptiveFuzzyPID(const SensorData& sensors, const DigitalTwinData& twin);
  
  // 更新控制器参数
  void updatePIDParameters(float Kp, float Ki, float Kd);
  void updateFuzzyParameters(const LearningData& learningData);
  
  // 获取控制性能指标
  float getControlEffort() const;
  float getTrackingError() const;
  float getEnergyConsumption() const;
  
  // 重置控制系统
  void reset();
  
private:
  // 选择最优控制模式
  ControlMode selectOptimalMode(const SensorData& sensors, const DigitalTwinData& twin) const;
  
  // 评估控制成本
  float evaluateControlCost(float control, const SensorData& sensors, 
                           const DigitalTwinData& twin, ControlMode mode) const;
  
  // 调整PID参数（模糊逻辑）
  void adjustPIDWithFuzzyLogic(float error, float errorChange);
  
  // 模式切换处理
  void handleModeTransition(ControlMode newMode);
};

#endif // CONTROL_SYSTEM_H