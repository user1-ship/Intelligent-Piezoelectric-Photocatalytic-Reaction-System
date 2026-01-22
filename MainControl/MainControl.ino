// ========== 高级智能压电光催化反应装置控制程序 ==========
// 用于Arduino Mega 2560

#include <Arduino.h>
#include <Servo.h>
#include <EEPROM.h>

// 核心模块
#include "src/Core/SystemConfig.h"
#include "src/Core/SystemState.h"
#include "src/Core/CommonTypes.h"

// 工具模块
#include "src/Utilities/Timer.h"
#include "src/Utilities/MathUtils.h"

// 功能模块
#include "src/Sensors/SensorManager.h"
#include "src/Sensors/SensorFusion.h"
#include "src/Control/ControlSystem.h"
#include "src/Model/DigitalTwin.h"
#include "src/Learning/LearningSystem.h"
#include "src/Learning/DataStorage.h"
#include "src/Communication/SerialMonitor.h"
// ========== 在原有包含后添加 ==========
#include "src/Communication/WiFiComm.h"

// ========== 全局对象实例 ==========
SystemStateManager stateManager;
TimerManager timerManager;

// 传感器模块
SensorManager sensorManager;
SensorFusion sensorFusion;

// 控制模块
ControlSystem controlSystem;

// 模型模块
DigitalTwin digitalTwin;

// 学习模块
LearningSystem learningSystem;
DataStorage dataStorage;

// 通信模块
SerialMonitor serialMonitor;

// ========== 在全局对象后添加 ==========
// WiFi通信模块
WiFiComm wifiComm;

// 定时器
Timer sensorTimer(SAMPLING_INTERVAL);
Timer controlTimer(CONTROL_INTERVAL);
Timer learningTimer(LEARNING_INTERVAL);
Timer displayTimer(5000);  // 5秒显示更新

// 全局数据
SensorData currentSensors;
DigitalTwinData currentTwin;
ControlDecision currentDecision;

// ========== 辅助函数声明 ==========
float calculateEnergyUsage(const SensorData& sensors);
float calculateSystemEfficiency(const SensorData& sensors);
ControlDecision makeControlDecision(const SensorData& sensors, const DigitalTwinData& twin);
String generateDecisionReasoning(const SensorData& sensors, const DigitalTwinData& twin, ControlMode mode);
void displaySystemStatus();
void logSystemData();
void handleSerialCommands();
void resetSystem();
void calibrateSensors();

// ========== 其他状态处理函数 ==========
void optimizingState();
void maintenanceState();
void emergencyState();
void errorState();
void idleState();

// ========== 新增WiFi处理函数 ==========
void handleWiFiCommands();
void sendDataToWiFi();

// ========== 系统初始化 ==========
void setup() {
  // 初始化串口通信
  serialMonitor.initialize(115200);
  
  // 显示启动信息
  serialMonitor.printSystemHeader("高级智能压电光催化系统 V3.0");
  serialMonitor.printMessage("系统初始化中...");
  
  // 设置系统状态
  stateManager.setState(STATE_INITIALIZING);
  
  // 初始化各模块
  bool initSuccess = true;
  
  serialMonitor.printMessage("初始化传感器模块...");
  initSuccess &= sensorManager.initialize();
  
  serialMonitor.printMessage("初始化控制模块...");
  initSuccess &= controlSystem.initialize();
  
  serialMonitor.printMessage("初始化数字孪生模块...");
  initSuccess &= digitalTwin.initialize();
  
  serialMonitor.printMessage("初始化学习系统...");
  initSuccess &= learningSystem.initialize();
  
  serialMonitor.printMessage("初始化数据存储...");
  initSuccess &= dataStorage.initialize();
  
  // ========== 在setup()中添加 ==========
  // 初始化WiFi通信
  serialMonitor.printMessage("初始化WiFi通信模块...");
  
  WiFiConfig wifiConfig;
  wifiConfig.ssid = "PiezoCatalyticSystem";
  wifiConfig.password = "12345678";
  wifiConfig.hostname = "piezocatalytic";
  wifiConfig.apMode = true;
  wifiConfig.rxPin = 19;  // RX引脚
  wifiConfig.txPin = 18;  // TX引脚
  wifiConfig.baudRate = 115200;
  wifiConfig.heartbeatInterval = 5000;
  
  if (wifiComm.initialize(wifiConfig)) {
    serialMonitor.printMessage("WiFi通信模块初始化成功");
    wifiComm.sendLogMessage("压电光催化系统启动完成");
  } else {
    serialMonitor.printError("WiFi通信模块初始化失败");
  }
  
  serialMonitor.printMessage("初始化定时器...");
  timerManager.addTimer(&sensorTimer);
  timerManager.addTimer(&controlTimer);
  timerManager.addTimer(&learningTimer);
  timerManager.addTimer(&displayTimer);
  
  // 检查初始化结果
  if (initSuccess) {
    stateManager.setState(STATE_RUNNING);
    serialMonitor.printMessage("系统初始化完成，进入运行模式");
    serialMonitor.printSeparator();
  } else {
    stateManager.setState(STATE_ERROR);
    serialMonitor.printError("系统初始化失败，请检查硬件连接");
    while (true) {
      // 等待重启
      delay(1000);
    }
  }
}

