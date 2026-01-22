#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

class PIDController {
private:
  // PID参数
  float Kp, Ki, Kd;
  float integralTerm;
  float previousError;
  float previousMeasurement;
  
  // 限制参数
  float outputMin, outputMax;
  float integralMin, integralMax;
  
  // 抗饱和参数
  bool antiWindupEnabled;
  float windupThreshold;
  
  // 滤波器参数
  float derivativeFilterAlpha;
  float filteredDerivative;
  
  // 自适应参数
  bool adaptiveMode;
  float adaptiveGain;
  
public:
  PIDController(float Kp = 1.0f, float Ki = 0.1f, float Kd = 0.05f);
  
  // 配置PID参数
  void setParameters(float Kp, float Ki, float Kd);
  void getParameters(float& Kp, float& Ki, float& Kd) const;
  
  // 设置输出限制
  void setOutputLimits(float min, float max);
  void setIntegralLimits(float min, float max);
  
  // 抗饱和设置
  void enableAntiWindup(bool enable, float threshold = 0.8f);
  
  // 滤波器设置
  void setDerivativeFilter(float alpha);
  
  // 自适应设置
  void enableAdaptiveMode(bool enable, float gain = 0.1f);
  
  // 计算控制输出
  float compute(float setpoint, float measurement, float dt);
  
  // 重置控制器
  void reset();
  
  // 获取内部状态
  float getIntegralTerm() const;
  float getDerivativeTerm() const;
  float getProportionalTerm() const;
  
private:
  // 计算自适应增益
  float calculateAdaptiveGain(float error, float dt);
  
  // 抗饱和处理
  void handleIntegralWindup(float output, float dt);
};

#endif // PID_CONTROLLER_H