#ifndef TIMER_H
#define TIMER_H

#include <Arduino.h>

// 非阻塞定时器类
class Timer {
private:
  unsigned long interval;
  unsigned long previousTime;
  bool autoReset;
  
public:
  Timer(unsigned long interval, bool autoReset = true);
  
  // 检查定时器是否触发
  bool check();
  
  // 手动重置定时器
  void reset();
  
  // 设置新的时间间隔
  void setInterval(unsigned long newInterval);
  
  // 获取剩余时间
  unsigned long getRemainingTime() const;
  
  // 获取已过时间
  unsigned long getElapsedTime() const;
  
  // 获取时间间隔
  unsigned long getInterval() const;
};

// 多重定时器管理器
class TimerManager {
private:
  Timer** timers;
  uint8_t timerCount;
  uint8_t maxTimers;
  
public:
  TimerManager(uint8_t maxTimers = 10);
  ~TimerManager();
  
  // 添加定时器
  bool addTimer(Timer* timer);
  
  // 移除定时器
  bool removeTimer(uint8_t index);
  
  // 检查所有定时器
  void update();
  
  // 获取定时器数量
  uint8_t getTimerCount() const;
};

#endif // TIMER_H