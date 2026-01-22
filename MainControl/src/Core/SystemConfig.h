#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

#include <Arduino.h>

// ========== 系统配置参数 ==========
#define DEBUG_MODE true            // 调试模式开关

// 引脚定义
#define FLOW_SENSOR_PIN A0         // 流量传感器引脚
#define POLLUTION_SENSOR_PIN A1    // 污染物传感器引脚
#define LIGHT_SENSOR_PIN A2        // 光强传感器引脚
#define PH_SENSOR_PIN A3           // pH传感器引脚
#define TEMPERATURE_SENSOR_PIN A4  // 温度传感器引脚
#define SERVO_PIN 9                // 舵机控制引脚
#define BUZZER_PIN 10              // 蜂鸣器报警引脚

// 系统参数
#define TARGET_POLLUTION 100.0     // 目标污染物浓度 (ppm)
#define MAX_ENERGY_USAGE 80.0      // 最大能耗限制 (%)
#define SAMPLING_INTERVAL 1000     // 采样间隔 (ms)
#define LEARNING_INTERVAL 60000    // 学习间隔 (ms)
#define CONTROL_INTERVAL 100       // 控制周期 (ms)

// 缓冲区大小
#define FLOW_BUFFER_SIZE 10
#define POLLUTION_BUFFER_SIZE 10
#define LIGHT_BUFFER_SIZE 5
#define PH_BUFFER_SIZE 5

// 传感器范围
#define FLOW_MIN 0.0
#define FLOW_MAX 100.0
#define POLLUTION_MIN 0.0
#define POLLUTION_MAX 500.0
#define LIGHT_MIN 0.0
#define LIGHT_MAX 1000.0
#define PH_MIN 0.0
#define PH_MAX 14.0
#define TEMP_MIN 0.0
#define TEMP_MAX 100.0

#endif // SYSTEM_CONFIG_H