// ========== 主控制循环 ==========
void loop() {
  // 更新定时器
  timerManager.update();
  
  // ========== 在loop()中添加 ==========
  // 更新WiFi通信
  wifiComm.update();
  
  // 处理WiFi命令
  handleWiFiCommands();
  
  // 根据系统状态执行相应操作
  switch (stateManager.getCurrentState()) {
    case STATE_RUNNING:
      runningState();
      break;
      
    case STATE_OPTIMIZING:
      optimizingState();
      break;
      
    case STATE_MAINTENANCE:
      maintenanceState();
      break;
      
    case STATE_EMERGENCY:
      emergencyState();
      break;
      
    case STATE_ERROR:
      errorState();
      break;
      
    default:
      idleState();
      break;
  }
  
  // 处理串口命令
  handleSerialCommands();
}

// ========== 运行状态处理 ==========
void runningState() {
  // 传感器数据采集
  if (sensorTimer.check()) {
    currentSensors = sensorManager.readAllSensors();
    
    // 传感器数据融合
    currentSensors.pollutionLevel = sensorFusion.fuseSensorData(currentSensors);
    
    // 计算能耗和效率
    currentSensors.energyUsage = calculateEnergyUsage(currentSensors);
    currentSensors.systemEfficiency = calculateSystemEfficiency(currentSensors);
  }
  
  // 控制计算
  if (controlTimer.check()) {
    // 数字孪生仿真
    currentTwin = digitalTwin.simulate(currentSensors);
    
    // 智能决策
    currentDecision = makeControlDecision(currentSensors, currentTwin);
    
    // 执行控制
    controlSystem.executeControl(currentDecision.controlOutput);
  }
  
  // 学习更新
  if (learningTimer.check()) {
    learningSystem.performOnlineLearning(currentSensors, currentTwin);
  }
  
  // 显示更新
  if (displayTimer.check()) {
    displaySystemStatus();
  }
  
  // ========== 在runningState()中添加 ==========
  // 发送数据到WiFi
  static unsigned long lastWiFiSend = 0;
  if (millis() - lastWiFiSend >= 1000) { // 每1秒发送一次
    lastWiFiSend = millis();
    sendDataToWiFi();
  }
  
  // 数据记录（可选）
  static unsigned long lastLogTime = 0;
  if (millis() - lastLogTime > 10000) { // 每10秒记录一次
    lastLogTime = millis();
    logSystemData();
  }
}

// ========== 辅助函数实现 ==========
float calculateEnergyUsage(const SensorData& sensors) {
  // 简化计算：基于流量和控制输出
  float baseEnergy = 20.0f; // 基础能耗
  float flowFactor = sensors.flowRate / 50.0f; // 参考流速50cm/s
  float controlFactor = currentDecision.controlOutput / 50.0f; // 参考控制输出50%
  
  return baseEnergy * (0.4f + 0.3f * flowFactor + 0.3f * controlFactor);
}

float calculateSystemEfficiency(const SensorData& sensors) {
  // 简化计算：基于污染物去除率和能耗
  float removalEfficiency = 1.0f - (sensors.pollutionLevel / POLLUTION_MAX);
  float energyEfficiency = 1.0f - (sensors.energyUsage / 100.0f);
  
  return (removalEfficiency * 0.7f + energyEfficiency * 0.3f) * 100.0f;
}

