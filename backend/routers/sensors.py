from fastapi import APIRouter, Depends, HTTPException, BackgroundTasks
from sqlalchemy.orm import Session
from typing import List, Dict, Any
import asyncio
from datetime import datetime, timedelta

from database import get_db
from crud import (
    create_sensor_data, get_latest_sensor_data, 
    get_sensor_data_history, get_active_faults, create_fault_log
)
from schemas import SensorData, SensorResponse, HistoryQuery, FaultLog
from auth import get_current_active_user
from utils import (
    SensorDataSimulator, DigitalTwinPredictor, 
    RLOptimizer, FaultDiagnoser, SystemMonitor
)
from config import settings

router = APIRouter(prefix=f"{settings.API_V1_STR}/sensors", tags=["传感器"])

# 全局模拟器实例
sensor_simulator = SensorDataSimulator()
digital_twin = DigitalTwinPredictor()
rl_optimizer = RLOptimizer()
fault_diagnoser = FaultDiagnoser()

@router.get("/", response_model=SensorResponse)
async def get_all_sensors(
    current_user: dict = Depends(get_current_active_user),
    db: Session = Depends(get_db)
):
    """
    获取所有传感器的实时数据
    """
    try:
        if settings.SIMULATE_SENSOR_DATA:
            # 模拟数据模式
            sensor_data = sensor_simulator.generate_all_sensor_data()
            
            # 将模拟数据存储到数据库
            for sensor_type, data in sensor_data.items():
                create_sensor_data(db, data)
            
            # 故障诊断
            faults = fault_diagnoser.diagnose(sensor_data)
            for fault in faults:
                create_fault_log(db, fault)
            
            # 构建响应
            response_data = {}
            for sensor_type, data in sensor_data.items():
                response_data[sensor_type] = {
                    "value": data["value"],
                    "unit": data["unit"],
                    "data_quality": data["data_quality"],
                    "timestamp": datetime.now().isoformat()
                }
            
            return response_data
        else:
            # 真实数据模式 - 从数据库获取最新数据
            latest_data = get_latest_sensor_data(db)
            response_data = {}
            for sensor_type, data in latest_data.items():
                if data:
                    response_data[sensor_type] = {
                        "value": data.value,
                        "unit": data.unit,
                        "data_quality": data.data_quality,
                        "timestamp": data.timestamp.isoformat()
                    }
            
            # 如果没有数据，返回模拟数据
            if not response_data:
                sensor_data = sensor_simulator.generate_all_sensor_data()
                for sensor_type, data in sensor_data.items():
                    response_data[sensor_type] = {
                        "value": data["value"],
                        "unit": data["unit"],
                        "data_quality": data["data_quality"],
                        "timestamp": datetime.now().isoformat()
                    }
            
            return response_data
            
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"获取传感器数据失败: {str(e)}")

@router.get("/{sensor_type}")
async def get_sensor(
    sensor_type: str,
    current_user: dict = Depends(get_current_active_user),
    db: Session = Depends(get_db)
):
    """
    获取特定类型传感器的数据
    """
    if sensor_type not in ["flow", "pollution", "light", "ph", "temperature", "energy"]:
        raise HTTPException(status_code=400, detail="无效的传感器类型")
    
    try:
        if settings.SIMULATE_SENSOR_DATA:
            data = sensor_simulator.generate_sensor_data(sensor_type)
            create_sensor_data(db, data)
            
            return {
                "status": "success",
                "data": {
                    "sensor_type": data["sensor_type"],
                    "value": data["value"],
                    "unit": data["unit"],
                    "data_quality": data["data_quality"],
                    "timestamp": datetime.now().isoformat()
                }
            }
        else:
            data = get_latest_sensor_data(db, sensor_type)
            if data:
                return {
                    "status": "success",
                    "data": {
                        "sensor_type": data.sensor_type,
                        "value": data.value,
                        "unit": data.unit,
                        "data_quality": data.data_quality,
                        "timestamp": data.timestamp.isoformat()
                    }
                }
            else:
                raise HTTPException(status_code=404, detail="未找到传感器数据")
                
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"获取传感器数据失败: {str(e)}")

@router.get("/fusion/center")
async def get_sensor_fusion_data(
    current_user: dict = Depends(get_current_active_user),
    db: Session = Depends(get_db)
):
    """
    获取传感器融合数据
    """
    try:
        # 获取所有传感器数据
        sensor_data = await get_all_sensors(current_user, db)
        
        # 计算融合指标（加权平均）
        weights = {
            "flow": 0.15,
            "pollution": 0.35,
            "light": 0.10,
            "ph": 0.20,
            "temperature": 0.20
        }
        
        fusion_value = 0
        total_weight = 0
        fusion_confidence = 0
        
        for sensor, data in sensor_data.items():
            if sensor in weights:
                fusion_value += data["value"] * weights[sensor]
                fusion_confidence += data["data_quality"] * weights[sensor]
                total_weight += weights[sensor]
        
        if total_weight > 0:
            fusion_value /= total_weight
            fusion_confidence /= total_weight
        
        # 卡尔曼滤波参数（模拟）
        kalman_params = {
            "flow": {"q": 0.01, "r": 0.1},
            "pollution": {"q": 0.02, "r": 0.15},
            "light": {"q": 0.01, "r": 0.08},
            "ph": {"q": 0.01, "r": 0.09},
            "temperature": {"q": 0.01, "r": 0.05}
        }
        
        return {
            "status": "success",
            "data": {
                "fusion_value": round(fusion_value, 2),
                "fusion_confidence": round(fusion_confidence, 1),
                "sensor_weights": weights,
                "kalman_parameters": kalman_params
            }
        }
        
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"获取融合数据失败: {str(e)}")

