#include "WiFiComm.h"
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

WiFiComm::WiFiComm() 
    : espSerial(nullptr), 
      initialized(false), 
      connected(false),
      lastHeartbeat(0),
      lastDataSend(0) {
    
    // 默认配置
    config.ssid = "PiezoCatalyticSystem";
    config.password = "12345678";
    config.hostname = "piezocatalytic";
    config.apMode = true;
    config.rxPin = 19;  // 默认RX引脚
    config.txPin = 18;  // 默认TX引脚
    config.baudRate = 115200;
    config.heartbeatInterval = 5000;
    
    // 初始化命令
    currentCommand.mode = 1; // 标准模式
    currentCommand.target = 100.0f;
    currentCommand.manualOverride = false;
    currentCommand.manualOutput = 0.0f;
    currentCommand.resetRequested = false;
    currentCommand.calibrateRequested = false;
    currentCommand.commandType = "";
}

WiFiComm::~WiFiComm() {
    if (espSerial != nullptr) {
        delete espSerial;
    }
}

bool WiFiComm::initialize(const WiFiConfig& cfg) {
    config = cfg;
    
    // 初始化软件串口
    espSerial = new SoftwareSerial(config.rxPin, config.txPin);
    espSerial->begin(config.baudRate);
    
    // 等待初始化
    delay(1000);
    
    // 清空缓冲区
    while (espSerial->available()) {
        espSerial->read();
    }
    
    // 发送初始化命令到ESP8266
    sendLogMessage("WiFi模块初始化中...");
    
    // 配置ESP8266（AT命令）
    espSerial->println("AT");
    delay(1000);
    
    // 检查响应
    if (espSerial->available()) {
        String response = espSerial->readString();
        if (response.indexOf("OK") != -1) {
            sendLogMessage("WiFi模块响应正常");
            
            // 设置WiFi模式
            if (config.apMode) {
                espSerial->println("AT+CWMODE=2"); // AP模式
            } else {
                espSerial->println("AT+CWMODE=1"); // STA模式
                String cmd = "AT+CWJAP=\"" + config.ssid + "\",\"" + config.password + "\"";
                espSerial->println(cmd);
            }
            delay(2000);
            
            // 启动服务器
            espSerial->println("AT+CIPMUX=1");
            delay(500);
            espSerial->println("AT+CIPSERVER=1,80");
            delay(500);
            
            initialized = true;
            connected = true;
            
            sendLogMessage("WiFi通信模块初始化完成");
            return true;
        }
    }
    
    sendLogMessage("WiFi模块初始化失败", 0);
    return false;
}

void WiFiComm::update() {
    if (!initialized) return;
    
    unsigned long currentMillis = millis();
    
    // 接收数据
    while (espSerial->available()) {
        char c = espSerial->read();
        receiveBuffer += c;
        
        if (c == '\n') {
            receiveBuffer.trim();
            if (receiveBuffer.length() > 0) {
                processReceivedData(receiveBuffer);
                receiveBuffer = "";
            }
        }
    }
    
    // 发送心跳
    if (currentMillis - lastHeartbeat >= config.heartbeatInterval) {
        lastHeartbeat = currentMillis;
        sendHeartbeat();
    }
    
    // 检查连接状态（简化）
    if (currentMillis - lastHeartbeat > config.heartbeatInterval * 2) {
        connected = false;
    }
}

void WiFiComm::processReceivedData(String data) {
    // 记录原始数据
    sendLogMessage("收到数据: " + data, 3);
    
    // 尝试解析JSON
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, data);
    
    if (!error) {
        // JSON格式命令
        if (doc.containsKey("command")) {
            String command = doc["command"];
            currentCommand.commandType = command;
            
            if (command == "setMode") {
                currentCommand.mode = doc["mode"] | 1;
                currentCommand.manualOverride = false;
                sendLogMessage("设置控制模式: " + String(currentCommand.mode));
            } else if (command == "setTarget") {
                currentCommand.target = doc["target"] | 100.0f;
                sendLogMessage("设置目标值: " + String(currentCommand.target));
            } else if (command == "manualControl") {
                currentCommand.manualOverride = true;
                currentCommand.manualOutput = doc["output"] | 50.0f;
                sendLogMessage("手动控制: " + String(currentCommand.manualOutput) + "%");
            } else if (command == "autoControl") {
                currentCommand.manualOverride = false;
                sendLogMessage("切换到自动控制");
            } else if (command == "reset") {
                currentCommand.resetRequested = true;
                sendLogMessage("系统重置请求");
            } else if (command == "calibrate") {
                currentCommand.calibrateRequested = true;
                sendLogMessage("传感器校准请求");
            }
        }
    } else {
        // 简单命令格式
        if (data.startsWith("MODE:")) {
            currentCommand.mode = data.substring(5).toInt();
            currentCommand.manualOverride = false;
            currentCommand.commandType = "setMode";
            sendLogMessage("设置控制模式: " + String(currentCommand.mode));
        } else if (data.startsWith("TARGET:")) {
            currentCommand.target = data.substring(7).toFloat();
            currentCommand.commandType = "setTarget";
            sendLogMessage("设置目标值: " + String(currentCommand.target));
        } else if (data.startsWith("MANUAL:")) {
            currentCommand.manualOverride = true;
            currentCommand.manualOutput = data.substring(7).toFloat();
            currentCommand.commandType = "manualControl";
            sendLogMessage("手动控制: " + String(currentCommand.manualOutput) + "%");
        } else if (data == "AUTO") {
            currentCommand.manualOverride = false;
            currentCommand.commandType = "autoControl";
            sendLogMessage("切换到自动控制");
        } else if (data == "RESET") {
            currentCommand.resetRequested = true;
            currentCommand.commandType = "reset";
            sendLogMessage("系统重置请求");
        } else if (data == "CALIBRATE") {
            currentCommand.calibrateRequested = true;
            currentCommand.commandType = "calibrate";
            sendLogMessage("传感器校准请求");
        }
    }
}

