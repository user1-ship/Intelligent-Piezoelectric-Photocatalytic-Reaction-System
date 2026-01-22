#ifndef SERIAL_MONITOR_H
#define SERIAL_MONITOR_H

#include <Arduino.h>

class SerialMonitor {
private:
  // 显示配置
  struct DisplayConfig {
    bool enabled;
    uint16_t baudRate;
    uint8_t outputLevel; // 0: 错误, 1: 警告, 2: 信息, 3: 调试
  };
  
  DisplayConfig config;
  
  // 缓冲区
  String outputBuffer;
  
  // 时间管理
  unsigned long lastPrintTime;
  unsigned long minPrintInterval;
  
public:
  SerialMonitor();
  
  // 初始化
  bool initialize(uint16_t baudRate = 115200);
  
  // 配置
  void setOutputLevel(uint8_t level);
  void enable(bool enable);
  void setMinPrintInterval(unsigned long interval);
  
  // 基础打印方法
  void print(const String& message);
  void println(const String& message);
  
  // 格式化输出
  void printHeader(const String& title);
  void printSection(const String& section);
  void printKeyValue(const String& key, const String& value);
  
  // 修复数组参数问题：使用指针代替引用数组
  void printList(const String* items, uint8_t count);
  void printTable(const String* headers, const String* rows, 
                  uint8_t colCount, uint8_t rowCount);
  
  // 状态消息
  void printMessage(const String& message, unsigned long interval = 0);
  void printWarning(const String& warning, unsigned long interval = 0);
  void printError(const String& error, unsigned long interval = 0);
  void printDebug(const String& debug, unsigned long interval = 0);
  
  // 特殊格式
  void printSeparator(char ch = '=', uint8_t length = 40);
  void printProgressBar(uint8_t percentage, uint8_t width = 20);
  void printSystemHeader(const String& systemName);
  void printSystemStatus(const String& status, uint8_t level = 2);
  
  // 数据流输出
  void printDataStream(const String& label, float value, 
                      const String& unit = "", uint8_t decimals = 2);
  
  // 清屏和光标控制
  void clearScreen();
  void setCursor(uint8_t row, uint8_t col);
  
  // 获取状态
  bool isEnabled() const;
  uint8_t getOutputLevel() const;
  
private:
  // 内部方法
  bool shouldPrint(uint8_t messageLevel, unsigned long interval);
  String formatTime(unsigned long milliseconds);
  String formatValue(float value, uint8_t decimals, const String& unit);
  
  // 时间戳
  String getTimestamp();
  
  // 颜色控制（如果终端支持）
  void setColor(uint8_t colorCode);
  void resetColor();
};

#endif // SERIAL_MONITOR_H