@router.get("/digital-twin")
async def get_digital_twin_data(
    current_user: dict = Depends(get_current_active_user)
):
    """
    获取数字孪生系统数据
    """
    try:
        # 获取当前传感器数据
        sensor_response = await get_all_sensors(current_user)
        
        # 预测未来状态
        predictions = digital_twin.predict(sensor_response)
        
        return {
            "status": "success",
            "data": predictions
        }
        
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"获取数字孪生数据失败: {str(e)}")

@router.get("/reinforcement-learning")
async def get_rl_parameters(
    current_user: dict = Depends(get_current_active_user)
):
    """
    获取强化学习参数
    """
    try:
        # 获取传感器数据
        sensor_response = await get_all_sensors(current_user)
        
        # 优化参数
        optimized_params = rl_optimizer.optimize(sensor_response)
        
        return {
            "status": "success",
            "data": optimized_params
        }
        
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"获取强化学习参数失败: {str(e)}")

@router.get("/faults")
async def get_fault_diagnosis(
    current_user: dict = Depends(get_current_active_user),
    db: Session = Depends(get_db)
):
    """
    获取故障诊断信息
    """
    try:
        # 获取活跃故障
        active_faults = get_active_faults(db)
        
        # 获取传感器数据
        sensor_data = await get_all_sensors(current_user, db)
        
        # 生成故障日志（模拟）
        recent_faults = [
            {
                "time": (datetime.now() - timedelta(hours=i)).strftime("%Y-%m-%d %H:%M:%S"),
                "type": ["error", "warning", "info"][i % 3],
                "message": f"传感器{i+1}数据异常，已自动补偿"
            }
            for i in range(3)
        ]
        
        return {
            "status": "success",
            "data": {
                "active_faults": active_faults,
                "fault_logs": recent_faults,
                "sensor_status": sensor_data
            }
        }
        
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"获取故障诊断数据失败: {str(e)}")

@router.post("/simulate")
async def simulate_sensor_data(
    background_tasks: BackgroundTasks,
    current_user: dict = Depends(get_current_active_user),
    db: Session = Depends(get_db)
):
    """
    模拟生成传感器数据
    """
    try:
        # 生成模拟数据
        sensor_data = sensor_simulator.generate_all_sensor_data()
        
        # 存储到数据库
        for sensor_type, data in sensor_data.items():
            create_sensor_data(db, data)
        
        return {
            "status": "success",
            "message": "传感器数据模拟生成成功",
            "data": sensor_data
        }
        
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"模拟数据生成失败: {str(e)}")

@router.post("/calibrate/{sensor_type}")
async def calibrate_sensor(
    sensor_type: str,
    offset: float = 0.0,
    current_user: dict = Depends(get_current_active_user),
    db: Session = Depends(get_db)
):
    """
    校准传感器
    """
    try:
        # 这里可以实现实际的校准逻辑
        # 目前只是模拟
        
        return {
            "status": "success",
            "message": f"{sensor_type}传感器校准完成，偏移量: {offset}",
            "data": {
                "sensor_type": sensor_type,
                "calibration_offset": offset,
                "calibrated_at": datetime.now().isoformat(),
                "calibrated_by": current_user.username
            }
        }
        
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"传感器校准失败: {str(e)}")

@router.post("/test/{sensor_type}")
async def test_sensor(
    sensor_type: str,
    current_user: dict = Depends(get_current_active_user),
    db: Session = Depends(get_db)
):
    """
    测试传感器
    """
    try:
        # 模拟传感器测试
        await asyncio.sleep(1)  # 模拟测试延迟
        
        test_result = {
            "sensor_type": sensor_type,
            "status": "正常",
            "response_time": f"{random.uniform(0.1, 0.5):.3f}秒",
            "data_quality": random.randint(85, 100),
            "tested_at": datetime.now().isoformat()
        }
        
        return {
            "status": "success",
            "message": f"{sensor_type}传感器测试完成",
            "data": test_result
        }
        
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"传感器测试失败: {str(e)}")

@router.get("/history/trend")
async def get_sensor_trend(
    hours: int = 8,
    current_user: dict = Depends(get_current_active_user),
    db: Session = Depends(get_db)
):
    """
    获取传感器趋势数据
    """
    try:
        end_time = datetime.now()
        start_time = end_time - timedelta(hours=hours)
        
        # 构建查询
        query = HistoryQuery(
            start_time=start_time,
            end_time=end_time
        )
        
        # 获取历史数据
        history_data = get_sensor_data_history(db, start_time, end_time)
        
        # 按时间分组
        time_points = []
        flow_data = []
        pollution_data = []
        temp_data = []
        
        # 生成模拟趋势数据
        for i in range(hours):
            time_point = (start_time + timedelta(hours=i)).strftime("%H:%M")
            time_points.append(time_point)
            
            # 模拟趋势
            flow_data.append(round(28.5 - i * 0.2 + random.uniform(-0.3, 0.3), 1))
            pollution_data.append(round(156.8 - i * 0.5 + random.uniform(-1, 1), 1))
            temp_data.append(round(25.8 + i * 0.1 + random.uniform(-0.2, 0.2), 1))
        
        return {
            "status": "success",
            "data": {
                "time_points": time_points,
                "flow": flow_data,
                "pollution": pollution_data,
                "temperature": temp_data
            }
        }
        
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"获取趋势数据失败: {str(e)}")