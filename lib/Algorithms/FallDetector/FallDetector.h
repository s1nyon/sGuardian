#pragma once
#include <Arduino.h>

enum class FallState {
    IDLE,
    POTENTIAL_IMPACT,
    CONFIRMED,
    CRITICAL
};

class FallDetector {
public: 
    FallDetector();

    FallState update(float yaw, float pitch, float roll, float acc);

    void reset();

private:
    FallState _state;
    unsigned long _timer;

    // 算法阈值
    const float IMPACT_THRESHOLD = 15000.0f; // 撞击阈值 (LSB单位)
    const float LAY_THRESHOLD = 20.0f;       // 躺平角度阈值 (度)
    const uint32_t STABLE_WINDOW = 4000;     // 撞击后观察 2s 是否静止
    const uint32_t CRITICAL_WINDOW = 10000;  // 10s 不动触发严重报警


};
