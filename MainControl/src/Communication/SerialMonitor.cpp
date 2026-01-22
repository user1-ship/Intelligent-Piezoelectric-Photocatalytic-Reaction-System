#include "SerialMonitor.h"

SerialMonitor::SerialMonitor() {
  config.enabled = true;
  config.baudRate = 115200;
  config.outputLevel = 3; // 默认显示所有信息
  lastPrintTime = 0;
  minPrintInterval = 100; // 最小打印间隔100ms
  outputBuffer.reserve(256);
}

bool SerialMonitor::initialize(uint16_t baudRate) {
  config.baudRate = baudRate;
  Serial.begin(baudRate);
  
  // 等待串口连接
  unsigned long startTime = millis();
  while (!Serial && millis() - startTime < 2000) {
    delay(10);
  }
  
  return true;
}

void SerialMonitor::setOutputLevel(uint8_t level) {
  config.outputLevel = level;
}

void SerialMonitor::enable(bool enable) {
  config.enabled = enable;
}

void SerialMonitor::setMinPrintInterval(unsigned long interval) {
  minPrintInterval = interval;
}

void SerialMonitor::print(const String& message) {
  if (!config.enabled) return;
  Serial.print(message);
}

void SerialMonitor::println(const String& message) {
  if (!config.enabled) return;
  Serial.println(message);
}

void SerialMonitor::printHeader(const String& title) {
  if (!config.enabled || config.outputLevel < 2) return;
  
  println("");
  printSeparator('=', title.length() + 4);
  println("  " + title + "  ");
  printSeparator('=', title.length() + 4);
  println("");
}

void SerialMonitor::printSection(const String& section) {
  if (!config.enabled || config.outputLevel < 2) return;
  
  println("");
  println("=== " + section + " ===");
}

void SerialMonitor::printKeyValue(const String& key, const String& value) {
  if (!config.enabled || config.outputLevel < 2) return;
  
  // 对齐显示
  String formatted = "  " + key + ": ";
  while (formatted.length() < 20) {
    formatted += " ";
  }
  formatted += value;
  println(formatted);
}

void SerialMonitor::printList(const String* items, uint8_t count) {
  if (!config.enabled || config.outputLevel < 2) return;
  
  for (uint8_t i = 0; i < count; i++) {
    println("  - " + items[i]);
  }
}

void SerialMonitor::printTable(const String* headers, const String* rows, 
                               uint8_t colCount, uint8_t rowCount) {
  if (!config.enabled || config.outputLevel < 2) return;
  
  // 简化实现：打印表头
  String headerLine;
  for (uint8_t c = 0; c < colCount; c++) {
    headerLine += headers[c];
    if (c < colCount - 1) headerLine += " | ";
  }
  println(headerLine);
  
  // 打印分隔线
  String separator;
  for (uint8_t c = 0; c < colCount; c++) {
    for (uint8_t i = 0; i < headers[c].length(); i++) {
      separator += "-";
    }
    if (c < colCount - 1) separator += "-+-";
  }
  println(separator);
  
  // 打印行数据
  for (uint8_t r = 0; r < rowCount; r++) {
    String rowLine;
    for (uint8_t c = 0; c < colCount; c++) {
      rowLine += rows[r * colCount + c];
      if (c < colCount - 1) rowLine += " | ";
    }
    println(rowLine);
  }
}

void SerialMonitor::printMessage(const String& message, unsigned long interval) {
  if (!config.enabled || config.outputLevel < 2) return;
  if (!shouldPrint(2, interval)) return;
  
  println("[INFO] " + message);
}

void SerialMonitor::printWarning(const String& warning, unsigned long interval) {
  if (!config.enabled || config.outputLevel < 1) return;
  if (!shouldPrint(1, interval)) return;
  
  println("[WARN] " + warning);
}

void SerialMonitor::printError(const String& error, unsigned long interval) {
  if (!config.enabled || config.outputLevel < 0) return;
  if (!shouldPrint(0, interval)) return;
  
  println("[ERROR] " + error);
}

