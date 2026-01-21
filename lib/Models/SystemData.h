#pragma once
#include <Arduino.h>
#include <freertos/semphr.h>

class SystemData {
public:
    static SystemData* getInstance() {
        static SystemData instance;
        return &instance;
    }

    struct Data {
        float ypr[3];
        float totalAcc;
        int fallLevel;
        uint32_t steps;
    };

    void setData(float y, float p, float r, float acc, int fall, uint32_t steps) {
        // 将 portMAX_DELAY 改为短时间等待，防止死锁
        if (xSemaphoreTake(_mutex, pdMS_TO_TICKS(5))) {
            _data.ypr[0] = y; _data.ypr[1] = p; _data.ypr[2] = r;
            _data.totalAcc = acc;
            _data.fallLevel = fall;
            _data.steps = steps;
            xSemaphoreGive(_mutex);
        }
    }

    Data getData() {
        Data temp = {0};
        if (xSemaphoreTake(_mutex, pdMS_TO_TICKS(5))) {
            temp = _data;
            xSemaphoreGive(_mutex);
        }
        return temp;
    }

private:
    SystemData() { _mutex = xSemaphoreCreateMutex(); }
    Data _data;
    SemaphoreHandle_t _mutex;
};