import random
from datetime import datetime, timedelta
from typing import Dict, Any, List
import json
from config import settings

# 传感器数据模拟生成器
class SensorDataSimulator:
    def __init__(self):
        self.sensor_ranges = settings.SIMULATION_RANGES
        self.last_values = {}
        
    def generate_sensor_data(self, sensor_type: str) -> Dict[str, Any]:
        if sensor_type not in self.sensor_ranges:
            raise ValueError(f"未知的传感器类型: {sensor_type}")
        
        config = self.sensor_ranges[sensor_type]
        min_val = config["min"]
        max_val = config["max"]
        unit = config["unit"]
        
        # 如果存在上次的值，基于上次的值进行小范围波动
        if sensor_type in self.last_values:
            last_value = self.last_values[sensor_type]
            # 随机波动范围为±5%
            fluctuation = random.uniform(-0.05, 0.05)
            new_value = last_value * (1 + fluctuation)
            # 确保在合理范围内
            new_value = max(min_val, min(max_val, new_value))
        else:
            new_value = random.uniform(min_val, max_val)
        
        # 添加一些趋势性变化
        if sensor_type == "pollution":
            # 污染物浓度呈下降趋势
            trend = random.uniform(-0.01, 0.005)
            new_value *= (1 + trend)
        elif sensor_type == "efficiency":
            # 效率呈上升趋势
            trend = random.uniform(-0.005, 0.01)
            new_value *= (1 + trend)
        
        # 生成数据质量（80-100%）
        data_quality = random.randint(80, 100)
        
        # 偶尔生成异常数据
        if random.random() < 0.05:  # 5%概率
            data_quality = random.randint(50, 80)
            if random.random() < 0.3:  # 30%概率的异常值
                new_value = random.uniform(min_val * 0.5, min_val)
        
        self.last_values[sensor_type] = new_value
        
        return {
            "sensor_type": sensor_type,
            "value": round(new_value, 2),
            "unit": unit,
            "data_quality": data_quality
        }
    
    def generate_all_sensor_data(self) -> Dict[str, Any]:
        sensors = ["flow", "pollution", "light", "ph", "temperature", "energy"]
        data = {}
        for sensor in sensors:
            sensor_data = self.generate_sensor_data(sensor)
            data[sensor] = sensor_data
        return data

# 数字孪生预测器
class DigitalTwinPredictor:
    @staticmethod
    def predict(sensor_data: Dict[str, Any]) -> Dict[str, Any]:
        # 基于当前传感器数据预测未来状态
        pollution = sensor_data.get("pollution", {}).get("value", 150)
        efficiency = sensor_data.get("energy", {}).get("value", 70)
        
        # 简单的预测模型
        predicted_pollution = pollution * 0.97  # 预测污染物下降3%
        predicted_efficiency = efficiency * 1.02  # 预测效率提高2%
        
        # 系统健康度计算
        data_qualities = [data.get("data_quality", 100) for data in sensor_data.values()]
        avg_quality = sum(data_qualities) / len(data_qualities)
        system_health = avg_quality * 0.95  # 健康度为平均数据质量的95%
        
        # 剩余寿命（基于运行时间）
        remaining_life = random.uniform(85, 95)
        
        return {
            "predicted_pollution": round(predicted_pollution, 1),
            "predicted_efficiency": round(predicted_efficiency, 1),
            "remaining_life": round(remaining_life, 1),
            "system_health": round(system_health, 1),
            "timestamp": datetime.now().isoformat()
        }

# 强化学习参数优化器
class RLOptimizer:
    def __init__(self):
        self.learning_samples = 0
        self.best_params = {
            "kp": 2.85,
            "ki": 0.42,
            "kd": 0.18,
            "learning_rate": 0.01,
            "discount_factor": 0.95,
            "exploration_rate": 0.085
        }
    
    def optimize(self, sensor_data: Dict[str, Any]) -> Dict[str, Any]:
        self.learning_samples += 1
        
        # 模拟强化学习优化过程
        if self.learning_samples % 100 == 0:
            # 每100个样本调整一次参数
            self.best_params["kp"] += random.uniform(-0.05, 0.05)
            self.best_params["ki"] += random.uniform(-0.01, 0.01)
            self.best_params["kd"] += random.uniform(-0.005, 0.005)
            
            # 确保参数在合理范围内
            self.best_params["kp"] = max(0.5, min(5.0, self.best_params["kp"]))
            self.best_params["ki"] = max(0.1, min(2.0, self.best_params["ki"]))
            self.best_params["kd"] = max(0.05, min(1.0, self.best_params["kd"]))
        
        return {
            **self.best_params,
            "learning_samples": self.learning_samples,
            "batch_size": 32,
            "optimal_condition_params": random.randint(10, 20)
        }

# 故障诊断器
class FaultDiagnoser:
    @staticmethod
    def diagnose(sensor_data: Dict[str, Any]) -> List[Dict[str, Any]]:
        faults = []
        
        # 检查每个传感器的数据质量
        for sensor_type, data in sensor_data.items():
            data_quality = data.get("data_quality", 100)
            value = data.get("value", 0)
            
            if data_quality < 80:
                severity = "high" if data_quality < 70 else "medium"
                faults.append({
                    "component": f"{sensor_type}_sensor",
                    "fault_type": "warning",
                    "description": f"{sensor_type}传感器数据质量低 ({data_quality}%)",
                    "severity": severity,
                    "status": "active",
                    "compensation_value": value * 0.95 if data_quality < 70 else None
                })
        
        return faults

# 数据导出工具
class DataExporter:
    @staticmethod
    def export_to_csv(data: List[Dict[str, Any]]) -> str:
        if not data:
            return ""
        
        # 获取表头
        headers = list(data[0].keys())
        
        # 生成CSV内容
        csv_lines = [",".join(headers)]
        for row in data:
            values = [str(row.get(header, "")) for header in headers]
            csv_lines.append(",".join(values))
        
        return "\n".join(csv_lines)
    
    @staticmethod
    def export_to_json(data: List[Dict[str, Any]]) -> str:
        return json.dumps(data, ensure_ascii=False, indent=2, default=str)

# 系统状态监控
class SystemMonitor:
    @staticmethod
    def get_system_status() -> Dict[str, Any]:
        return {
            "system_name": settings.PROJECT_NAME,
            "version": settings.VERSION,
            "uptime": f"{random.randint(300, 400)}小时{random.randint(0, 59)}分钟",
            "data_server": "online",
            "control_server": "online",
            "sensor_network": "online",
            "database_status": "healthy",
            "last_backup": (datetime.now() - timedelta(days=1)).strftime("%Y-%m-%d %H:%M:%S"),
            "storage_usage": f"{random.randint(50, 80)}%"
        }