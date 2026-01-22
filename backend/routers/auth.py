# backend/routers/auth.py
from datetime import timedelta
from fastapi import APIRouter, Depends, HTTPException, status
from fastapi.security import OAuth2PasswordRequestForm
from sqlalchemy.orm import Session

from database import get_db
from crud import authenticate_user, update_user_last_login
from security import create_access_token, create_remember_me_token
from schemas import UserLogin, Token
from config import settings
from auth import get_current_active_user  # 添加这行导入

router = APIRouter(prefix=f"{settings.API_V1_STR}/auth", tags=["认证"])

@router.post("/login", response_model=Token)
async def login(
    user_data: UserLogin,
    db: Session = Depends(get_db)
):
    # 验证用户
    user = authenticate_user(db, user_data.username, user_data.password)
    if not user:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="用户名或密码错误",
            headers={"WWW-Authenticate": "Bearer"},
        )
    
    # 更新最后登录时间
    update_user_last_login(db, user.id)
    
    # 根据remember_me选择token过期时间
    if user_data.remember_me:
        access_token = create_remember_me_token(data={"sub": user.username})
    else:
        access_token = create_access_token(data={"sub": user.username})
    
    # 准备用户信息
    user_info = {
        "id": user.id,
        "username": user.username,
        "role": user.role,
        "last_login": user.last_login.isoformat() if user.last_login else None
    }
    
    return {
        "access_token": access_token,
        "token_type": "bearer",
        "user_info": user_info
    }

@router.get("/me")
async def get_current_user_info(
    current_user: dict = Depends(get_current_active_user)  # 修改这里，去掉 auth.
):
    return {
        "status": "success",
        "data": {
            "id": current_user.id,
            "username": current_user.username,
            "role": current_user.role,
            "is_active": current_user.is_active,
            "last_login": current_user.last_login
        }
    }