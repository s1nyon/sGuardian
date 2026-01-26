#pragma once
#include <DHT.h>

class EnvManager {
public:
    EnvManager();
    void init(int dhtPin);
    void update(); 
    
    float getTemperature();
    float getHumidity();

private:
    DHT* _dht;
    float _temp;
    float _humi;
    unsigned long _lastReadTime;
    const unsigned long READ_INTERVAL = 2500; // DHT11 建议读取间隔不低于 2s
};