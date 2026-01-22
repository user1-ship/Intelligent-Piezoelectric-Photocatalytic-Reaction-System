from fastapi import APIRouter, Depends, HTTPException, File, UploadFile
from sqlalchemy.orm import Session
from datetime import datetime
import os
import json
import shutil

from database import get_db
from crud import (
    get_all_system_configs, create_or_update_system_config,
    get_system_config, create_fault_log
)
from schemas import SystemConfig, SystemConfigBase, FaultLogBase
from auth import get_current_active_user, check_admin_permission
from utils import SystemMonitor
from config import settings

router = APIRouter(prefix=f"{settings.API_V1_STR}/config", tags=["系统配置"])

@router.get("/system")
async def get_system_configuration(
    current_user: dict = Depends(get_current_active_user),
    db: Session = Depends(get_db)
):
    """
    获取系统配置
    """
    try:
        # 获取所有系统配置
        configs = get_all_system_configs(db)
        
        # 如果没有配置，创建默认配置
        if not configs:
            default_configs = [
                SystemConfigBase(
                    config_key="system_name",
                    config_value="智能压电光催化控制平台",
                    category="system",
                    description="系统名称"
                ),
                SystemConfigBase(
                    config_key="version",
                    config_value="v1.0.0",
                    category="system",
                    description="系统版本"
                ),
                SystemConfigBase(
                    config_key="startup_time",
                    config_value=datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
                    category="system",
                    description="启动时间"
                ),
                SystemConfigBase(
                    config_key="data_storage_path",
                    config_value="/data/sensor",
                    category="system",
                    description="数据存储路径"
                ),
                SystemConfigBase(
                    config_key="data_retention_days",
                    config_value="90",
                    category="system",
                    description="数据保留天数"
                ),
                SystemConfigBase(
                    config_key="timezone",
                    config_value="(UTC+08:00) 北京，上海",
                    category="system",
                    description="时区设置"
                ),
                SystemConfigBase(
                    config_key="ntp_server",
                    config_value="time.windows.com",
                    category="system",
                    description="NTP服务器"
                )
            ]
            
            for config in default_configs:
                create_or_update_system_config(db, config)
            
            configs = get_all_system_configs(db)
        
        # 按类别分组
        config_dict = {}
        for config in configs:
            if config.category not in config_dict:
                config_dict[config.category] = {}
            config_dict[config.category][config.config_key] = config.config_value
        
        # 获取系统状态
        system_status = SystemMonitor.get_system_status()
        
        return {
            "status": "success",
            "data": {
                "configurations": config_dict,
                "system_status": system_status
            }
        }
        
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"获取系统配置失败: {str(e)}")

@router.post("/system")
async def update_system_configuration(
    configs: dict,
    current_user: dict = Depends(get_current_active_user),
    db: Session = Depends(get_db)
):
    """
    更新系统配置
    """
    check_admin_permission(current_user)
    
    try:
        updated_configs = []
        
        for category, config_dict in configs.items():
            for key, value in config_dict.items():
                config_data = SystemConfigBase(
                    config_key=key,
                    config_value=str(value),
                    category=category,
                    description=f"{category}配置"
                )
                
                updated_config = create_or_update_system_config(db, config_data)
                updated_configs.append(updated_config)
        
        return {
            "status": "success",
            "message": "系统配置更新成功",
            "data": {
                "updated_count": len(updated_configs),
                "updated_by": current_user.username,
                "updated_at": datetime.now().isoformat()
            }
        }
        
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"更新系统配置失败: {str(e)}")

@router.get("/sensors")
async def get_sensor_configuration(
    current_user: dict = Depends(get_current_active_user),
    db: Session = Depends(get_db)
):
    """
    获取传感器配置
    """
    try:
        # 模拟传感器配置数据
        sensor_configs = [
            {
                "name": "流量传感器-01",
                "type": "流量传感器",
                "address": "0x01",
                "sampling_frequency": "5Hz",
                "range": "0-100 cm/s",
                "calibration_status": "已校准",
                "calibration_offset": 0.0
            },
            {
                "name": "污染物传感器-01",
                "type": "浓度传感器",
                "address": "0x02",
                "sampling_frequency": "2Hz",
                "range": "0-1000 ppm",
                "calibration_status": "需校准",
                "calibration_offset": 0.0
            },
            {
                "name": "光照传感器-01",
                "type": "光强传感器",
                "address": "0x03",
                "sampling_frequency": "1Hz",
                "range": "0-2000 lux",
                "calibration_status": "已校准",
                "calibration_offset": 0.0
            },
            {
                "name": "pH传感器-01",
                "type": "pH传感器",
                "address": "0x04",
                "sampling_frequency": "0.5Hz",
                "range": "0-14 pH",
                "calibration_status": "已校准",
                "calibration_offset": 0.0
            },
            {
                "name": "温度传感器-01",
                "type": "温度传感器",
                "address": "0x05",
                "sampling_frequency": "1Hz",
                "range": "-20~100 °C",
                "calibration_status": "异常",
                "calibration_offset": 0.2
            }
        ]
        
        # 获取数据库中的配置
        db_configs = get_all_system_configs(db, "sensor")
        if db_configs:
            for config in db_configs:
                # 更新模拟数据中的配置
                for sensor in sensor_configs:
                    if sensor["name"] == config.config_key:
                        sensor["calibration_offset"] = float(config.config_value)
        
        return {
            "status": "success",
            "data": sensor_configs
        }
        
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"获取传感器配置失败: {str(e)}")