ControlDecision makeControlDecision(const SensorData& sensors, const DigitalTwinData& twin) {
  ControlDecision decision;
  
  // 模式选择
  decision.mode = controlSystem.getCurrentMode();
  
  // 计算控制输出
  decision.controlOutput = controlSystem.computeControl(sensors, twin);
  
  // 生成决策理由
  decision.reasoning = generateDecisionReasoning(sensors, twin, decision.mode);
  
  return decision;
}

String generateDecisionReasoning(const SensorData& sensors, const DigitalTwinData& twin, ControlMode mode) {
  String reasoning;
  
  switch (mode) {
    case ENERGY_SAVING:
      reasoning = "节能模式：能耗(" + String(sensors.energyUsage, 1) + 
                  "%)超过阈值，降低控制强度";
      break;
      
    case HIGH_EFFICIENCY:
      reasoning = "高效模式：污染物浓度(" + String(sensors.pollutionLevel, 1) + 
                  "ppm)较高，提高处理效率";
      break;
      
    case SHOCK_LOAD:
      reasoning = "冲击负荷模式：检测到浓度激增，启动应急处理";
      break;
      
    case MAINTENANCE:
      reasoning = "维护模式：系统健康度(" + String(twin.systemHealth, 1) + 
                  "%)过低，建议维护";
      break;
      
    default: // STANDARD
      reasoning = "标准模式：系统运行正常，污染物浓度" + 
                  String(sensors.pollutionLevel, 1) + "ppm";
      break;
  }
  
  return reasoning;
}

void displaySystemStatus() {
  if (!DEBUG_MODE) return;
  
  serialMonitor.printSeparator();
  serialMonitor.printSection("系统状态");
  
  serialMonitor.printKeyValue("系统状态", String(stateManager.getCurrentState()));
  serialMonitor.printKeyValue("控制模式", String(currentDecision.mode));
  serialMonitor.printKeyValue("控制输出", String(currentDecision.controlOutput, 1) + "%");
  
  serialMonitor.printSection("传感器数据");
  serialMonitor.printKeyValue("流量", String(currentSensors.flowRate, 1) + " cm/s");
  serialMonitor.printKeyValue("污染物", String(currentSensors.pollutionLevel, 1) + " ppm");
  serialMonitor.printKeyValue("光照", String(currentSensors.lightIntensity, 0) + " lux");
  serialMonitor.printKeyValue("pH值", String(currentSensors.pH, 1));
  serialMonitor.printKeyValue("温度", String(currentSensors.temperature, 1) + " °C");
  
  serialMonitor.printSection("系统性能");
  serialMonitor.printKeyValue("系统效率", String(currentSensors.systemEfficiency, 1) + "%");
  serialMonitor.printKeyValue("能耗", String(currentSensors.energyUsage, 1) + "%");
  serialMonitor.printKeyValue("健康度", String(currentTwin.systemHealth, 1) + "%");
  serialMonitor.printKeyValue("剩余寿命", String(currentTwin.remainingLife, 1) + "%");
  
  serialMonitor.printSeparator();
}

void logSystemData() {
  // 记录传感器数据
  dataStorage.logSensorData(currentSensors, millis());
  
  // 记录控制决策
  dataStorage.logControlData(currentDecision, millis());
  
  // 记录系统状态
  dataStorage.logSystemStatus(currentTwin, millis());
}

void handleSerialCommands() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    serialMonitor.printMessage("收到命令: " + command);
    
    // 解析和执行命令
    if (command == "status") {
      displaySystemStatus();
    } else if (command == "reset") {
      serialMonitor.printMessage("重置系统...");
      resetSystem();
    } else if (command.startsWith("mode ")) {
      String modeStr = command.substring(5);
      ControlMode mode = static_cast<ControlMode>(modeStr.toInt());
      controlSystem.setControlMode(mode);
      serialMonitor.printMessage("切换到模式: " + String(mode));
    } else if (command == "calibrate") {
      serialMonitor.printMessage("开始传感器校准...");
      calibrateSensors();
    } else if (command == "help") {
      serialMonitor.printSection("可用命令");
      serialMonitor.println("  status     - 显示系统状态");
      serialMonitor.println("  mode <n>   - 切换控制模式 (0-4)");
      serialMonitor.println("  calibrate  - 开始传感器校准");
      serialMonitor.println("  reset      - 重置系统");
      serialMonitor.println("  help       - 显示帮助信息");
    } else {
      serialMonitor.printError("未知命令: " + command);
      serialMonitor.println("使用 'help' 命令查看可用命令列表");
    }
  }
}

