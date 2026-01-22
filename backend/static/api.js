// frontend/api.js
const API_BASE_URL = 'http://localhost:8000/api';

// 获取当前token（从localStorage或sessionStorage）
function getToken() {
    return localStorage.getItem('access_token') || sessionStorage.getItem('access_token') || '';
}

// API请求封装
async function apiRequest(endpoint, options = {}) {
    const url = `${API_BASE_URL}${endpoint}`;
    
    const defaultOptions = {
        headers: {
            'Content-Type': 'application/json',
            'Authorization': `Bearer ${getToken()}`
        }
    };
    
    const response = await fetch(url, { ...defaultOptions, ...options });
    
    if (!response.ok) {
        const errorData = await response.json().catch(() => ({}));
        throw new Error(errorData.detail || `API请求失败: ${response.status}`);
    }
    
    return response.json();
}

// 用户认证API
export const authApi = {
    login: (username, password, rememberMe = false) => {
        return apiRequest('/auth/login', {
            method: 'POST',
            body: JSON.stringify({ username, password, remember_me: rememberMe })
        });
    },
    
    getCurrentUser: () => {
        return apiRequest('/auth/me');
    }
};

// 传感器数据API
export const sensorApi = {
    getAllSensors: () => {
        return apiRequest('/sensors');
    },
    
    getSensor: (sensorType) => {
        return apiRequest(`/sensors/${sensorType}`);
    }
};

// 控制参数API
export const controlApi = {
    getParams: () => {
        return apiRequest('/control/params');
    },
    
    updateParams: (params) => {
        return apiRequest('/control/params', {
            method: 'POST',
            body: JSON.stringify(params)
        });
    }
};

// 历史数据API
export const historyApi = {
    getHistory: (startTime, endTime, dataType = 'all') => {
        let url = '/history?';
        if (startTime) url += `start_time=${startTime}&`;
        if (endTime) url += `end_time=${endTime}&`;
        if (dataType) url += `data_type=${dataType}`;
        return apiRequest(url);
    }
};

// 系统配置API
export const configApi = {
    getSystemConfig: () => {
        // 模拟数据，后续实现真实API
        return Promise.resolve({
            system_name: "智能压电光催化控制平台",
            version: "v1.0.0",
            uptime: "356小时28分钟"
        });
    }
};