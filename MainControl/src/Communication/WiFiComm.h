#ifndef WIFI_COMM_H
#define WIFI_COMM_H

#include <Arduino.h>
#include "../Core/CommonTypes.h"

// WiFi配置结构体
struct WiFiConfig {
    String ssid;
    String password;
    String hostname;
    bool apMode;           // true: AP模式, false: STA模式
    uint8_t rxPin;
    uint8_t txPin;
    unsigned long baudRate;
    unsigned long heartbeatInterval; // 心跳间隔
};

// WiFi命令结构体
struct WiFiCommand {
    uint8_t mode;           // 控制模式
    float target;           // 目标值
    bool manualOverride;    // 手动覆盖
    float manualOutput;     // 手动输出值
    bool resetRequested;    // 重置请求
    bool calibrateRequested; // 校准请求
    String commandType;     // 命令类型
};

class WiFiComm {
private:
    // 配置
    WiFiConfig config;
    
    // 软件串口（如果使用）
    class SoftwareSerial* espSerial;
    
    // 状态
    bool initialized;
    bool connected;
    unsigned long lastHeartbeat;
    unsigned long lastDataSend;
    
    // 接收缓冲区
    String receiveBuffer;
    
    // 当前命令
    WiFiCommand currentCommand;
    
    // 私有方法
    void processReceivedData(String data);
    void sendHeartbeat();
    void sendSystemData(const SensorData& sensors, const DigitalTwinData& twin, const ControlDecision& decision);
    
public:
    WiFiComm();
    ~WiFiComm();
    
    // 初始化
    bool initialize(const WiFiConfig& cfg);
    
    // 更新（需要在主循环中调用）
    void update();
    
    // 发送数据
    void sendSensorData(const SensorData& data);
    void sendControlData(const ControlDecision& decision);
    void sendTwinData(const DigitalTwinData& twin);
    void sendLogMessage(const String& message, uint8_t level = 2);
    
    // 接收命令
    bool hasCommand() const;
    WiFiCommand getCommand() const;
    void clearCommand();
    
    // 连接状态
    bool isConnected() const;
    bool isInitialized() const;
    
    // 配置管理
    WiFiConfig getConfig() const;
    void setConfig(const WiFiConfig& cfg);
    
    // 重置
    void reset();
};

#endif // WIFI_COMM_H