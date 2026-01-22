from sqlalchemy import Boolean, Column, Float, Integer, String, DateTime, Text, ForeignKey
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.sql import func
from database import Base
import datetime

Base = declarative_base()

class User(Base):
    __tablename__ = "users"
    
    id = Column(Integer, primary_key=True, index=True)
    username = Column(String(50), unique=True, index=True, nullable=False)
    password_hash = Column(String(255), nullable=False)
    role = Column(String(20), default="operator")  # admin, operator, viewer
    is_active = Column(Boolean, default=True)
    created_at = Column(DateTime, default=func.now())
    last_login = Column(DateTime, nullable=True)
    
class SensorData(Base):
    __tablename__ = "sensor_data"
    
    id = Column(Integer, primary_key=True, index=True)
    sensor_type = Column(String(50), index=True, nullable=False)
    value = Column(Float, nullable=False)
    unit = Column(String(20))
    data_quality = Column(Integer, default=100)  # 数据质量百分比
    timestamp = Column(DateTime, default=func.now(), index=True)
    
class ControlParameter(Base):
    __tablename__ = "control_parameters"
    
    id = Column(Integer, primary_key=True, index=True)
    param_name = Column(String(100), unique=True, index=True)
    param_value = Column(String(255))
    param_type = Column(String(50))  # pid, mpc, algorithm, system
    description = Column(Text)
    updated_at = Column(DateTime, default=func.now())
    updated_by = Column(Integer, ForeignKey("users.id"))
    
class SystemConfig(Base):
    __tablename__ = "system_config"
    
    id = Column(Integer, primary_key=True, index=True)
    config_key = Column(String(100), unique=True, index=True)
    config_value = Column(Text)
    category = Column(String(50))  # system, sensor, control, algorithm, communication, security, backup
    description = Column(Text)
    
class HistoryData(Base):
    __tablename__ = "history_data"
    
    id = Column(Integer, primary_key=True, index=True)
    data_type = Column(String(50), index=True)
    value = Column(Float)
    timestamp = Column(DateTime, index=True)
    meta_info = Column(Text)  # 将 metadata 改为 meta_info，避免与 SQLAlchemy 保留字冲突
    
class FaultLog(Base):
    __tablename__ = "fault_logs"
    
    id = Column(Integer, primary_key=True, index=True)
    component = Column(String(100))
    fault_type = Column(String(50))  # error, warning, info
    description = Column(Text)
    severity = Column(String(20))  # high, medium, low
    status = Column(String(20))  # active, resolved, investigating
    compensation_value = Column(Float, nullable=True)
    created_at = Column(DateTime, default=func.now())
    resolved_at = Column(DateTime, nullable=True)