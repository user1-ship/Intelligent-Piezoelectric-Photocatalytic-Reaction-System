import os
from pydantic_settings import BaseSettings
from typing import Optional

class Settings(BaseSettings):
    # API配置
    API_V1_STR: str = "/api"
    PROJECT_NAME: str = "智能压电光催化控制平台"
    VERSION: str = "v1.0.0"
    
    # 安全配置
    SECRET_KEY: str = "your-secret-key-for-jwt-token-generation"
    ALGORITHM: str = "HS256"
    ACCESS_TOKEN_EXPIRE_MINUTES: int = 30
    REMEMBER_ME_TOKEN_EXPIRE_DAYS: int = 7
    
    # 数据库配置
    DATABASE_URL: str = "sqlite:///./sensor_data.db"
    
    # 传感器配置
    SENSOR_UPDATE_INTERVAL: int = 5  # 秒
    
    # 文件存储
    DATA_STORAGE_PATH: str = "./data"
    BACKUP_PATH: str = "./backups"
    
    # 模拟数据配置
    SIMULATE_SENSOR_DATA: bool = True
    SIMULATION_RANGES: dict = {
        "flow": {"min": 20, "max": 35, "unit": "cm/s"},
        "pollution": {"min": 140, "max": 170, "unit": "ppm"},
        "light": {"min": 800, "max": 950, "unit": "lux"},
        "ph": {"min": 6.5, "max": 7.5, "unit": ""},
        "temperature": {"min": 24, "max": 27, "unit": "°C"},
        "energy": {"min": 60, "max": 75, "unit": "%"}
    }
    
    class Config:
        env_file = ".env"

settings = Settings()