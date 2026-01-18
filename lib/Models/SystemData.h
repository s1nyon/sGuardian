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
        bool isFall;
        float ybr[3];
    };

    void setIMUData() {
        
    }

    
    
private:
    SystemData() {} // 私有构造
    Data _data;
};

