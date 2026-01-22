#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

#include <Arduino.h>

// 系统状态枚举
enum SystemState : uint8_t {
  STATE_INITIALIZING = 0,  // 初始化中
  STATE_CALIBRATING,       // 校准中
  STATE_RUNNING,           // 正常运行
  STATE_OPTIMIZING,        // 优化中
  STATE_MAINTENANCE,       // 维护模式
  STATE_EMERGENCY,         // 紧急状态
  STATE_ERROR              // 错误状态
};

// 系统状态管理器
class SystemStateManager {
private:
  SystemState currentState;
  SystemState previousState;
  unsigned long stateEntryTime;
  unsigned long lastUpdateTime;
  
public:
  SystemStateManager();
  
  // 状态管理
  void setState(SystemState newState);
  SystemState getCurrentState() const;
  SystemState getPreviousState() const;
  
  // 状态时间管理
  unsigned long getStateDuration() const;
  void resetStateTimer();
  
  // 状态检查
  bool isRunning() const;
  bool isError() const;
  bool isMaintenance() const;
  
  // 状态转换检查
  bool canTransitionTo(SystemState newState) const;
  
private:
  // 状态转换验证
  bool validateTransition(SystemState newState) const;
};

#endif // SYSTEM_STATE_H