// ========== 新增WiFi处理函数实现 ==========
void handleWiFiCommands() {
  if (wifiComm.hasCommand()) {
    WiFiCommand cmd = wifiComm.getCommand();
    
    // 处理命令
    if (cmd.resetRequested) {
      serialMonitor.printMessage("收到WiFi重置命令");
      resetSystem();
      wifiComm.sendLogMessage("系统已重置");
    }
    
    if (cmd.calibrateRequested) {
      serialMonitor.printMessage("收到WiFi校准命令");
      calibrateSensors();
      wifiComm.sendLogMessage("传感器校准完成");
    }
    
    if (cmd.manualOverride) {
      // 手动控制模式
      controlSystem.setControlMode(MAINTENANCE);
      controlSystem.executeControl(cmd.manualOutput);
      
      String logMsg = "手动控制: " + String(cmd.manualOutput, 1) + "%";
      serialMonitor.printMessage(logMsg);
      wifiComm.sendLogMessage(logMsg);
    } else if (cmd.commandType == "setMode") {
      // 切换控制模式
      controlSystem.setControlMode(static_cast<ControlMode>(cmd.mode));
      
      String modeNames[] = {"节能模式", "标准模式", "高效模式", "冲击负荷模式", "维护模式"};
      if (cmd.mode < 5) {
        String logMsg = "切换到" + modeNames[cmd.mode];
        serialMonitor.printMessage(logMsg);
        wifiComm.sendLogMessage(logMsg);
      }
    }
    
    // 清除已处理的命令
    wifiComm.clearCommand();
  }
}

void sendDataToWiFi() {
  if (!wifiComm.isConnected()) return;
  
  // 发送传感器数据
  wifiComm.sendSensorData(currentSensors);
  
  // 发送控制数据
  wifiComm.sendControlData(currentDecision);
  
  // 发送数字孪生数据
  wifiComm.sendTwinData(currentTwin);
}

void resetSystem() {
  // 重置各模块
  for (int i = 0; i < 5; i++) {
    sensorManager.resetSensor(i);
  }
  controlSystem.reset();
  digitalTwin.reset();
  learningSystem.reset();
  
  // 重置定时器
  sensorTimer.reset();
  controlTimer.reset();
  learningTimer.reset();
  displayTimer.reset();
  
  // 重置状态
  stateManager.setState(STATE_RUNNING);
  
  serialMonitor.printMessage("系统重置完成");
}

void calibrateSensors() {
  // 简化校准过程
  stateManager.setState(STATE_CALIBRATING);
  
  serialMonitor.printMessage("传感器校准程序启动");
  serialMonitor.printMessage("请确保传感器处于校准状态...");
  delay(3000);
  
  // 这里可以实现具体的校准逻辑
  for (int i = 0; i < 5; i++) {
    serialMonitor.printMessage("校准传感器 " + String(i) + "...");
    // sensorManager.calibrateSensor(i, knownValue);
    delay(1000);
  }
  
  stateManager.setState(STATE_RUNNING);
  serialMonitor.printMessage("传感器校准完成");
}

// ========== 其他状态处理函数实现 ==========
void optimizingState() {
  // 系统优化逻辑
  serialMonitor.printMessage("系统优化中...");
  
  // 这里可以实现具体的优化算法
  // 例如：参数调整、控制策略优化等
  
  // 模拟优化过程
  static unsigned long optimizationStart = millis();
  static int optimizationStep = 0;
  
  if (millis() - optimizationStart > 2000) {
    optimizationStep++;
    optimizationStart = millis();
    
    switch (optimizationStep) {
      case 1:
        serialMonitor.printMessage("优化步骤1: 调整PID参数...");
        break;
      case 2:
        serialMonitor.printMessage("优化步骤2: 更新传感器融合权重...");
        break;
      case 3:
        serialMonitor.printMessage("优化步骤3: 调整控制策略...");
        break;
      default:
        serialMonitor.printMessage("优化完成，返回运行模式");
        stateManager.setState(STATE_RUNNING);
        optimizationStep = 0;
        return;
    }
  }
  
  // 临时降低控制频率以节省资源
  delay(100);
}

