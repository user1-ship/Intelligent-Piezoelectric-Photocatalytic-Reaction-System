# backend/main.py
from fastapi import FastAPI, Depends, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from fastapi.staticfiles import StaticFiles
from fastapi.responses import FileResponse
from sqlalchemy.orm import Session
from contextlib import asynccontextmanager
import uvicorn
import os
from datetime import datetime

from database import init_db, create_default_data, get_db
from config import settings
from routers import auth, sensors, control, history, config
from utils import SystemMonitor

# 在应用启动前导入所有模型，确保它们被注册到 Base.metadata 中
from models import Base

# 生命周期管理
@asynccontextmanager
async def lifespan(app: FastAPI):
    """应用生命周期管理"""
    # 启动时执行
    print("正在启动智能压电光催化控制平台...")
    
    # 确保数据目录存在
    data_dir = settings.DATA_STORAGE_PATH
    if not os.path.exists(data_dir):
        os.makedirs(data_dir, exist_ok=True)
        print(f"数据目录已创建：{data_dir}")
    
    if settings.BACKUP_PATH and not os.path.exists(settings.BACKUP_PATH):
        os.makedirs(settings.BACKUP_PATH, exist_ok=True)
    
    try:
        # 初始化数据库（创建表）
        print("正在初始化数据库...")
        init_db()
        
        # 创建默认数据
        print("正在创建默认数据...")
        create_default_data()
        print("默认数据创建完成")
        
    except Exception as e:
        print(f"数据库初始化失败：{e}")
        raise
    
    print("启动完成！")
    
    yield
    
    # 关闭时执行
    print("正在关闭应用...")

# 创建FastAPI应用
app = FastAPI(
    title=settings.PROJECT_NAME,
    description="智能压电光催化装置控制平台后端API",
    version=settings.VERSION,
    lifespan=lifespan
)

# 配置CORS
app.add_middleware(
    CORSMiddleware,
    allow_origins=["http://localhost:3000", "http://localhost:8000", "http://127.0.0.1:3000", "http://127.0.0.1:8000"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# 挂载静态文件（前端文件）
app.mount("/static", StaticFiles(directory="static"), name="static")

# 包含路由
app.include_router(auth.router)
app.include_router(sensors.router)
app.include_router(control.router)
app.include_router(history.router)
app.include_router(config.router)

# 根路由
@app.get("/")
async def root():
    """根路由"""
    return {
        "message": "欢迎使用智能压电光催化控制平台API",
        "version": settings.VERSION,
        "docs_url": "/docs",
        "redoc_url": "/redoc"
    }

@app.get("/health")
async def health_check():
    """健康检查"""
    return {
        "status": "healthy",
        "timestamp": datetime.now().isoformat(),
        "service": settings.PROJECT_NAME
    }

@app.get("/api/info")
async def api_info():
    """API信息"""
    return {
        "name": settings.PROJECT_NAME,
        "version": settings.VERSION,
        "description": "智能压电光催化装置控制平台",
        "endpoints": {
            "auth": f"{settings.API_V1_STR}/auth",
            "sensors": f"{settings.API_V1_STR}/sensors",
            "control": f"{settings.API_V1_STR}/control",
            "history": f"{settings.API_V1_STR}/history",
            "config": f"{settings.API_V1_STR}/config"
        }
    }

@app.get("/api/status")
async def system_status():
    """系统状态"""
    try:
        status_info = SystemMonitor.get_system_status()
        return {
            "status": "success",
            "data": status_info
        }
    except Exception as e:
        return {
            "status": "error",
            "message": f"获取系统状态失败：{str(e)}"
        }

# 服务前端页面
@app.get("/front.html")
async def serve_front():
    """服务前端页面"""
    try:
        return FileResponse("static/front.html")
    except:
        return {"error": "静态文件未找到"}

@app.get("/history.html")
async def serve_history():
    """服务历史数据页面"""
    try:
        return FileResponse("static/history.html")
    except:
        return {"error": "静态文件未找到"}

@app.get("/config.html")
async def serve_config():
    """服务系统配置页面"""
    try:
        return FileResponse("static/config.html")
    except:
        return {"error": "静态文件未找到"}

@app.get("/help.html")
async def serve_help():
    """服务帮助页面"""
    try:
        return FileResponse("static/help.html")
    except:
        return {"error": "静态文件未找到"}

@app.get("/log.html")
async def serve_login():
    """服务登录页面"""
    try:
        return FileResponse("static/log.html")
    except:
        return {"error": "静态文件未找到"}

# 错误处理
@app.exception_handler(404)
async def not_found_exception_handler(request, exc):
    """404错误处理"""
    return {
        "status": "error",
        "message": "请求的资源不存在",
        "path": request.url.path
    }

@app.exception_handler(500)
async def internal_error_handler(request, exc):
    """500错误处理"""
    return {
        "status": "error",
        "message": "服务器内部错误",
        "detail": str(exc) if settings.DEBUG else "内部服务器错误"
    }

if __name__ == "__main__":
    # 运行应用
    uvicorn.run(
        "main:app",
        host="0.0.0.0",
        port=8000,
        reload=True,
        log_level="info"
    )