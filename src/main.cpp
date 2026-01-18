#include <Arduino.h>
#include <IMUHandler/IMUManager.h>  // 请确保路径与你 lib 文件夹下的匹配
#include <SystemData.h>             // 请确保路径与你 lib 文件夹下的匹配

// 实例化硬件管理器
IMUManager imu;

// 用于控制串口打印频率的计时器
unsigned long lastPrintTime = 0;
const unsigned long printInterval = 100; // 每 100ms 打印一次
int const INTERRUPT_PIN = 5;  // Define the interruption #0 pin

void setup() {
    // 1. 初始化串口
    Serial.begin(115200);
    delay(1000); // 等待串口稳定
    Serial.println(F("\n====================================="));
    Serial.println(F("System Starting..."));
    Serial.println(F("====================================="));

    // 2. 初始化 IMU 管理器 (使用 GPIO 5 作为中断引脚)
    // 注意：内部已包含 Wire.begin()
    if (!imu.init(INTERRUPT_PIN)) {
        Serial.println(F("IMU Manager Init Failed! Please check wiring."));
        while (1) {
            delay(1000);
        }
    }
    Serial.println(F("IMU Manager Init Success!"));
    Serial.println(F("Data Flow: IMUManager -> SystemData -> Serial Out"));
    Serial.println(F("-------------------------------------"));
}

void loop() {
    // 3. 更新硬件状态
    imu.update();

    // 4. 获取硬件层处理好的原始数据
    const IMUData& raw = imu.getIMUData();

    // 5. 如果硬件层产生了新数据，同步到全局数据中心 SystemData
    if (raw.isDataNew) {
        SystemData::getInstance()->setIMUData(
            raw.ypr[0],          // Yaw
            raw.ypr[1],          // Pitch
            raw.ypr[2],          // Roll
            raw.totalLinearAcc   // 合加速度
        );
    }

    // 6. 模拟消费端：每隔一段时间从 SystemData 中提取数据并展示
    if (millis() - lastPrintTime >= printInterval) {
        lastPrintTime = millis();

        // 从 SystemData 获取最新快照
        SystemData::Data current = SystemData::getInstance()->getData();

        // 格式化输出：YPR 角度和合加速度
        Serial.printf("Yaw:%7.2f | Pitch:%7.2f | Roll:%7.2f | Acc:%8.0f", 
                      current.ypr[0], 
                      current.ypr[1], 
                      current.ypr[2], 
                      current.totalAcc);

        // 预留的跌倒状态显示
        if (current.isFall) {
            Serial.print(F(" | STATUS: FALL DETECTED! ⚠️"));
        } else {
            Serial.print(F(" | STATUS: OK"));
        }
        
        Serial.println();
    }
}