#include "PIDController.h"
#include <Arduino.h>

PIDController::PIDController(float Kp, float Ki, float Kd)
  : Kp(Kp), Ki(Ki), Kd(Kd),
    integralTerm(0.0f),
    previousError(0.0f),
    previousMeasurement(0.0f),
    outputMin(0.0f),
    outputMax(100.0f),
    integralMin(-1000.0f),
    integralMax(1000.0f),
    antiWindupEnabled(true),
    windupThreshold(0.8f),
    derivativeFilterAlpha(0.3f),
    filteredDerivative(0.0f),
    adaptiveMode(false),
    adaptiveGain(0.1f) {}

void PIDController::setParameters(float Kp, float Ki, float Kd) {
  this->Kp = Kp;
  this->Ki = Ki;
  this->Kd = Kd;
}

void PIDController::getParameters(float& Kp, float& Ki, float& Kd) const {
  Kp = this->Kp;
  Ki = this->Ki;
  Kd = this->Kd;
}

void PIDController::setOutputLimits(float min, float max) {
  outputMin = min;
  outputMax = max;
}

void PIDController::setIntegralLimits(float min, float max) {
  integralMin = min;
  integralMax = max;
}

void PIDController::enableAntiWindup(bool enable, float threshold) {
  antiWindupEnabled = enable;
  windupThreshold = threshold;
}

void PIDController::setDerivativeFilter(float alpha) {
  derivativeFilterAlpha = alpha;
}

void PIDController::enableAdaptiveMode(bool enable, float gain) {
  adaptiveMode = enable;
  adaptiveGain = gain;
}

float PIDController::compute(float setpoint, float measurement, float dt) {
  if (dt <= 0.0f) dt = 0.01f; // 避免除以零
  
  float error = setpoint - measurement;
  
  // 比例项
  float proportional = Kp * error;
  
  // 积分项
  integralTerm += Ki * error * dt;
  
  // 抗饱和处理
  if (antiWindupEnabled) {
    integralTerm = constrain(integralTerm, integralMin, integralMax);
  }
  
  // 微分项
  float derivative = (measurement - previousMeasurement) / dt;
  
  // 应用导数滤波器
  filteredDerivative = derivativeFilterAlpha * derivative + 
                      (1.0f - derivativeFilterAlpha) * filteredDerivative;
  
  float derivativeTerm = -Kd * filteredDerivative; // 负号是因为是对测量值微分
  
  // 自适应调整
  if (adaptiveMode) {
    float adaptiveFactor = calculateAdaptiveGain(error, dt);
    proportional *= adaptiveFactor;
  }
  
  // 计算输出
  float output = proportional + integralTerm + derivativeTerm;
  
  // 抗饱和处理
  if (antiWindupEnabled && fabs(output) > windupThreshold * outputMax) {
    handleIntegralWindup(output, dt);
  }
  
  // 限制输出
  output = constrain(output, outputMin, outputMax);
  
  // 更新状态
  previousError = error;
  previousMeasurement = measurement;
  
  return output;
}

void PIDController::reset() {
  integralTerm = 0.0f;
  previousError = 0.0f;
  previousMeasurement = 0.0f;
  filteredDerivative = 0.0f;
}

float PIDController::getIntegralTerm() const {
  return integralTerm;
}

float PIDController::getDerivativeTerm() const {
  return filteredDerivative;
}

float PIDController::getProportionalTerm() const {
  return previousError * Kp;
}

float PIDController::calculateAdaptiveGain(float error, float dt) {
  // 简化自适应增益计算
  float adaptiveFactor = 1.0f + adaptiveGain * fabs(error);
  return adaptiveFactor;
}

void PIDController::handleIntegralWindup(float output, float dt) {
  // 简化抗饱和处理
  if (fabs(output) > outputMax) {
    integralTerm *= 0.95f; // 减小积分项
  }
}