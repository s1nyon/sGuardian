#pragma once
#include <Arduino.h>

enum class StepState {
    WAITING_FOR_PEAK,
    WAITING_FOR_VALLEY
};

class StepCounter {
public:
    StepCounter();

    uint32_t count(float accZ);

    float getFilteredZ() const { return _sum / WINDOW_SIZE; }
    float getThreshold() const { return _dynamicThreshold; }

private:
    
    uint32_t _steps;
    StepState _stepState;

     // --- 滤波器相关 ---
    static const int WINDOW_SIZE = 10; // 窗口大小，你可以根据 VOFA+ 观察结果调整（5-15）
    float _buffer[WINDOW_SIZE];        // 存放历史数据的环形队列
    int _index;                        // 当前写入的位置
    float _sum;                        // 当前窗口内所有数据的总和
    bool _isInitialized;               // 是否已完成第一次初始化
    float _currentMax;
    float _currentMin;

    float _activeMax;     // 当前生效的窗口最大值
    float _activeMin;     // 当前生效的窗口最小值
    float _tempMax;       // 正在探测中的窗口最大值
    float _tempMin;       // 正在探测中的窗口最小值
    int _sampleCount;     // 采样计数器
    const int RANGE_WINDOW = 50; // 窗口大小（假设采样率50Hz，50个点约为1秒，覆盖一个迈步周期）
    float _dynamicThreshold; // 动态阈值（中点）
    float _swing;            // 震荡幅度（Max - Min）

    
    float _movingAverageFilter(float accData);
    void _updateDynamicRange(float fData);

};