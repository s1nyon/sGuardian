#include <FallDetector/FallDetector.h>

FallDetector::FallDetector() : _state(FallState::IDLE), _timer(0) {}

FallState FallDetector::update(float yaw, float pitch, float roll, float acc) {
    unsigned long now = millis();

    switch (_state) {
        case FallState::IDLE:
            if (acc > IMPACT_THRESHOLD) {
                // 优化：进入新状态时才打印，防止瞬间刷屏卡死串口
                Serial.printf("\n[Alg] >>> IMPACT! (Acc:%.0f) Waiting 2s for stability...\n", acc);
                _state = FallState::POTENTIAL_IMPACT;
                _timer = now;
            }
            break;

        case FallState::POTENTIAL_IMPACT:
            // 2秒观察期内不执行逻辑，只等待
            if (now - _timer > STABLE_WINDOW) {
                if (abs(pitch) < LAY_THRESHOLD || abs(roll) < LAY_THRESHOLD) {
                    _state = FallState::CONFIRMED;
                    _timer = now; // 重新计时，用于计算 10s 静止
                    Serial.printf("[Alg] >>> FALL CONFIRMED! (P:%.1f, R:%.1f)\n", pitch, roll);
                } else {
                    _state = FallState::IDLE;
                    Serial.println("[Alg] False Alarm: Recovered quickly.");
                }
            }
            break;

        case FallState::CONFIRMED:
            // 每一秒打印一次倒计时
            {
                static unsigned long lastCountPrint = 0;
                if (now - lastCountPrint > 1000) {
                    int timeLeft = (CRITICAL_WINDOW - (now - _timer)) / 1000;
                    if (timeLeft >= 0) Serial.printf("[Alg] Waiting for Critical... %ds\n", timeLeft);
                    lastCountPrint = now;
                }
            }

            // 优化：不仅判断接近0度，也要判断是否真的“站立”
            // 如果你发现传感器经常翻转，可以调宽判定范围
            if (abs(pitch) > 30.0f || abs(roll) > 30.0f) {
                _state = FallState::IDLE;
                Serial.println("[Alg] User stood up. Clear.");
            }
            // 增加：如果 acc 有明显的连续波动（说明人在动，不论姿态），也应退出
            else if (acc > 12000) { 
                _state = FallState::IDLE;
                Serial.println("[Alg] Significant movement detected. Clear.");
            } 
            else if (now - _timer > CRITICAL_WINDOW) {
                _state = FallState::CRITICAL;
                Serial.println("[Alg] >>> !!! CRITICAL !!! <<<");
            }
            break;

        case FallState::CRITICAL:
            // 处于此状态时，不再进行自动重置，等待外部干预
            break;
    }
    return _state;
}