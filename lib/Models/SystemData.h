#pragma once
#include <Arduino.h>

class SystemData {
public:
    static SystemData* getInstance() {
        static SystemData instance;
        return &instance;
    }

    struct Data {
        float totalAcc;
        int fallLevel;
        float ypr[3];
    };

    void setIMUData(float y, float p, float r, float acc) {
        _data.ypr[0] = y;
        _data.ypr[1] = p;
        _data.ypr[2] = r;
        _data.totalAcc = acc;
    }

    void setIsFall(int level) {
        _data.fallLevel = level;
    }

    Data getData() {
        return _data;
    }
    
    
private:
    SystemData() {} // 私有构造
    Data _data;
};

