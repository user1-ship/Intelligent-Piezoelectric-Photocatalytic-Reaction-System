# backend/crud.py
from sqlalchemy.orm import Session
from sqlalchemy import and_, or_, desc
from datetime import datetime, timedelta
from typing import List, Optional, Dict, Any
import json

from models import User, SensorData, ControlParameter, SystemConfig, HistoryData, FaultLog
from schemas import (
    UserCreate, SensorDataCreate, ControlParameterCreate, 
    SystemConfigBase, FaultLogBase, HistoryQuery
)
from security import get_password_hash, verify_password  # 从 security 导入

# 用户操作
def get_user_by_username(db: Session, username: str):
    return db.query(User).filter(User.username == username).first()

def create_user(db: Session, user: UserCreate):
    hashed_password = get_password_hash(user.password)
    db_user = User(
        username=user.username,
        password_hash=hashed_password,
        role=user.role
    )
    db.add(db_user)
    db.commit()
    db.refresh(db_user)
    return db_user

def authenticate_user(db: Session, username: str, password: str):
    user = get_user_by_username(db, username)
    if not user:
        return False
    if not verify_password(password, user.password_hash):
        return False
    return user

def update_user_last_login(db: Session, user_id: int):
    user = db.query(User).filter(User.id == user_id).first()
    if user:
        user.last_login = datetime.now()
        db.commit()
        db.refresh(user)
    return user

# 传感器数据操作
def create_sensor_data(db: Session, sensor_data: SensorDataCreate):
    db_sensor_data = SensorData(**sensor_data.dict())
    db.add(db_sensor_data)
    db.commit()
    db.refresh(db_sensor_data)
    return db_sensor_data

def get_latest_sensor_data(db: Session, sensor_type: Optional[str] = None):
    if sensor_type:
        return db.query(SensorData).filter(
            SensorData.sensor_type == sensor_type
        ).order_by(desc(SensorData.timestamp)).first()
    else:
        # 获取所有传感器的最新数据
        sensors = ["flow", "pollution", "light", "ph", "temperature", "energy"]
        latest_data = {}
        for sensor in sensors:
            data = db.query(SensorData).filter(
                SensorData.sensor_type == sensor
            ).order_by(desc(SensorData.timestamp)).first()
            if data:
                latest_data[sensor] = data
        return latest_data

def get_sensor_data_history(
    db: Session, 
    start_time: datetime, 
    end_time: datetime,
    sensor_type: Optional[str] = None
):
    query = db.query(SensorData).filter(
        SensorData.timestamp >= start_time,
        SensorData.timestamp <= end_time
    )
    
    if sensor_type and sensor_type != "all":
        query = query.filter(SensorData.sensor_type == sensor_type)
    
    return query.order_by(SensorData.timestamp).all()

# 控制参数操作
def get_control_parameter(db: Session, param_name: str):
    return db.query(ControlParameter).filter(
        ControlParameter.param_name == param_name
    ).first()

def get_all_control_params(db: Session):
    return db.query(ControlParameter).all()

def create_or_update_control_param(db: Session, param: ControlParameterCreate, user_id: int):
    existing_param = get_control_parameter(db, param.param_name)
    
    if existing_param:
        existing_param.param_value = param.param_value
        existing_param.updated_at = datetime.now()
        existing_param.updated_by = user_id
    else:
        existing_param = ControlParameter(
            **param.dict(),
            updated_by=user_id
        )
        db.add(existing_param)
    
    db.commit()
    db.refresh(existing_param)
    return existing_param

def get_pid_params(db: Session):
    return {
        "kp": get_control_parameter(db, "pid_kp"),
        "ki": get_control_parameter(db, "pid_ki"),
        "kd": get_control_parameter(db, "pid_kd")
    }

def get_mpc_params(db: Session):
    return {
        "prediction_horizon": get_control_parameter(db, "mpc_prediction_horizon"),
        "control_horizon": get_control_parameter(db, "mpc_control_horizon"),
        "weights": get_control_parameter(db, "mpc_weights")
    }

# 系统配置操作
def get_system_config(db: Session, config_key: str):
    return db.query(SystemConfig).filter(
        SystemConfig.config_key == config_key
    ).first()

def get_all_system_configs(db: Session, category: Optional[str] = None):
    query = db.query(SystemConfig)
    if category:
        query = query.filter(SystemConfig.category == category)
    return query.all()

def create_or_update_system_config(db: Session, config: SystemConfigBase):
    existing_config = get_system_config(db, config.config_key)
    
    if existing_config:
        existing_config.config_value = config.config_value
        existing_config.category = config.category
        existing_config.description = config.description
    else:
        existing_config = SystemConfig(**config.dict())
        db.add(existing_config)
    
    db.commit()
    db.refresh(existing_config)
    return existing_config

# 故障日志操作
def create_fault_log(db: Session, fault_log: FaultLogBase):
    db_fault_log = FaultLog(**fault_log.dict())
    db.add(db_fault_log)
    db.commit()
    db.refresh(db_fault_log)
    return db_fault_log

def get_active_faults(db: Session):
    return db.query(FaultLog).filter(
        FaultLog.status == "active"
    ).order_by(desc(FaultLog.created_at)).all()

def resolve_fault(db: Session, fault_id: int):
    fault = db.query(FaultLog).filter(FaultLog.id == fault_id).first()
    if fault:
        fault.status = "resolved"
        fault.resolved_at = datetime.now()
        db.commit()
        db.refresh(fault)
    return fault

# 历史数据操作
def create_history_data(db: Session, data_type: str, value: float, metadata: Optional[Dict] = None):
    db_history = HistoryData(
        data_type=data_type,
        value=value,
        timestamp=datetime.now(),
        meta_info=json.dumps(metadata) if metadata else None  # 注意：字段名已改为 meta_info
    )
    db.add(db_history)
    db.commit()
    db.refresh(db_history)
    return db_history

def get_history_data(db: Session, query: HistoryQuery):
    q = db.query(HistoryData)
    
    if query.start_time:
        q = q.filter(HistoryData.timestamp >= query.start_time)
    if query.end_time:
        q = q.filter(HistoryData.timestamp <= query.end_time)
    if query.data_type and query.data_type != "all":
        q = q.filter(HistoryData.data_type == query.data_type)
    
    return q.order_by(HistoryData.timestamp).all()