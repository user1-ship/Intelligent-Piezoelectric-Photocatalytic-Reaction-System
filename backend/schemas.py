from pydantic import BaseModel, EmailStr, Field
from typing import Optional, List, Dict, Any
from datetime import datetime

# 用户相关
class UserBase(BaseModel):
    username: str
    role: Optional[str] = "operator"
    
class UserCreate(UserBase):
    password: str
    
class UserLogin(BaseModel):
    username: str
    password: str
    remember_me: bool = False

class User(UserBase):
    id: int
    is_active: bool
    created_at: datetime
    last_login: Optional[datetime]
    
    class Config:
        from_attributes = True

# 认证相关
class Token(BaseModel):
    access_token: str
    token_type: str
    user_info: dict

class TokenData(BaseModel):
    username: Optional[str] = None

# 传感器数据
class SensorDataBase(BaseModel):
    sensor_type: str
    value: float
    unit: Optional[str] = ""
    data_quality: Optional[int] = 100

class SensorDataCreate(SensorDataBase):
    pass

class SensorData(SensorDataBase):
    id: int
    timestamp: datetime
    
    class Config:
        from_attributes = True

# 控制参数
class ControlParameterBase(BaseModel):
    param_name: str
    param_value: str
    param_type: str
    description: Optional[str] = ""

class ControlParameterCreate(ControlParameterBase):
    pass

class ControlParameter(ControlParameterBase):
    id: int
    updated_at: datetime
    updated_by: Optional[int] = None
    
    class Config:
        from_attributes = True

# 历史数据查询
class HistoryQuery(BaseModel):
    start_time: Optional[datetime] = None
    end_time: Optional[datetime] = None
    data_type: Optional[str] = "all"
    sensor_type: Optional[str] = None

# 历史数据
class HistoryData(BaseModel):
    data_type: str
    value: float
    timestamp: datetime
    metadata: Optional[Dict[str, Any]] = None
    
    class Config:
        from_attributes = True

# 故障日志
class FaultLogBase(BaseModel):
    component: str
    fault_type: str
    description: str
    severity: str = "medium"
    status: str = "active"
    compensation_value: Optional[float] = None

class FaultLog(FaultLogBase):
    id: int
    created_at: datetime
    resolved_at: Optional[datetime] = None
    
    class Config:
        from_attributes = True

# 系统配置
class SystemConfigBase(BaseModel):
    config_key: str
    config_value: str
    category: str
    description: Optional[str] = ""

class SystemConfig(SystemConfigBase):
    id: int
    
    class Config:
        from_attributes = True

# API响应
class ApiResponse(BaseModel):
    status: str
    message: Optional[str] = None
    data: Optional[Any] = None

class SensorResponse(BaseModel):
    flow: Dict[str, Any]
    pollution: Dict[str, Any]
    light: Dict[str, Any]
    ph: Dict[str, Any]
    temperature: Dict[str, Any]
    energy: Dict[str, Any]

# 数字孪生
class DigitalTwinData(BaseModel):
    predicted_pollution: float
    predicted_efficiency: float
    remaining_life: float
    system_health: float
    timestamp: datetime
    
# 强化学习
class RLParameters(BaseModel):
    kp: float
    ki: float
    kd: float
    learning_rate: float
    discount_factor: float
    exploration_rate: float
    batch_size: int
    learning_samples: int