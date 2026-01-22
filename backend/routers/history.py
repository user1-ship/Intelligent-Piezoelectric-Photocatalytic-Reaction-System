from fastapi import APIRouter, Depends, HTTPException, Query
from sqlalchemy.orm import Session
from datetime import datetime, timedelta
from typing import List, Optional
import json

from database import get_db
from crud import get_sensor_data_history, get_history_data
from schemas import HistoryQuery, SensorData, HistoryData
from auth import get_current_active_user
from utils import DataExporter
from config import settings

router = APIRouter(prefix=f"{settings.API_V1_STR}/history", tags=["历史数据"])

@router.get("/", response_model=dict)
async def get_history(
    start_time: Optional[datetime] = None,
    end_time: Optional[datetime] = None,
    data_type: str = "all",
    frequency: str = "daily",
    current_user: dict = Depends(get_current_active_user),
    db: Session = Depends(get_db)
):
    """
    获取历史数据
    """
    try:
        # 如果没有提供时间范围，默认查询最近20天
        if not end_time:
            end_time = datetime.now()
        if not start_time:
            start_time = end_time - timedelta(days=20)
        
        # 构建查询对象
        query = HistoryQuery(
            start_time=start_time,
            end_time=end_time,
            data_type=data_type
        )
        
        if data_type == "sensor":
            # 获取传感器历史数据
            history_records = get_sensor_data_history(
                db, start_time, end_time, data_type
            )
            
            # 根据频率聚合数据
            aggregated_data = aggregate_by_frequency(history_records, frequency)
            
            # 统计数据概览
            stats = calculate_statistics(aggregated_data)
            
            return {
                "status": "success",
                "data": {
                    "records": aggregated_data,
                    "statistics": stats,
                    "query_info": {
                        "start_time": start_time.isoformat(),
                        "end_time": end_time.isoformat(),
                        "data_type": data_type,
                        "frequency": frequency,
                        "record_count": len(aggregated_data)
                    }
                }
            }
        else:
            # 获取其他类型的历史数据
            history_records = get_history_data(db, query)
            
            return {
                "status": "success",
                "data": {
                    "records": [
                        {
                            "data_type": record.data_type,
                            "value": record.value,
                            "timestamp": record.timestamp.isoformat(),
                            "metadata": json.loads(record.metadata) if record.metadata else {}
                        }
                        for record in history_records
                    ]
                }
            }
        
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"获取历史数据失败: {str(e)}")

@router.get("/trend")
async def get_history_trend(
    days: int = 20,
    current_user: dict = Depends(get_current_active_user),
    db: Session = Depends(get_db)
):
    """
    获取历史趋势数据
    """
    try:
        end_time = datetime.now()
        start_time = end_time - timedelta(days=days)
        
        # 获取传感器历史数据
        history_records = get_sensor_data_history(db, start_time, end_time)
        
        # 按日期分组
        daily_data = {}
        for record in history_records:
            date_str = record.timestamp.strftime("%m-%d")
            if date_str not in daily_data:
                daily_data[date_str] = {
                    "flow": [], "pollution": [], "efficiency": [], "energy": []
                }
            
            if record.sensor_type in daily_data[date_str]:
                daily_data[date_str][record.sensor_type].append(record.value)
        
        # 计算每日平均值
        dates = []
        flow_avg = []
        pollution_avg = []
        efficiency_avg = []
        energy_avg = []
        
        for date_str, values in sorted(daily_data.items()):
            dates.append(date_str)
            flow_avg.append(round(sum(values["flow"]) / len(values["flow"]) if values["flow"] else 0, 1))
            pollution_avg.append(round(sum(values["pollution"]) / len(values["pollution"]) if values["pollution"] else 0, 1))
            efficiency_avg.append(round(sum(values["efficiency"]) / len(values["efficiency"]) if values["efficiency"] else 0, 1))
            energy_avg.append(round(sum(values["energy"]) / len(values["energy"]) if values["energy"] else 0, 1))
        
        return {
            "status": "success",
            "data": {
                "dates": dates,
                "flow": flow_avg,
                "pollution": pollution_avg,
                "efficiency": efficiency_avg,
                "energy": energy_avg,
                "trend_analysis": analyze_trends(pollution_avg, efficiency_avg, energy_avg)
            }
        }
        
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"获取历史趋势失败: {str(e)}")

