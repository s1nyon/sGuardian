#pragma once
#include <Arduino.h>

class SystemData {
public:
    static SystemData* getInstance() {
        static SystemData instance;
        return &instance;
    }

    
    
private:
    SystemData() {} // 私有构造
};

