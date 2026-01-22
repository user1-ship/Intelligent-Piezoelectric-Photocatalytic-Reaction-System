#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include <Arduino.h>
#include <math.h>

class MathUtils {
public:
  // 约束函数
  static float constrainFloat(float value, float minVal, float maxVal) {
    if (value < minVal) return minVal;
    if (value > maxVal) return maxVal;
    return value;
  }
  
  // 映射函数（浮点版本）
  static float mapFloat(float value, float inMin, float inMax, float outMin, float outMax) {
    return (value - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
  }
  
  // 低通滤波器
  static float lowPassFilter(float currentValue, float previousValue, float alpha) {
    return alpha * currentValue + (1.0f - alpha) * previousValue;
  }
  
  // 移动平均
  static float movingAverage(float newValue, float* buffer, size_t size, size_t* index) {
    buffer[(*index)++] = newValue;
    if (*index >= size) {
      *index = 0;
    }
    
    float sum = 0.0f;
    for (size_t i = 0; i < size; i++) {
      sum += buffer[i];
    }
    return sum / size;
  }
  
  // 指数加权移动平均
  static float exponentialMovingAverage(float newValue, float previousEMA, float alpha) {
    return alpha * newValue + (1.0f - alpha) * previousEMA;
  }
  
  // 归一化函数
  static float normalize(float value, float minVal, float maxVal) {
    return constrainFloat((value - minVal) / (maxVal - minVal), 0.0f, 1.0f);
  }
  
  // 去归一化函数
  static float denormalize(float normalizedValue, float minVal, float maxVal) {
    return minVal + normalizedValue * (maxVal - minVal);
  }
  
  // 计算角度差（-180到180度）
  static float angleDifference(float angle1, float angle2) {
    float diff = angle2 - angle1;
    while (diff < -180.0f) diff += 360.0f;
    while (diff > 180.0f) diff -= 360.0f;
    return diff;
  }
  
  // 计算向量的模
  static float magnitude(float x, float y, float z = 0.0f) {
    return sqrt(x * x + y * y + z * z);
  }
  
  // 判断两个浮点数是否近似相等
  static bool approximatelyEqual(float a, float b, float epsilon = 0.0001f) {
    return fabs(a - b) <= epsilon;
  }
};

#endif // MATH_UTILS_H