@router.post("/sensors/{sensor_name}/calibrate")
async def calibrate_sensor(
    sensor_name: str,
    offset: float = 0.0,
    current_user: dict = Depends(get_current_active_user),
    db: Session = Depends(get_db)
):
    """
    校准传感器
    """
    check_admin_permission(current_user)
    
    try:
        # 保存校准参数
        config_data = SystemConfigBase(
            config_key=sensor_name,
            config_value=str(offset),
            category="sensor",
            description=f"{sensor_name}校准偏移量"
        )
        
        create_or_update_system_config(db, config_data)
        
        # 记录校准日志
        fault_log = FaultLogBase(
            component=sensor_name,
            fault_type="info",
            description=f"{sensor_name}校准完成，偏移量: {offset}",
            severity="low",
            status="resolved"
        )
        
        create_fault_log(db, fault_log)
        
        return {
            "status": "success",
            "message": f"{sensor_name}校准完成",
            "data": {
                "sensor": sensor_name,
                "calibration_offset": offset,
                "calibrated_by": current_user.username,
                "calibrated_at": datetime.now().isoformat()
            }
        }
        
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"传感器校准失败: {str(e)}")

@router.get("/communication")
async def get_communication_configuration(
    current_user: dict = Depends(get_current_active_user),
    db: Session = Depends(get_db)
):
    """
    获取通信配置
    """
    try:
        # 模拟通信配置
        serial_config = {
            "port": "COM2",
            "baud_rate": "115200",
            "data_bits": "8",
            "parity": "无",
            "stop_bits": "1"
        }
        
        network_config = {
            "ip_address": "192.168.1.100",
            "subnet_mask": "255.255.255.0",
            "gateway": "192.168.1.1",
            "data_port": "5000"
        }
        
        # 获取数据库中的配置
        db_configs = get_all_system_configs(db, "communication")
        if db_configs:
            for config in db_configs:
                if config.config_key in serial_config:
                    serial_config[config.config_key] = config.config_value
                elif config.config_key in network_config:
                    network_config[config.config_key] = config.config_value
        
        return {
            "status": "success",
            "data": {
                "serial": serial_config,
                "network": network_config,
                "communication_status": {
                    "serial": "connected",
                    "network": "connected",
                    "last_test": datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                }
            }
        }
        
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"获取通信配置失败: {str(e)}")

@router.post("/communication/test")
async def test_communication(
    current_user: dict = Depends(get_current_active_user),
    db: Session = Depends(get_db)
):
    """
    测试通信连接
    """
    try:
        # 模拟通信测试
        import time
        time.sleep(1)  # 模拟测试延迟
        
        test_results = {
            "serial_test": {
                "status": "success",
                "response_time": "0.125s",
                "error_rate": "0%"
            },
            "network_test": {
                "status": "success",
                "response_time": "0.045s",
                "packet_loss": "0%"
            },
            "sensor_network_test": {
                "status": "success",
                "connected_sensors": 5,
                "response_time": "0.234s"
            },
            "overall_status": "所有通信连接正常",
            "tested_at": datetime.now().isoformat()
        }
        
        return {
            "status": "success",
            "message": "通信测试成功",
            "data": test_results
        }
        
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"通信测试失败: {str(e)}")

@router.get("/backup")
async def get_backup_configuration(
    current_user: dict = Depends(get_current_active_user),
    db: Session = Depends(get_db)
):
    """
    获取备份配置
    """
    try:
        # 模拟备份配置
        backup_config = {
            "auto_backup": True,
            "backup_frequency": "每周",
            "backup_retention": 10,
            "backup_path": "/backup/config",
            "last_backup": (datetime.now() - timedelta(days=1)).strftime("%Y-%m-%d %H:%M:%S"),
            "next_backup": (datetime.now() + timedelta(days=6)).strftime("%Y-%m-%d %H:%M:%S")
        }
        
        # 模拟备份文件列表
        backup_files = [
            {
                "filename": "config_backup_20240520.zip",
                "size": "2.3MB",
                "created_at": "2024-05-20 02:00:00",
                "contains": ["系统配置", "传感器配置", "控制参数"]
            },
            {
                "filename": "config_backup_20240513.zip",
                "size": "2.2MB",
                "created_at": "2024-05-13 02:00:00",
                "contains": ["系统配置", "传感器配置"]
            },
            {
                "filename": "config_backup_20240506.zip",
                "size": "2.1MB",
                "created_at": "2024-05-06 02:00:00",
                "contains": ["系统配置"]
            }
        ]
        
        return {
            "status": "success",
            "data": {
                "backup_config": backup_config,
                "backup_files": backup_files
            }
        }
        
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"获取备份配置失败: {str(e)}")

