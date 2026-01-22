#include "SystemState.h"

SystemStateManager::SystemStateManager() 
  : currentState(STATE_INITIALIZING), 
    previousState(STATE_INITIALIZING), 
    stateEntryTime(millis()),
    lastUpdateTime(millis()) {}

void SystemStateManager::setState(SystemState newState) {
  if (canTransitionTo(newState)) {
    previousState = currentState;
    currentState = newState;
    stateEntryTime = millis();
  }
}

SystemState SystemStateManager::getCurrentState() const {
  return currentState;
}

SystemState SystemStateManager::getPreviousState() const {
  return previousState;
}

unsigned long SystemStateManager::getStateDuration() const {
  return millis() - stateEntryTime;
}

void SystemStateManager::resetStateTimer() {
  stateEntryTime = millis();
}

bool SystemStateManager::isRunning() const {
  return currentState == STATE_RUNNING;
}

bool SystemStateManager::isError() const {
  return currentState == STATE_ERROR;
}

bool SystemStateManager::isMaintenance() const {
  return currentState == STATE_MAINTENANCE;
}

bool SystemStateManager::canTransitionTo(SystemState newState) const {
  // 简化版本：允许所有状态转换
  // 在实际应用中，这里应该有更复杂的逻辑
  return true;
}

bool SystemStateManager::validateTransition(SystemState newState) const {
  // 简化验证逻辑
  if (currentState == STATE_ERROR && newState != STATE_ERROR) {
    return false; // 错误状态下不能转换到其他状态
  }
  return true;
}