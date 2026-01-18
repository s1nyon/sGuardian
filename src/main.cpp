#include <Arduino.h>
#include <IMUHandler/IMUManager.h>
#include <FallDetector/FallDetector.h>
#include <SystemData.h>

// --- 硬件配置 ---
#define SDA_PIN 8
#define SCL_PIN 9
#define INT_PIN 5
#define SERIAL_BAUD 115200

// --- 实例化对象 ---
IMUManager imu;
FallDetector detector;

// --- 计时器 ---
unsigned long lastPrintTime = 0;
const unsigned long printInterval = 100; // 0.1秒刷新一次

void setup() {
    Serial.begin(SERIAL_BAUD);
    while(!Serial); // 等待串口准备就绪

    Serial.println(F("\n[System] Elderly Care System Initializing..."));

    if (!imu.init(INT_PIN)) {
        Serial.println(F("[Error] IMU Initialization Failed!"));
        while (1) delay(100);
    }

    Serial.println(F("[Success] System Ready. Monitoring Behavior..."));
}

void loop() {
    imu.update();
    const IMUData& raw = imu.getIMUData();

    if (raw.isDataNew) {
        // 1. 获取算法结果
        FallState result = detector.update(raw.ypr[0], raw.ypr[1], raw.ypr[2], raw.totalLinearAcc);
        
        // 2. 映射 Level
        int level = 0;
        if (result == FallState::CONFIRMED) level = 1;
        if (result == FallState::CRITICAL)  level = 2;

        // 3. 一次性更新所有数据到 SystemData
        // 注意：建议给 setIMUData 增加一个 level 参数，而不是分两次写
        SystemData::getInstance()->setIMUData(
            raw.ypr[0], raw.ypr[1], raw.ypr[2], raw.totalLinearAcc
        );
        SystemData::getInstance()->setIsFall(level); // 这里的 level 会影响下面的打印
    }

    // 4. 定时打印输出
    if (millis() - lastPrintTime >= printInterval) {
        lastPrintTime = millis();
        SystemData::Data current = SystemData::getInstance()->getData();

        // 增加了一个状态展示
        const char* statusStr = "OK";
        if (current.fallLevel == 1) statusStr = "FALLING! ⚠️";
        if (current.fallLevel == 2) statusStr = "!!! CRITICAL !!! 🚨";

        Serial.printf("P:%6.1f R:%6.1f Acc:%6.0f | Status: %s\n", 
                      current.ypr[1], current.ypr[2], current.totalAcc, statusStr);
    }
}