void SerialMonitor::printDebug(const String& debug, unsigned long interval) {
  if (!config.enabled || config.outputLevel < 3) return;
  if (!shouldPrint(3, interval)) return;
  
  println("[DEBUG] " + debug);
}

void SerialMonitor::printSeparator(char ch, uint8_t length) {
  if (!config.enabled) return;
  
  String separator;
  for (uint8_t i = 0; i < length; i++) {
    separator += ch;
  }
  println(separator);
}

void SerialMonitor::printProgressBar(uint8_t percentage, uint8_t width) {
  if (!config.enabled || config.outputLevel < 2) return;
  
  percentage = min(percentage, (uint8_t)100);
  uint8_t filled = (percentage * width) / 100;
  
  String bar = "[";
  for (uint8_t i = 0; i < width; i++) {
    if (i < filled) {
      bar += "=";
    } else {
      bar += " ";
    }
  }
  bar += "] " + String(percentage) + "%";
  
  println(bar);
}

void SerialMonitor::printSystemHeader(const String& systemName) {
  if (!config.enabled) return;
  
  println("");
  printSeparator('=', 50);
  println("         " + systemName);
  printSeparator('=', 50);
  println("");
}

void SerialMonitor::printSystemStatus(const String& status, uint8_t level) {
  if (!config.enabled || config.outputLevel < level) return;
  
  String timestamp = getTimestamp();
  println("[" + timestamp + "] " + status);
}

void SerialMonitor::printDataStream(const String& label, float value, 
                                   const String& unit, uint8_t decimals) {
  if (!config.enabled || config.outputLevel < 3) return;
  
  String formatted = label + ": " + String(value, decimals);
  if (unit.length() > 0) {
    formatted += " " + unit;
  }
  println(formatted);
}

void SerialMonitor::clearScreen() {
  if (!config.enabled) return;
  
  // 发送清屏序列（适用于大多数终端）
  Serial.print("\033[2J");  // 清屏
  Serial.print("\033[H");   // 光标移动到左上角
}

void SerialMonitor::setCursor(uint8_t row, uint8_t col) {
  if (!config.enabled) return;
  
  // 设置光标位置
  Serial.print("\033[");
  Serial.print(row);
  Serial.print(";");
  Serial.print(col);
  Serial.print("H");
}

bool SerialMonitor::isEnabled() const {
  return config.enabled;
}

uint8_t SerialMonitor::getOutputLevel() const {
  return config.outputLevel;
}

bool SerialMonitor::shouldPrint(uint8_t messageLevel, unsigned long interval) {
  if (interval == 0) return true;
  
  unsigned long currentTime = millis();
  if (currentTime - lastPrintTime >= interval) {
    lastPrintTime = currentTime;
    return true;
  }
  return false;
}

String SerialMonitor::formatTime(unsigned long milliseconds) {
  unsigned long seconds = milliseconds / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  
  seconds %= 60;
  minutes %= 60;
  
  char buffer[12];
  snprintf(buffer, sizeof(buffer), "%02lu:%02lu:%02lu", hours, minutes, seconds);
  return String(buffer);
}

String SerialMonitor::formatValue(float value, uint8_t decimals, const String& unit) {
  String formatted = String(value, decimals);
  if (unit.length() > 0) {
    formatted += " " + unit;
  }
  return formatted;
}

String SerialMonitor::getTimestamp() {
  unsigned long seconds = millis() / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  
  seconds %= 60;
  minutes %= 60;
  
  char buffer[12];
  snprintf(buffer, sizeof(buffer), "%02lu:%02lu:%02lu", hours, minutes, seconds);
  return String(buffer);
}

void SerialMonitor::setColor(uint8_t colorCode) {
  // 设置终端颜色（如果支持）
  Serial.print("\033[");
  Serial.print(colorCode);
  Serial.print("m");
}

void SerialMonitor::resetColor() {
  // 重置颜色
  Serial.print("\033[0m");
}