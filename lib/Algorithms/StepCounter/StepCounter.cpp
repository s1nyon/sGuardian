#include <StepCounter/StepCounter.h>

StepCounter::StepCounter() : 
    _steps(0), 
    _stepState(StepState::WAITING_FOR_PEAK),
    _index(0),
    _sum(0.0f),
    _isInitialized(false) 
{
    _tempMax = -99999.0f;
    _tempMin = 99999.0f;
    _dynamicThreshold = 0.0f;
    _sampleCount = 0;
    _swing = 0.0f;

    for (int i = 0; i < WINDOW_SIZE; i++) {
        _buffer[i] = 0.0f;
    }
}

float StepCounter::_movingAverageFilter(float accData) {
    if (!_isInitialized) {
        for (int i = 0; i < WINDOW_SIZE; i++) {
            _buffer[i] = accData;
        }
        _sum = accData * WINDOW_SIZE;
        _isInitialized = true;
        return accData;
    }
    _sum -= _buffer[_index];
    _buffer[_index] = accData;
    _sum += _buffer[_index];
    _index = (_index + 1) % WINDOW_SIZE;

    return _sum / WINDOW_SIZE;
}

void StepCounter::_updateDynamicRange(float fData) {
    // 1. 在当前搜索窗口内，不断更新临时最大/最小值
    if (fData > _tempMax) _tempMax = fData;
    if (fData < _tempMin) _tempMin = fData;

    _sampleCount++;

    // 2. 当窗口走满（例如 1 秒钟）
    if (_sampleCount >= RANGE_WINDOW) {
        // 将这一秒内发现的极值交给“生效变量”
        _activeMax = _tempMax;
        _activeMin = _tempMin;
        
        // 计算这一秒内的动态中点（阈值）
        _dynamicThreshold = (_activeMax + _activeMin) / 2.0f;
        
        // 计算这一秒内的震荡幅度
        _swing = _activeMax - _activeMin;

        // 3. 重置探测变量，为下一秒做准备
        // 注意：重置时不能设为0，而应设为当前值，否则会导致下一秒开头数据跳变
        _tempMax = fData;
        _tempMin = fData;
        _sampleCount = 0;
    }
}

uint32_t StepCounter::count(float accZ) {
    // 1. 基础滤波（保持波形平滑）
    float fData = _movingAverageFilter(accZ);

    // 2. 更新动态范围（每秒自动调整一次基准线）
    _updateDynamicRange(fData);

    // 3. 计步状态机
    if (_stepState == StepState::WAITING_FOR_PEAK) {
        // 判定条件：
        // A. 超过动态中点
        // B. 波动幅度大于 2000 (根据你 VOFA+ 观察到的差值定，防止静止噪声)
        if (fData > _dynamicThreshold && _swing > 2000) {
            _steps++;
            _stepState = StepState::WAITING_FOR_VALLEY;
        }
    } else {
        // 回落判定：跌破动态中点
        if (fData < _dynamicThreshold) {
            _stepState = StepState::WAITING_FOR_PEAK;
        }
    }
    
    return _steps;
}
