Intelligent-Piezoelectric-Photocatalytic-Reaction-System
智能压电光催化反应系统是一套覆盖前端人机交互、硬件底层控制、后端数据管理的全栈解决方案，基于 Arduino + Python + 前端静态页面技术栈构建，旨在实现压电光催化反应过程的智能化监控、设备精准控制、数据持久化存储与可视化管理。
核心特性
🎛️ 全链路硬件控制：基于 Arduino 实现压电 / 光催化设备的底层驱动、传感器数据实时采集、执行器精准控制；
📊 可视化人机交互：前端提供配置、实时监控、历史数据追溯、日志查看、帮助文档等全场景操作界面；
💾 可靠的数据管理：后端基于 SQLite 实现数据持久化存储，内置自动备份机制，保障反应数据不丢失；
🔐 安全权限控制：后端集成权限认证、数据加密机制，规范系统访问与数据操作；
🧩 模块化架构：前端 / 后端 / 硬件分层设计，代码解耦，便于功能扩展与维护。
环境要求
1. 硬件控制模块（MainControl）
Arduino IDE 2.0+（或兼容的 Arduino 开发环境）
兼容的 Arduino 主控板（如 Uno/Nano/Mega，根据硬件适配）
压电 / 光催化反应设备、传感器（如温度 / 浓度传感器）、执行器等外设
2. 后端服务模块（backend）
Python 3.8+
依赖库：详见 backend/requirements.txt（FastAPI、SQLAlchemy、pyserial 等）
3. 前端模块（frontend）
现代浏览器（Chrome/Firefox/Edge 最新版）
可选：Nginx/Apache（用于部署前端静态页面，本地测试可直接打开 HTML 文件）
快速开始
1. 克隆仓库
bash
运行
git clone https://github.com/[你的用户名]/Intelligent-Piezoelectric-Photocatalytic-Reaction-System.git
cd Intelligent-Piezoelectric-Photocatalytic-Reaction-System
2. 硬件控制模块部署
打开 Arduino IDE，导入 MainControl/MainControl.ino 文件；
根据实际硬件接线，修改代码中传感器 / 执行器的引脚定义；
将程序烧录到 Arduino 主控板；
连接 Arduino 与电脑（或工控机），确认串口通信正常。
3. 后端服务启动
bash
运行
# 进入后端目录
cd backend
# 安装依赖
pip install -r requirements.txt
# 启动 FastAPI 服务（默认端口 8000）
uvicorn main:app --host 0.0.0.0 --port 8000
# 可选：后台运行服务（Linux/macOS）
nohup uvicorn main:app --host 0.0.0.0 --port 8000 > backend.log 2>&1 &
服务启动后，可访问 http://localhost:8000/docs 查看自动生成的 API 文档；
根据实际需求修改 backend/config.py 中的串口、数据库、端口等配置。
4. 前端页面访问
本地测试：直接打开 frontend/front.html 文件即可进入系统主界面；
生产部署：将 frontend 目录部署到 Nginx/Apache 静态资源目录，配置反向代理对接后端 API。
项目结构
plaintext
Intelligent-Piezoelectric-Photocatalytic-Reaction-System/
├── frontend/                # 前端交互模块（静态页面）
│   ├── api.js               # 后端接口封装
│   ├── config.html          # 系统配置页面
│   ├── front.html           # 系统主界面（实时监控）
│   ├── help.html            # 帮助文档页面
│   ├── history.html         # 历史数据追溯页面
│   └── log.html             # 系统日志页面
├── MainControl/             # 硬件主控模块（Arduino）
│   ├── MainControl.ino      # Arduino 主控程序
│   ├── src/                 # 硬件控制辅助源码
│   └── 结构.txt             # 硬件模块结构说明
├── backend/                 # 后端服务模块（Python/FastAPI）
│   ├── main.py              # 后端服务入口
│   ├── database.py          # 数据库配置
│   ├── models.py            # 数据模型定义
│   ├── schemas.py           # 数据校验/序列化
│   ├── crud.py              # 数据库CRUD操作
│   ├── config.py            # 全局配置
│   ├── auth.py/security.py  # 权限认证/数据安全
│   ├── sensor_data.db       # SQLite 数据库文件
│   ├── backups/             # 数据备份目录
│   ├── routers/             # 接口路由拆分
│   └── requirements.txt     # Python 依赖清单
└── README.md                # 项目说明文档
核心模块说明
1. 前端模块（frontend）
作为用户操作入口，提供直观的可视化界面：
front.html：核心主界面，展示实时传感器数据、设备运行状态；
config.html：配置设备通信参数、反应工艺参数等核心设置；
history.html：查询 / 导出历史反应数据，支持时间范围筛选；
log.html：查看系统运行日志、错误日志，辅助故障定位；
api.js：统一封装前端与后端的接口请求，简化数据交互逻辑。
2. 硬件主控模块（MainControl）
系统与物理设备的交互核心：
MainControl.ino：实现硬件初始化、传感器数据采集、压电 / 光催化执行器控制、串口通信等核心逻辑；
适配多种传感器 / 执行器，可根据实际硬件修改引脚和通信协议。
3. 后端服务模块（backend）
系统的数据与服务中枢：
承接前端请求，下发指令至硬件，同步采集传感器数据；
基于 SQLite 实现数据持久化存储，backups 目录自动备份核心数据；
内置权限认证、数据校验机制，保障系统安全与数据准确性；
模块化路由设计，便于扩展新功能（如多设备管理、数据分析）。
数据备份与维护
后端会定期将 sensor_data.db 数据库文件备份至 backend/backups 目录；
建议定期手动备份 backups 目录至外部存储，防止数据丢失；
修改硬件 / 系统配置前，建议先备份 config.py（后端）、MainControl.ino（硬件）等核心配置文件。
贡献指南
Fork 本仓库至个人账号；
创建特性分支：git checkout -b feature/xxx；
提交代码：git commit -m 'feat: 新增xxx功能'；
推送分支：git push origin feature/xxx；
提交 Pull Request 至主仓库，描述功能 / 修复内容。
许可证
本项目基于 MIT License 开源（如需修改许可证类型，请替换 LICENSE 文件并更新此处）。
注意事项
硬件接线需严格遵循 Arduino 引脚定义，避免短路损坏设备；
后端服务启动前需确认 Arduino 串口已正确连接，且无其他程序占用串口；
前端页面若无法访问后端 API，需检查浏览器跨域设置、后端服务端口是否开放。
总结
该项目是覆盖硬件控制、后端服务、前端交互的智能压电光催化反应系统全栈解决方案，基于 Arduino + Python + 静态前端技术栈构建；
核心优势为模块化架构、全链路硬件控制、可靠的数据管理与可视化交互；
部署需分别配置硬件（Arduino 烧录）、后端（Python 依赖安装 + 服务启动）、前端（直接打开 / 静态部署）三个模块，使用前需确认硬件串口与后端配置匹配。
