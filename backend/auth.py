# backend/auth.py
from fastapi import Depends, HTTPException, status
from fastapi.security import OAuth2PasswordBearer
from typing import Optional

from crud import get_user_by_username, authenticate_user
from schemas import TokenData
from database import SessionLocal
from security import (
    verify_password, 
    get_password_hash, 
    create_access_token, 
    create_remember_me_token,
    decode_token,
    SECRET_KEY,
    ALGORITHM
)

# OAuth2 配置
oauth2_scheme = OAuth2PasswordBearer(tokenUrl="api/v1/auth/login")

async def get_current_user(token: str = Depends(oauth2_scheme)):
    """获取当前用户"""
    credentials_exception = HTTPException(
        status_code=status.HTTP_401_UNAUTHORIZED,
        detail="无法验证凭据",
        headers={"WWW-Authenticate": "Bearer"},
    )
    
    payload = decode_token(token)
    if not payload:
        raise credentials_exception
        
    username: str = payload.get("sub")
    if username is None:
        raise credentials_exception
        
    token_data = TokenData(username=username)
    
    # 从数据库获取用户
    db = SessionLocal()
    try:
        user = get_user_by_username(db, username=token_data.username)
        if user is None:
            raise credentials_exception
        return user
    finally:
        db.close()

async def get_current_active_user(current_user = Depends(get_current_user)):
    """获取当前活跃用户"""
    if not current_user.is_active:
        raise HTTPException(status_code=400, detail="用户已禁用")
    return current_user

# 权限检查
def check_admin_permission(user):
    """检查管理员权限"""
    if user.role != "admin":
        raise HTTPException(
            status_code=status.HTTP_403_FORBIDDEN,
            detail="需要管理员权限"
        )

def check_operator_permission(user):
    """检查操作员权限"""
    if user.role not in ["admin", "operator"]:
        raise HTTPException(
            status_code=status.HTTP_403_FORBIDDEN,
            detail="需要操作员或管理员权限"
        )

# 为了方便导入，这里也导出安全函数
__all__ = [
    "oauth2_scheme",
    "get_current_user",
    "get_current_active_user",
    "check_admin_permission",
    "check_operator_permission",
    "create_access_token",
    "create_remember_me_token",
    "verify_password",
    "get_password_hash",
    "SECRET_KEY",
    "ALGORITHM"
]