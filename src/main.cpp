#include <Arduino.h>
#include <IMUHandler/IMUManager.h>
#include <FallDetector/FallDetector.h>
#include <StepCounter/StepCounter.h>
#include <EnvHandler/EnvManager.h>
#include <NetworkManager/NetworkManager.h>
#include <SystemData.h>

TaskHandle_t SensorTaskHandle = NULL;
TaskHandle_t NetworkTaskHandle = NULL;
SemaphoreHandle_t serialMutex = NULL; 

IMUManager imu;
FallDetector detector;
StepCounter stepCounter;
EnvManager env;
NetworkManager net;

// 封装一个安全的打印函数，防止多核冲突卡死 USB
void safePrint(const char* msg) {
    if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(50))) {
        Serial.println(msg);
        xSemaphoreGive(serialMutex);
    }
}

void SensorTask(void *pvParameters) {
    safePrint("[Task] SensorTask started on Core 0");
    for (;;) {
        imu.update();
        const IMUData& raw = imu.getIMUData();
        if (raw.isDataNew) {
            FallState fState = detector.update(raw.ypr[0], raw.ypr[1], raw.ypr[2], raw.totalLinearAcc);
            uint32_t steps = stepCounter.count(raw.aaWorld.z);
            
            // === 修改开始: 状态映射逻辑 ===
            // 库状态 CONFIRMED -> 系统状态 1 (已摔倒，观察中)
            // 库状态 CRITICAL  -> 系统状态 3 (最终确认，发送报警)
            int fLevel = 0;
            if (fState == FallState::CRITICAL) {
                fLevel = 3;
            } else if (fState == FallState::CONFIRMED) {
                fLevel = 1;
            }
            // === 修改结束 ===
            
            SystemData::getInstance()->setData(raw.ypr[0], raw.ypr[1], raw.ypr[2], raw.totalLinearAcc, fLevel, steps);
        }
        vTaskDelay(pdMS_TO_TICKS(10)); 
    }
}

void NetworkTask(void *pvParameters) {
    safePrint("[Task] NetworkTask started on Core 1");
    // 务必确认你的路由器 IP 和账号密码
    net.init("Wyz", "13903989778", "broker.emqx.io", 1883);
    unsigned long lastUpload = 0;
    unsigned long lastHeartbeat = 0;

    for (;;) {
        net.loop();
        env.update();
        SystemData::getInstance()->setEnvData(env.getTemperature(), env.getHumidity());

        unsigned long now = millis();

        if (now - lastHeartbeat > 2000) {
            SystemData::Data current = SystemData::getInstance()->getData();
            if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(50))) {
                Serial.printf("[Monitor] Steps:%u | P:%.1f | Fall:%d | T:%.1f H:%.1f | WiFi:%s\n", 
                               current.steps, current.ypr[1], current.fallLevel,
                               current.temperature, current.humidity,
                               net.isConnected() ? "OK" : "DISCONNECT");
                xSemaphoreGive(serialMutex);
            }
            lastHeartbeat = now;
        }

        if (net.isConnected()) {
            SystemData::Data current = SystemData::getInstance()->getData();
            
            // 只要有异常状态(1或3)或者是定时上传，都发送数据
            bool urgent = (current.fallLevel > 0);
            
            if (urgent || (now - lastUpload > 5000)) {
                JsonDocument doc; 
                doc["device_id"] = "DEVICE_001";
                doc["steps"] = current.steps;
                doc["fall"] = current.fallLevel;
                doc["accel_magnitude"] = current.totalAcc; // 保留浮点精度
                doc["temperature"] = current.temperature;  // 修改为全称
                doc["humidity"] = current.humidity;        // 修改为全称
                
                // 补全字段，避免后端警告
                doc["accel_x"] = 0.0;
                doc["accel_y"] = 0.0;
                doc["accel_z"] = 0.0;
                doc["tilt_angle"] = current.ypr[1];

                if (net.publishData("elderly/sensor/data", doc)) {
                    lastUpload = now;
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100)); 
    }
}

void setup() {
    Serial.begin(115200);
    delay(2500); // 增加延迟给 S3 识别 USB
    
    serialMutex = xSemaphoreCreateMutex();
    
    // 重要：先预热单例，防止多核同时创建
    SystemData::getInstance();
    
    // 初始化硬件前显式调用 Wire.begin 避开 PSRAM 引脚
    Wire.begin(8, 9); 
    env.init(6); // DHT11 接 GPIO 6
    if (!imu.init(5)) {
        Serial.println("IMU Init Failed!");
        while(1) delay(1000);
    }
    Serial.println("Hardware Ready.");

    xTaskCreatePinnedToCore(SensorTask, "SensorTask", 8192, NULL, 10, &SensorTaskHandle, 0);
    xTaskCreatePinnedToCore(NetworkTask, "NetworkTask", 10240, NULL, 5, &NetworkTaskHandle, 1);
}

void loop() {
    vTaskDelete(NULL); 
}
