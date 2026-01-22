# backend/database.py
from sqlalchemy import create_engine
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import sessionmaker
from config import settings

# 创建数据库引擎
engine = create_engine(
    settings.DATABASE_URL, 
    connect_args={"check_same_thread": False}  # SQLite需要这个参数
)

# 创建SessionLocal类
SessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)

# 创建基类
Base = declarative_base()

def init_db():
    """初始化数据库，创建所有表"""
    # 需要先导入 models 以确保所有模型都被注册到 Base.metadata 中
    from models import Base, User, SensorData, ControlParameter, SystemConfig, HistoryData, FaultLog
    
    print("开始创建数据库表...")
    Base.metadata.create_all(bind=engine)
    print("数据库表创建完成！")

def get_db():
    """获取数据库会话"""
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()

def create_default_data():
    """创建默认数据"""
    print("开始创建默认数据...")
    
    from crud import get_user_by_username, create_user
    from schemas import UserCreate
    from security import get_password_hash
    
    db = SessionLocal()
    
    try:
        # 创建默认管理员用户
        admin_user = get_user_by_username(db, "admin")
        if not admin_user:
            print("创建管理员用户...")
            hashed_password = get_password_hash("admin123")
            admin_user = UserCreate(
                username="admin",
                password="admin123",
                role="admin"
            )
            create_user(db, admin_user)
            print("管理员用户创建成功：admin / admin123")
        
        # 创建默认操作员用户
        operator_user = get_user_by_username(db, "operator")
        if not operator_user:
            print("创建操作员用户...")
            operator_user = UserCreate(
                username="operator",
                password="operator123",
                role="operator"
            )
            create_user(db, operator_user)
            print("操作员用户创建成功：operator / operator123")
        
        # 创建默认查看者用户
        viewer_user = get_user_by_username(db, "viewer")
        if not viewer_user:
            print("创建查看者用户...")
            viewer_user = UserCreate(
                username="viewer",
                password="viewer123",
                role="viewer"
            )
            create_user(db, viewer_user)
            print("查看者用户创建成功：viewer / viewer123")
        
        db.commit()
        print("默认数据创建完成！")
        
    except Exception as e:
        db.rollback()
        print(f"创建默认数据时出错：{e}")
        raise e
    finally:
        db.close()