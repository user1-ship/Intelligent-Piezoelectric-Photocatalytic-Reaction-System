from fastapi import APIRouter, Depends, HTTPException
from sqlalchemy.orm import Session
from datetime import datetime
import json

from database import get_db
from crud import (
    get_all_control_params, create_or_update_control_param,
    get_pid_params, get_mpc_params
)
from schemas import ControlParameter, ControlParameterCreate
from auth import get_current_active_user, check_operator_permission
from config import settings

router = APIRouter(prefix=f"{settings.API_V1_STR}/control", tags=["控制"])

@router.get("/params", response_model=dict)
async def get_control_parameters(
    current_user: dict = Depends(get_current_active_user),
    db: Session = Depends(get_db)
):
    """
    获取所有控制参数
    """
    try:
        # 获取所有参数
        all_params = get_all_control_params(db)
        
        # 按类型分组
        params_dict = {}
        for param in all_params:
            if param.param_type not in params_dict:
                params_dict[param.param_type] = {}
            params_dict[param.param_type][param.param_name] = param.param_value
        
        # 如果没有参数，返回默认值
        if not params_dict:
            params_dict = {
                "pid": {
                    "kp": "2.85",
                    "ki": "0.42",
                    "kd": "0.18"
                },
                "mpc": {
                    "prediction_horizon": "10.0",
                    "control_horizon": "5.0",
                    "weights": "[0.8, 0.15, 0.05]"
                },
                "algorithm": {
                    "learning_rate": "0.01",
                    "discount_factor": "0.95",
                    "exploration_rate": "0.085",
                    "batch_size": "32"
                },
                "system": {
                    "control_mode": "auto",
                    "efficiency_threshold": "85.0"
                }
            }
        
        return {
            "status": "success",
            "data": params_dict
        }
        
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"获取控制参数失败: {str(e)}")

@router.post("/params")
async def update_control_parameters(
    params: dict,
    current_user: dict = Depends(get_current_active_user),
    db: Session = Depends(get_db)
):
    """
    更新控制参数
    """
    check_operator_permission(current_user)
    
    try:
        updated_params = []
        
        for param_type, param_dict in params.items():
            for param_name, param_value in param_dict.items():
                # 创建参数对象
                param_data = ControlParameterCreate(
                    param_name=param_name,
                    param_value=str(param_value),
                    param_type=param_type,
                    description=f"{param_type}控制参数"
                )
                
                # 更新参数
                updated_param = create_or_update_control_param(
                    db, param_data, current_user.id
                )
                updated_params.append(updated_param)
        
        return {
            "status": "success",
            "message": "控制参数更新成功",
            "data": {
                "updated_count": len(updated_params),
                "updated_by": current_user.username,
                "updated_at": datetime.now().isoformat()
            }
        }
        
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"更新控制参数失败: {str(e)}")

@router.get("/pid")
async def get_pid_parameters(
    current_user: dict = Depends(get_current_active_user),
    db: Session = Depends(get_db)
):
    """
    获取PID控制参数
    """
    try:
        pid_params = get_pid_params(db)
        
        # 如果数据库中没有，返回默认值
        pid_data = {}
        for key, param in pid_params.items():
            if param:
                pid_data[key] = float(param.param_value)
            else:
                # 默认值
                defaults = {"kp": 2.85, "ki": 0.42, "kd": 0.18}
                pid_data[key] = defaults.get(key, 0.0)
        
        return {
            "status": "success",
            "data": pid_data
        }
        
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"获取PID参数失败: {str(e)}")

@router.get("/mpc")
async def get_mpc_parameters(
    current_user: dict = Depends(get_current_active_user),
    db: Session = Depends(get_db)
):
    """
    获取MPC控制参数
    """
    try:
        mpc_params = get_mpc_params(db)
        
        # 如果数据库中没有，返回默认值
        mpc_data = {
            "prediction_horizon": 10.0,
            "control_horizon": 5.0,
            "weights": [0.8, 0.15, 0.05]
        }
        
        for key, param in mpc_params.items():
            if param:
                if key == "weights":
                    mpc_data[key] = json.loads(param.param_value)
                else:
                    mpc_data[key] = float(param.param_value)
        
        return {
            "status": "success",
            "data": mpc_data
        }
        
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"获取MPC参数失败: {str(e)}")

@router.get("/mode")
async def get_control_mode(
    current_user: dict = Depends(get_current_active_user),
    db: Session = Depends(get_db)
):
    """
    获取当前控制模式
    """
    try:
        from crud import get_control_parameter
        
        mode_param = get_control_parameter(db, "control_mode")
        
        if mode_param:
            control_mode = mode_param.param_value
        else:
            control_mode = "auto"  # 默认自动模式
        
        # 获取控制输出值（模拟）
        control_output = 75.2
        system_efficiency = 89.6
        
        return {
            "status": "success",
            "data": {
                "control_mode": control_mode,
                "control_output": control_output,
                "system_efficiency": system_efficiency,
                "mode_options": ["auto", "manual", "learning"]
            }
        }
        
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"获取控制模式失败: {str(e)}")

@router.post("/mode/{mode}")
async def set_control_mode(
    mode: str,
    current_user: dict = Depends(get_current_active_user),
    db: Session = Depends(get_db)
):
    """
    设置控制模式
    """
    check_operator_permission(current_user)
    
    if mode not in ["auto", "manual", "learning"]:
        raise HTTPException(status_code=400, detail="无效的控制模式")
    
    try:
        # 更新控制模式
        param_data = ControlParameterCreate(
            param_name="control_mode",
            param_value=mode,
            param_type="system",
            description="系统控制模式"
        )
        
        create_or_update_control_param(db, param_data, current_user.id)
        
        return {
            "status": "success",
            "message": f"控制模式已切换为{mode}模式",
            "data": {
                "control_mode": mode,
                "updated_by": current_user.username,
                "updated_at": datetime.now().isoformat()
            }
        }
        
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"设置控制模式失败: {str(e)}")

@router.post("/output")
async def set_control_output(
    output_value: float,
    current_user: dict = Depends(get_current_active_user),
    db: Session = Depends(get_db)
):
    """
    设置控制输出值
    """
    check_operator_permission(current_user)
    
    try:
        # 确保输出值在合理范围内
        output_value = max(0.0, min(100.0, output_value))
        
        # 这里可以实现实际的控制逻辑
        # 目前只是模拟
        
        return {
            "status": "success",
            "message": f"控制输出已设置为{output_value}%",
            "data": {
                "control_output": output_value,
                "set_by": current_user.username,
                "set_at": datetime.now().isoformat(),
                "predicted_efficiency": min(95.0, 85.0 + output_value * 0.1)
            }
        }
        
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"设置控制输出失败: {str(e)}")

@router.get("/decision")
async def get_control_decision(
    current_user: dict = Depends(get_current_active_user),
    db: Session = Depends(get_db)
):
    """
    获取控制决策理由
    """
    try:
        # 模拟决策理由
        decision_reason = """
        当前污染物浓度为156.8ppm，温度25.8°C，系统效率89.6%，
        采用标准控制模式，控制输出调整至75.2%以平衡能耗和净化效率。
        冲击负荷检测为阴性，无需增强控制输出。
        """
        
        return {
            "status": "success",
            "data": {
                "decision_reason": decision_reason,
                "recommended_action": "维持当前控制策略",
                "confidence": 0.92,
                "generated_at": datetime.now().isoformat()
            }
        }
        
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"获取控制决策失败: {str(e)}")