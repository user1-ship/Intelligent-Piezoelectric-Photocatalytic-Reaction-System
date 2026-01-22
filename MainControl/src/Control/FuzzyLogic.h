#ifndef FUZZY_LOGIC_H
#define FUZZY_LOGIC_H

#include <Arduino.h>
#include "../Core/CommonTypes.h"

class FuzzyLogicSystem {
private:
  // 隶属度函数参数
  struct MembershipParams {
    float veryLow[2];    // [中心, 宽度]
    float low[2];
    float medium[2];
    float high[2];
    float veryHigh[2];
  };
  
  // 模糊规则
  struct FuzzyRule {
    uint8_t input1Level;
    uint8_t input2Level;
    uint8_t outputLevel;
    float weight;
  };
  
  MembershipParams membershipParams;
  FuzzyRule* rules;
  uint8_t ruleCount;
  
  // 隶属度值
  float membershipValues[5];
  
public:
  FuzzyLogicSystem();
  ~FuzzyLogicSystem();
  
  // 初始化模糊系统
  bool initialize(const MembershipParams& params, const FuzzyRule* rules, uint8_t count);
  
  // 设置隶属度函数参数
  void setMembershipParams(const MembershipParams& params);
  
  // 设置模糊规则
  void setRules(const FuzzyRule* rules, uint8_t count);
  
  // 计算隶属度
  void calculateMembership(float inputValue);
  
  // 模糊推理
  float infer(float input1, float input2);
  
  // 去模糊化（重心法）
  float defuzzify(const float* outputValues) const;
  
  // 获取隶属度值
  const float* getMembershipValues() const;
  
  // 清除隶属度值
  void clearMembership();
  
private:
  // 计算三角隶属度函数
  float triangleMF(float x, float center, float width) const;
  
  // 计算高斯隶属度函数
  float gaussianMF(float x, float center, float width) const;
  
  // 计算梯形隶属度函数
  float trapezoidMF(float x, float a, float b, float c, float d) const;
  
  // 规则评估
  float evaluateRule(const FuzzyRule& rule, float input1Membership, float input2Membership) const;
};

#endif // FUZZY_LOGIC_H