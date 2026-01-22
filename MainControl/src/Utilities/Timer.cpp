#include "Timer.h"

Timer::Timer(unsigned long interval, bool autoReset)
  : interval(interval), previousTime(millis()), autoReset(autoReset) {}

bool Timer::check() {
  unsigned long currentTime = millis();
  
  // 处理时间溢出
  if (currentTime < previousTime) {
    previousTime = currentTime;
    return false;
  }
  
  if (currentTime - previousTime >= interval) {
    if (autoReset) {
      previousTime = currentTime;
    }
    return true;
  }
  return false;
}

void Timer::reset() {
  previousTime = millis();
}

void Timer::setInterval(unsigned long newInterval) {
  interval = newInterval;
  reset();
}

unsigned long Timer::getRemainingTime() const {
  unsigned long currentTime = millis();
  if (currentTime < previousTime) {
    return interval;
  }
  
  unsigned long elapsed = currentTime - previousTime;
  if (elapsed >= interval) {
    return 0;
  }
  return interval - elapsed;
}

unsigned long Timer::getElapsedTime() const {
  unsigned long currentTime = millis();
  if (currentTime < previousTime) {
    return 0;
  }
  return min(currentTime - previousTime, interval);
}

unsigned long Timer::getInterval() const {
  return interval;
}

// TimerManager 实现
TimerManager::TimerManager(uint8_t maxTimers) 
  : maxTimers(maxTimers), timerCount(0) {
  timers = new Timer*[maxTimers];
  for (uint8_t i = 0; i < maxTimers; i++) {
    timers[i] = nullptr;
  }
}

TimerManager::~TimerManager() {
  delete[] timers;
}

bool TimerManager::addTimer(Timer* timer) {
  if (timerCount >= maxTimers || timer == nullptr) {
    return false;
  }
  
  for (uint8_t i = 0; i < maxTimers; i++) {
    if (timers[i] == nullptr) {
      timers[i] = timer;
      timerCount++;
      return true;
    }
  }
  return false;
}

bool TimerManager::removeTimer(uint8_t index) {
  if (index >= maxTimers || timers[index] == nullptr) {
    return false;
  }
  
  timers[index] = nullptr;
  timerCount--;
  return true;
}

void TimerManager::update() {
  for (uint8_t i = 0; i < maxTimers; i++) {
    if (timers[i] != nullptr) {
      timers[i]->check();
    }
  }
}

uint8_t TimerManager::getTimerCount() const {
  return timerCount;
}