void WiFiComm::sendHeartbeat() {
    if (!connected) return;
    
    StaticJsonDocument<128> doc;
    doc["type"] = "heartbeat";
    doc["timestamp"] = millis();
    doc["status"] = "alive";
    
    String json;
    serializeJson(doc, json);
    
    espSerial->println(json);
}

void WiFiComm::sendSensorData(const SensorData& data) {
    if (!connected) return;
    
    StaticJsonDocument<512> doc;
    doc["type"] = "sensorData";
    doc["timestamp"] = millis();
    doc["flowRate"] = data.flowRate;
    doc["pollutionLevel"] = data.pollutionLevel;
    doc["lightIntensity"] = data.lightIntensity;
    doc["pH"] = data.pH;
    doc["temperature"] = data.temperature;
    doc["energyUsage"] = data.energyUsage;
    doc["systemEfficiency"] = data.systemEfficiency;
    
    String json;
    serializeJson(doc, json);
    
    espSerial->println(json);
    lastDataSend = millis();
}

void WiFiComm::sendControlData(const ControlDecision& decision) {
    if (!connected) return;
    
    StaticJsonDocument<256> doc;
    doc["type"] = "controlData";
    doc["timestamp"] = millis();
    doc["controlOutput"] = decision.controlOutput;
    doc["mode"] = decision.mode;
    doc["reasoning"] = decision.reasoning;
    
    String json;
    serializeJson(doc, json);
    
    espSerial->println(json);
}

void WiFiComm::sendTwinData(const DigitalTwinData& twin) {
    if (!connected) return;
    
    StaticJsonDocument<256> doc;
    doc["type"] = "twinData";
    doc["timestamp"] = millis();
    doc["predictedPollution"] = twin.predictedPollution;
    doc["predictedEfficiency"] = twin.predictedEfficiency;
    doc["remainingLife"] = twin.remainingLife;
    doc["optimalSetpoint"] = twin.optimalSetpoint;
    doc["systemHealth"] = twin.systemHealth;
    doc["performanceTrend"] = twin.performanceTrend;
    
    String json;
    serializeJson(doc, json);
    
    espSerial->println(json);
}

void WiFiComm::sendLogMessage(const String& message, uint8_t level) {
    if (!connected) return;
    
    StaticJsonDocument<192> doc;
    doc["type"] = "log";
    doc["timestamp"] = millis();
    doc["level"] = level;
    doc["message"] = message;
    
    String json;
    serializeJson(doc, json);
    
    espSerial->println(json);
}

// ========== 修改这里 ==========
bool WiFiComm::hasCommand() const {
    return currentCommand.resetRequested || 
           currentCommand.calibrateRequested ||
           (currentCommand.commandType.length() > 0);  // 修改这里
}

WiFiCommand WiFiComm::getCommand() const {
    return currentCommand;
}

void WiFiComm::clearCommand() {
    currentCommand.resetRequested = false;
    currentCommand.calibrateRequested = false;
    currentCommand.commandType = "";
}

bool WiFiComm::isConnected() const {
    return connected;
}

bool WiFiComm::isInitialized() const {
    return initialized;
}

WiFiConfig WiFiComm::getConfig() const {
    return config;
}

void WiFiComm::setConfig(const WiFiConfig& cfg) {
    config = cfg;
    if (initialized) {
        // 重新初始化
        initialized = false;
        if (espSerial != nullptr) {
            delete espSerial;
            espSerial = nullptr;
        }
        initialize(config);
    }
}

void WiFiComm::reset() {
    clearCommand();
    if (espSerial != nullptr) {
        espSerial->println("AT+RST");
    }
    connected = false;
    initialized = false;
}