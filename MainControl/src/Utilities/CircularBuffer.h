#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

template<typename T, size_t N>
class CircularBuffer {
private:
  T buffer[N];
  size_t head;
  size_t tail;
  size_t count;
  size_t capacity;
  
public:
  CircularBuffer() : head(0), tail(0), count(0), capacity(N) {}
  
  // 添加数据
  bool push(const T& value) {
    if (isFull()) {
      return false;
    }
    buffer[head] = value;
    head = (head + 1) % capacity;
    count++;
    return true;
  }
  
  // 弹出数据
  bool pop(T& value) {
    if (isEmpty()) {
      return false;
    }
    value = buffer[tail];
    tail = (tail + 1) % capacity;
    count--;
    return true;
  }
  
  // 查看数据（不弹出）
  bool peek(T& value) const {
    if (isEmpty()) {
      return false;
    }
    value = buffer[tail];
    return true;
  }
  
  // 获取指定位置的数据
  bool get(size_t index, T& value) const {
    if (index >= count) {
      return false;
    }
    size_t actualIndex = (tail + index) % capacity;
    value = buffer[actualIndex];
    return true;
  }
  
  // 更新指定位置的数据
  bool set(size_t index, const T& value) {
    if (index >= count) {
      return false;
    }
    size_t actualIndex = (tail + index) % capacity;
    buffer[actualIndex] = value;
    return true;
  }
  
  // 缓冲区状态
  bool isEmpty() const { return count == 0; }
  bool isFull() const { return count == capacity; }
  size_t size() const { return count; }
  size_t getCapacity() const { return capacity; }
  
  // 清空缓冲区
  void clear() {
    head = 0;
    tail = 0;
    count = 0;
  }
  
  // 获取平均值
  float getAverage() const {
    if (isEmpty()) {
      return 0.0f;
    }
    
    float sum = 0.0f;
    for (size_t i = 0; i < count; i++) {
      size_t index = (tail + i) % capacity;
      sum += buffer[index];
    }
    return sum / count;
  }
  
  // 获取标准差
  float getStandardDeviation() const {
    if (count < 2) {
      return 0.0f;
    }
    
    float mean = getAverage();
    float sumSquaredDiff = 0.0f;
    
    for (size_t i = 0; i < count; i++) {
      size_t index = (tail + i) % capacity;
      float diff = buffer[index] - mean;
      sumSquaredDiff += diff * diff;
    }
    
    return sqrt(sumSquaredDiff / (count - 1));
  }
};

#endif // CIRCULAR_BUFFER_H