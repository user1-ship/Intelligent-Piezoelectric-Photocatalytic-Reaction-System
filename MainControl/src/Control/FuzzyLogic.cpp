#include "FuzzyLogic.h"

FuzzyLogicSystem::FuzzyLogicSystem() : rules(nullptr), ruleCount(0) {
  clearMembership();
}

FuzzyLogicSystem::~FuzzyLogicSystem() {
  if (rules != nullptr) {
    delete[] rules;
  }
}

bool FuzzyLogicSystem::initialize(const MembershipParams& params, const FuzzyRule* rules, uint8_t count) {
  setMembershipParams(params);
  setRules(rules, count);
  return true;
}

void FuzzyLogicSystem::setMembershipParams(const MembershipParams& params) {
  membershipParams = params;
}

void FuzzyLogicSystem::setRules(const FuzzyRule* rules, uint8_t count) {
  if (this->rules != nullptr) {
    delete[] this->rules;
  }
  
  if (count > 0 && rules != nullptr) {
    this->rules = new FuzzyRule[count];
    for (uint8_t i = 0; i < count; i++) {
      this->rules[i] = rules[i];
    }
    ruleCount = count;
  } else {
    this->rules = nullptr;
    ruleCount = 0;
  }
}

void FuzzyLogicSystem::calculateMembership(float inputValue) {
  clearMembership();
  
  // 简化实现：使用三角隶属度函数
  // 这里使用一个简化的方法，实际应该根据membershipParams计算
  
  if (inputValue <= 25.0f) {
    membershipValues[FUZZY_VERY_LOW] = 1.0f;
  } else if (inputValue <= 50.0f) {
    membershipValues[FUZZY_VERY_LOW] = (50.0f - inputValue) / 25.0f;
    membershipValues[FUZZY_LOW] = (inputValue - 25.0f) / 25.0f;
  } else if (inputValue <= 75.0f) {
    membershipValues[FUZZY_LOW] = (75.0f - inputValue) / 25.0f;
    membershipValues[FUZZY_MEDIUM] = (inputValue - 50.0f) / 25.0f;
  } else if (inputValue <= 100.0f) {
    membershipValues[FUZZY_MEDIUM] = (100.0f - inputValue) / 25.0f;
    membershipValues[FUZZY_HIGH] = (inputValue - 75.0f) / 25.0f;
  } else {
    membershipValues[FUZZY_HIGH] = 1.0f;
  }
}

float FuzzyLogicSystem::infer(float input1, float input2) {
  // 简化实现：返回输入的平均值
  return (input1 + input2) / 2.0f;
}

float FuzzyLogicSystem::defuzzify(const float* outputValues) const {
  float numerator = 0.0f;
  float denominator = 0.0f;
  
  for (int i = 0; i < 5; i++) {
    numerator += membershipValues[i] * outputValues[i];
    denominator += membershipValues[i];
  }
  
  if (denominator > 0.0f) {
    return numerator / denominator;
  }
  
  return 0.5f; // 默认值
}

const float* FuzzyLogicSystem::getMembershipValues() const {
  return membershipValues;
}

void FuzzyLogicSystem::clearMembership() {
  for (int i = 0; i < 5; i++) {
    membershipValues[i] = 0.0f;
  }
}

float FuzzyLogicSystem::triangleMF(float x, float center, float width) const {
  float left = center - width;
  float right = center + width;
  
  if (x <= left || x >= right) return 0.0f;
  if (x <= center) return (x - left) / (center - left);
  return (right - x) / (right - center);
}

float FuzzyLogicSystem::gaussianMF(float x, float center, float width) const {
  return exp(-pow((x - center) / width, 2) / 2.0f);
}

float FuzzyLogicSystem::trapezoidMF(float x, float a, float b, float c, float d) const {
  if (x <= a || x >= d) return 0.0f;
  if (x >= b && x <= c) return 1.0f;
  if (x > a && x < b) return (x - a) / (b - a);
  return (d - x) / (d - c);
}

float FuzzyLogicSystem::evaluateRule(const FuzzyRule& rule, float input1Membership, float input2Membership) const {
  // 使用最小运算作为"与"操作
  float activation = min(input1Membership, input2Membership);
  return activation * rule.weight;
}