@router.get("/comparison")
async def get_data_comparison(
    period1_start: datetime,
    period1_end: datetime,
    period2_start: datetime,
    period2_end: datetime,
    current_user: dict = Depends(get_current_active_user),
    db: Session = Depends(get_db)
):
    """
    数据对比分析
    """
    try:
        # 获取两个时间段的数据
        period1_data = get_sensor_data_history(db, period1_start, period1_end)
        period2_data = get_sensor_data_history(db, period2_start, period2_end)
        
        # 计算统计数据
        period1_stats = calculate_period_statistics(period1_data)
        period2_stats = calculate_period_statistics(period2_data)
        
        # 计算变化百分比
        comparison = compare_periods(period1_stats, period2_stats)
        
        return {
            "status": "success",
            "data": {
                "period1": {
                    "range": f"{period1_start.strftime('%Y-%m-%d')} 至 {period1_end.strftime('%Y-%m-%d')}",
                    "statistics": period1_stats
                },
                "period2": {
                    "range": f"{period2_start.strftime('%Y-%m-%d')} 至 {period2_end.strftime('%Y-%m-%d')}",
                    "statistics": period2_stats
                },
                "comparison": comparison
            }
        }
        
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"数据对比分析失败: {str(e)}")

@router.get("/export")
async def export_history_data(
    start_time: Optional[datetime] = None,
    end_time: Optional[datetime] = None,
    data_type: str = "all",
    format: str = "csv",
    current_user: dict = Depends(get_current_active_user),
    db: Session = Depends(get_db)
):
    """
    导出历史数据
    """
    try:
        if not end_time:
            end_time = datetime.now()
        if not start_time:
            start_time = end_time - timedelta(days=30)
        
        # 获取数据
        query = HistoryQuery(
            start_time=start_time,
            end_time=end_time,
            data_type=data_type
        )
        
        if data_type == "sensor":
            records = get_sensor_data_history(db, start_time, end_time)
            data = [
                {
                    "时间": record.timestamp.strftime("%Y-%m-%d %H:%M:%S"),
                    "传感器类型": record.sensor_type,
                    "数值": record.value,
                    "单位": record.unit,
                    "数据质量": record.data_quality
                }
                for record in records
            ]
        else:
            records = get_history_data(db, query)
            data = [
                {
                    "时间": record.timestamp.strftime("%Y-%m-%d %H:%M:%S"),
                    "数据类型": record.data_type,
                    "数值": record.value
                }
                for record in records
            ]
        
        # 导出数据
        if format == "csv":
            exported_data = DataExporter.export_to_csv(data)
            content_type = "text/csv"
            filename = f"历史数据导出_{datetime.now().strftime('%Y%m%d_%H%M%S')}.csv"
        else:
            exported_data = DataExporter.export_to_json(data)
            content_type = "application/json"
            filename = f"历史数据导出_{datetime.now().strftime('%Y%m%d_%H%M%S')}.json"
        
        return {
            "status": "success",
            "message": "数据导出成功",
            "data": {
                "filename": filename,
                "content_type": content_type,
                "data_size": len(exported_data),
                "record_count": len(data)
            },
            "exported_data": exported_data
        }
        
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"数据导出失败: {str(e)}")

@router.get("/details")
async def get_history_details(
    page: int = 1,
    page_size: int = 20,
    sensor_type: Optional[str] = None,
    current_user: dict = Depends(get_current_active_user),
    db: Session = Depends(get_db)
):
    """
    获取详细历史数据（分页）
    """
    try:
        # 模拟分页数据
        total_records = 28650
        start_index = (page - 1) * page_size
        
        # 生成模拟数据
        data = []
        for i in range(start_index, min(start_index + page_size, total_records)):
            record_time = datetime.now() - timedelta(hours=i)
            data.append({
                "id": i + 1,
                "timestamp": record_time.strftime("%Y-%m-%d %H:%M:%S"),
                "flow": round(28.5 - i * 0.001, 1),
                "pollution": round(156.8 - i * 0.01, 1),
                "light": round(890.2 - i * 0.1, 1),
                "ph": round(7.2 - i * 0.0001, 2),
                "temperature": round(25.8 + i * 0.001, 1),
                "energy": round(68.4 - i * 0.005, 1),
                "efficiency": round(89.6 + i * 0.002, 1),
                "data_quality": random.randint(85, 100)
            })
        
        return {
            "status": "success",
            "data": {
                "records": data,
                "pagination": {
                    "page": page,
                    "page_size": page_size,
                    "total_records": total_records,
                    "total_pages": (total_records + page_size - 1) // page_size
                }
            }
        }
        
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"获取详细数据失败: {str(e)}")

