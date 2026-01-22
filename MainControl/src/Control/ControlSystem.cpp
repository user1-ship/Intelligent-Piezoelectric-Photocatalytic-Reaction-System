#include "ControlSystem.h"

ControlSystem::ControlSystem() 
  : currentMode(STANDARD),
    previousMode(STANDARD),
    controlOutput(0.0f),
    previousOutput(0.0f),
    predictionHorizon(3.0f),
    controlHorizon(2.0f),
    controlEffort(0.0f),
    trackingError(0.0f),
    energyConsumption(0.0f),
    lastControlTime(0),
    initialized(false) {}

bool ControlSystem::initialize() {
  stressServo.attach(SERVO_PIN);
  pinMode(BUZZER_PIN, OUTPUT);
  stressServo.write(0);
  
  // 初始化PID控制器
  pidController = PIDController(1.0f, 0.1f, 0.05f);
  
  // 初始化模糊系统（简化）
  
  initialized = true;
  return true;
}

void ControlSystem::setControlMode(ControlMode mode) {
  previousMode = currentMode;
  currentMode = mode;
  handleModeTransition(mode);
}

ControlMode ControlSystem::getCurrentMode() const {
  return currentMode;
}

ControlMode ControlSystem::getPreviousMode() const {
  return previousMode;
}

float ControlSystem::computeControl(const SensorData& sensors, const DigitalTwinData& twin) {
  float output = 0.0f;
  
  switch (currentMode) {
    case ENERGY_SAVING:
      output = energySavingControl(sensors, twin);
      break;
    case HIGH_EFFICIENCY:
      output = highEfficiencyControl(sensors, twin);
      break;
    case SHOCK_LOAD:
      output = shockLoadControl(sensors, twin);
      break;
    case MAINTENANCE:
      output = maintenanceControl(sensors, twin);
      break;
    default: // STANDARD
      output = standardControl(sensors, twin);
  }
  
  controlOutput = output;
  return output;
}

void ControlSystem::executeControl(float output) {
  int servoPos = map(output, 0, 100, 0, 180);
  stressServo.write(servoPos);
  previousOutput = output;
  
  // 更新控制性能指标
  controlEffort = output;
  energyConsumption = output * 0.8f; // 简化能耗计算
}

float ControlSystem::standardControl(const SensorData& sensors, const DigitalTwinData& twin) {
  return adaptiveFuzzyPID(sensors, twin);
}

float ControlSystem::energySavingControl(const SensorData& sensors, const DigitalTwinData& twin) {
  float baseControl = adaptiveFuzzyPID(sensors, twin);
  return baseControl * 0.7f; // 节能模式下降低控制强度
}

float ControlSystem::highEfficiencyControl(const SensorData& sensors, const DigitalTwinData& twin) {
  return modelPredictiveControl(sensors, twin);
}

float ControlSystem::shockLoadControl(const SensorData& sensors, const DigitalTwinData& twin) {
  float baseControl = adaptiveFuzzyPID(sensors, twin);
  return min(100.0f, baseControl * 1.5f); // 冲击负荷时增强控制
}

float ControlSystem::maintenanceControl(const SensorData& sensors, const DigitalTwinData& twin) {
  // 维护模式下保守运行
  return 30.0f;
}

float ControlSystem::modelPredictiveControl(const SensorData& sensors, const DigitalTwinData& twin) {
  float bestControl = 50.0f; // 默认值
  float bestCost = 1e9;
  
  // 简化的MPC：评估几个可能的控制量
  for (int testControl = 0; testControl <= 100; testControl += 20) {
    float cost = evaluateControlCost(testControl, sensors, twin, HIGH_EFFICIENCY);
    
    if (cost < bestCost) {
      bestCost = cost;
      bestControl = testControl;
    }
  }
  
  return bestControl;
}

float ControlSystem::adaptiveFuzzyPID(const SensorData& sensors, const DigitalTwinData& twin) {
  float error = sensors.pollutionLevel - twin.optimalSetpoint;
  float dt = 0.1f; // 假设时间间隔
  
  // 使用PID控制器
  float output = pidController.compute(twin.optimalSetpoint, sensors.pollutionLevel, dt);
  
  // 应用输出限制
  output = constrain(output, 0.0f, 100.0f);
  
  // 更新跟踪误差
  trackingError = fabs(error);
  
  return output;
}

void ControlSystem::updatePIDParameters(float Kp, float Ki, float Kd) {
  pidController.setParameters(Kp, Ki, Kd);
}

void ControlSystem::updateFuzzyParameters(const LearningData& learningData) {
  // 简化实现
}

float ControlSystem::getControlEffort() const {
  return controlEffort;
}

float ControlSystem::getTrackingError() const {
  return trackingError;
}

float ControlSystem::getEnergyConsumption() const {
  return energyConsumption;
}

void ControlSystem::reset() {
  stressServo.write(0);
  controlOutput = 0.0f;
  previousOutput = 0.0f;
  controlEffort = 0.0f;
  trackingError = 0.0f;
  energyConsumption = 0.0f;
  lastControlTime = millis();
}

ControlMode ControlSystem::selectOptimalMode(const SensorData& sensors, const DigitalTwinData& twin) const {
  // 简化模式选择逻辑
  if (sensors.pollutionLevel > 300.0f) return SHOCK_LOAD;
  if (sensors.energyUsage > MAX_ENERGY_USAGE * 0.8f) return ENERGY_SAVING;
  if (twin.systemHealth < 70.0f) return MAINTENANCE;
  return STANDARD;
}

float ControlSystem::evaluateControlCost(float control, const SensorData& sensors, 
                                         const DigitalTwinData& twin, ControlMode mode) const {
  // 简化成本评估
  float pollutionCost = fabs(twin.predictedPollution - control * 0.5f - TARGET_POLLUTION);
  float energyCost = control * 0.8f;
  float lifeCost = control > 80.0f ? (control - 80.0f) * 2.0f : 0.0f;
  
  return pollutionCost + energyCost + lifeCost;
}

void ControlSystem::adjustPIDWithFuzzyLogic(float error, float errorChange) {
  // 简化模糊逻辑调整
}

void ControlSystem::handleModeTransition(ControlMode newMode) {
  // 模式切换处理
  if (DEBUG_MODE) {
    Serial.print("控制模式切换: ");
    Serial.print((int)previousMode);
    Serial.print(" -> ");
    Serial.println((int)newMode);
  }
}