void maintenanceState() {
  // 维护模式逻辑
  static bool maintenanceAlertSent = false;
  
  if (!maintenanceAlertSent) {
    serialMonitor.printError("系统进入维护模式");
    serialMonitor.printMessage("建议执行以下维护操作:");
    serialMonitor.println("  1. 清洁传感器");
    serialMonitor.println("  2. 检查催化剂状态");
    serialMonitor.println("  3. 校准所有传感器");
    serialMonitor.println("  4. 检查执行器连接");
    
    maintenanceAlertSent = true;
  }
  
  // 执行维护操作
  controlSystem.setControlMode(MAINTENANCE);
  
  // 简化维护检测：模拟维护过程
  static unsigned long maintenanceStart = millis();
  
  // 每5秒检查一次是否退出维护模式
  if (millis() - maintenanceStart > 5000) {
    maintenanceStart = millis();
    
    // 模拟维护后系统健康度提升
    if (currentTwin.systemHealth < 80.0f) {
      // 模拟维护操作
      currentTwin.systemHealth += 5.0f;
      currentTwin.systemHealth = min(currentTwin.systemHealth, 100.0f);
      
      serialMonitor.printMessage("维护操作执行中... 健康度: " + 
                               String(currentTwin.systemHealth, 1) + "%");
    }
    
    // 检查是否可以退出维护模式
    if (currentTwin.systemHealth > 80.0f) {
      stateManager.setState(STATE_RUNNING);
      controlSystem.setControlMode(STANDARD);
      maintenanceAlertSent = false;
      serialMonitor.printMessage("维护完成，返回运行模式");
    }
  }
}

void emergencyState() {
  // 紧急状态处理
  static bool emergencyAlertSent = false;
  
  if (!emergencyAlertSent) {
    serialMonitor.printError("!!! 紧急状态 !!!");
    serialMonitor.printError("污染物浓度过高: " + String(currentSensors.pollutionLevel, 1) + "ppm");
    serialMonitor.printMessage("启动应急处理程序...");
    
    emergencyAlertSent = true;
  }
  
  // 紧急停止或安全处理
  controlSystem.executeControl(100); // 最大控制输出
  
  // 检查是否可以恢复
  static unsigned long emergencyStart = millis();
  
  if (millis() - emergencyStart > 2000) {
    emergencyStart = millis();
    
    if (currentSensors.pollutionLevel < 400.0f) {
      stateManager.setState(STATE_RUNNING);
      emergencyAlertSent = false;
      serialMonitor.printMessage("紧急状态解除，恢复运行");
    } else {
      serialMonitor.printWarning("污染物浓度仍然过高: " + 
                                String(currentSensors.pollutionLevel, 1) + "ppm");
    }
  }
}

void errorState() {
  // 错误状态处理
  static unsigned long errorStartTime = millis();
  static int recoveryAttempts = 0;
  
  if (millis() - errorStartTime > 5000) {
    recoveryAttempts++;
    
    if (recoveryAttempts <= 3) {
      serialMonitor.printWarning("尝试恢复系统 (尝试 " + String(recoveryAttempts) + "/3)...");
      
      // 尝试恢复各模块
      bool recoverySuccess = true;
      recoverySuccess &= sensorManager.initialize();
      recoverySuccess &= controlSystem.initialize();
      
      if (recoverySuccess) {
        stateManager.setState(STATE_RUNNING);
        recoveryAttempts = 0;
        serialMonitor.printMessage("系统恢复成功，返回运行模式");
      } else {
        serialMonitor.printError("恢复失败，将在5秒后重试");
      }
    } else {
      serialMonitor.printError("多次恢复尝试失败，系统停止");
      serialMonitor.printMessage("请检查硬件连接并重启系统");
      
      // 进入死循环等待人工干预
      while (true) {
        delay(1000);
      }
    }
    
    errorStartTime = millis();
  }
}

void idleState() {
  // 空闲状态处理
  static unsigned long idleStart = millis();
  
  // 每10秒显示一次状态
  if (millis() - idleStart > 10000) {
    serialMonitor.printMessage("系统处于空闲状态，等待命令...");
    idleStart = millis();
  }
  
  // 简单的指示
  static unsigned long lastBlink = millis();
  if (millis() - lastBlink > 1000) {
    static bool ledState = false;
    digitalWrite(LED_BUILTIN, ledState ? HIGH : LOW);
    ledState = !ledState;
    lastBlink = millis();
  }
  
  delay(100);
}