# 辅助函数
def aggregate_by_frequency(records: List[SensorData], frequency: str) -> List[dict]:
    """根据频率聚合数据"""
    if not records:
        return []
    
    aggregated = []
    
    if frequency == "hourly":
        # 按小时聚合
        hourly_data = {}
        for record in records:
            hour_key = record.timestamp.strftime("%Y-%m-%d %H:00")
            if hour_key not in hourly_data:
                hourly_data[hour_key] = []
            hourly_data[hour_key].append(record.value)
        
        for hour_key, values in hourly_data.items():
            aggregated.append({
                "timestamp": hour_key,
                "value": round(sum(values) / len(values), 2),
                "count": len(values)
            })
    
    elif frequency == "daily":
        # 按日聚合
        daily_data = {}
        for record in records:
            date_key = record.timestamp.strftime("%Y-%m-%d")
            if date_key not in daily_data:
                daily_data[date_key] = []
            daily_data[date_key].append(record.value)
        
        for date_key, values in daily_data.items():
            aggregated.append({
                "timestamp": date_key,
                "value": round(sum(values) / len(values), 2),
                "count": len(values)
            })
    
    else:
        # 返回原始数据
        aggregated = [
            {
                "timestamp": record.timestamp.strftime("%Y-%m-%d %H:%M:%S"),
                "value": record.value,
                "unit": record.unit
            }
            for record in records
        ]
    
    return aggregated

def calculate_statistics(data: List[dict]) -> dict:
    """计算统计数据"""
    if not data:
        return {}
    
    values = [item["value"] for item in data if "value" in item]
    
    if not values:
        return {}
    
    return {
        "count": len(values),
        "mean": round(sum(values) / len(values), 2),
        "min": round(min(values), 2),
        "max": round(max(values), 2),
        "std": round(calculate_std(values), 2)
    }

def calculate_std(values: List[float]) -> float:
    """计算标准差"""
    if len(values) <= 1:
        return 0.0
    
    mean = sum(values) / len(values)
    variance = sum((x - mean) ** 2 for x in values) / (len(values) - 1)
    return variance ** 0.5

def calculate_period_statistics(records: List[SensorData]) -> dict:
    """计算时间段统计数据"""
    if not records:
        return {}
    
    # 按传感器类型分组
    sensor_data = {}
    for record in records:
        if record.sensor_type not in sensor_data:
            sensor_data[record.sensor_type] = []
        sensor_data[record.sensor_type].append(record.value)
    
    # 计算各传感器的统计数据
    stats = {}
    for sensor_type, values in sensor_data.items():
        if values:
            stats[sensor_type] = {
                "mean": round(sum(values) / len(values), 2),
                "count": len(values),
                "min": round(min(values), 2),
                "max": round(max(values), 2)
            }
    
    return stats

def compare_periods(period1_stats: dict, period2_stats: dict) -> dict:
    """对比两个时间段的数据"""
    comparison = {}
    
    for sensor_type in set(period1_stats.keys()) | set(period2_stats.keys()):
        if sensor_type in period1_stats and sensor_type in period2_stats:
            p1_mean = period1_stats[sensor_type]["mean"]
            p2_mean = period2_stats[sensor_type]["mean"]
            
            if p1_mean != 0:
                change_percent = ((p2_mean - p1_mean) / p1_mean) * 100
            else:
                change_percent = 0
            
            comparison[sensor_type] = {
                "period1_mean": p1_mean,
                "period2_mean": p2_mean,
                "change": round(change_percent, 1),
                "trend": "上升" if change_percent > 0 else "下降" if change_percent < 0 else "持平"
            }
    
    return comparison

def analyze_trends(pollution: List[float], efficiency: List[float], energy: List[float]) -> dict:
    """分析趋势"""
    if len(pollution) < 2:
        return {}
    
    # 计算趋势
    pollution_trend = calculate_trend(pollution)
    efficiency_trend = calculate_trend(efficiency)
    energy_trend = calculate_trend(energy)
    
    # 生成分析结论
    conclusion = []
    if pollution_trend < -0.5:
        conclusion.append("污染物浓度呈现明显下降趋势")
    elif pollution_trend > 0.5:
        conclusion.append("污染物浓度呈现上升趋势，需关注")
    else:
        conclusion.append("污染物浓度保持稳定")
    
    if efficiency_trend > 0.5:
        conclusion.append("系统效率持续提升")
    elif efficiency_trend < -0.5:
        conclusion.append("系统效率有所下降，需要检查")
    else:
        conclusion.append("系统效率保持稳定")
    
    if energy_trend < -0.5:
        conclusion.append("能耗持续优化，节能效果显著")
    elif energy_trend > 0.5:
        conclusion.append("能耗有所增加，需要优化")
    else:
        conclusion.append("能耗保持稳定")
    
    return {
        "pollution_trend": round(pollution_trend, 2),
        "efficiency_trend": round(efficiency_trend, 2),
        "energy_trend": round(energy_trend, 2),
        "conclusion": "；".join(conclusion)
    }

def calculate_trend(values: List[float]) -> float:
    """计算线性趋势"""
    if len(values) < 2:
        return 0.0
    
    # 简单趋势计算：首尾差异百分比
    first = values[0]
    last = values[-1]
    
    if first != 0:
        return ((last - first) / first) * 100
    return 0.0