@router.post("/backup/create")
async def create_backup(
    current_user: dict = Depends(get_current_active_user),
    db: Session = Depends(get_db)
):
    """
    创建系统备份
    """
    check_admin_permission(current_user)
    
    try:
        # 模拟备份过程
        import time
        time.sleep(2)  # 模拟备份延迟
        
        # 生成备份文件名
        backup_time = datetime.now().strftime("%Y%m%d_%H%M%S")
        backup_filename = f"config_backup_{backup_time}.zip"
        backup_size = f"{random.uniform(2.1, 2.5):.1f}MB"
        
        return {
            "status": "success",
            "message": "系统配置备份创建成功",
            "data": {
                "backup_file": backup_filename,
                "backup_size": backup_size,
                "backup_path": "/backup/config",
                "created_at": datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
                "contains": ["系统配置", "传感器配置", "控制参数", "算法参数"],
                "backup_id": f"backup_{backup_time}"
            }
        }
        
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"创建备份失败: {str(e)}")

@router.post("/backup/restore")
async def restore_backup(
    backup_id: str,
    current_user: dict = Depends(get_current_active_user),
    db: Session = Depends(get_db)
):
    """
    恢复系统备份
    """
    check_admin_permission(current_user)
    
    try:
        # 模拟恢复过程
        import time
        time.sleep(3)  # 模拟恢复延迟
        
        return {
            "status": "success",
            "message": "系统配置恢复成功",
            "data": {
                "backup_id": backup_id,
                "restored_at": datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
                "restored_by": current_user.username,
                "requires_restart": True
            }
        }
        
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"恢复备份失败: {str(e)}")

@router.post("/factory-reset")
async def factory_reset(
    current_user: dict = Depends(get_current_active_user),
    db: Session = Depends(get_db)
):
    """
    恢复出厂设置
    """
    check_admin_permission(current_user)
    
    try:
        # 模拟恢复出厂设置过程
        import time
        time.sleep(4)  # 模拟恢复延迟
        
        # 这里应该实际重置所有配置
        # 目前只是模拟
        
        return {
            "status": "success",
            "message": "系统已恢复出厂设置",
            "data": {
                "reset_at": datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
                "reset_by": current_user.username,
                "reset_items": [
                    "系统配置",
                    "传感器配置",
                    "控制参数",
                    "算法参数",
                    "用户配置"
                ],
                "requires_restart": True
            }
        }
        
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"恢复出厂设置失败: {str(e)}")

@router.get("/users")
async def get_users(
    current_user: dict = Depends(get_current_active_user),
    db: Session = Depends(get_db)
):
    """
    获取用户列表
    """
    try:
        from crud import get_user_by_username
        
        # 模拟用户数据
        users = [
            {
                "username": "admin",
                "role": "管理员",
                "last_login": "2024-05-20 14:30",
                "status": "active"
            },
            {
                "username": "operator1",
                "role": "操作员",
                "last_login": "2024-05-20 10:15",
                "status": "active"
            },
            {
                "username": "viewer1",
                "role": "查看者",
                "last_login": "2024-05-19 16:45",
                "status": "active"
            }
        ]
        
        return {
            "status": "success",
            "data": users
        }
        
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"获取用户列表失败: {str(e)}")

@router.post("/change-password")
async def change_password(
    current_password: str,
    new_password: str,
    confirm_password: str,
    current_user: dict = Depends(get_current_active_user),
    db: Session = Depends(get_db)
):
    """
    修改密码
    """
    try:
        # 验证当前密码
        from crud import authenticate_user
        if not authenticate_user(db, current_user.username, current_password):
            raise HTTPException(status_code=400, detail="当前密码错误")
        
        # 验证新密码
        if new_password != confirm_password:
            raise HTTPException(status_code=400, detail="新密码和确认密码不一致")
        
        if len(new_password) < 8:
            raise HTTPException(status_code=400, detail="密码长度至少8位")
        
        # 更新密码
        from auth import get_password_hash
        current_user.password_hash = get_password_hash(new_password)
        db.commit()
        
        return {
            "status": "success",
            "message": "密码修改成功",
            "data": {
                "username": current_user.username,
                "password_changed_at": datetime.now().isoformat()
            }
        }
        
    except HTTPException:
        raise
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"修改密码失败: {str(e)}")