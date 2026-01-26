#include <EnvHandler/EnvManager.h>

EnvManager::EnvManager() : _dht(nullptr), _temp(0.0f), _humi(0.0f), _lastReadTime(0) {}

void EnvManager::init(int dhtPin) {
    // 动态创建 DHT 实例，指定类型为 DHT11
    _dht = new DHT(dhtPin, DHT11);
    _dht->begin();
    Serial.println("[Env] DHT11 initialized.");
}

void EnvManager::update() {
    unsigned long now = millis();
    
    // 只有距离上次读取超过 2.5 秒时才执行读取动作
    if (now - _lastReadTime >= READ_INTERVAL) {
        _lastReadTime = now;

        // 执行硬件读取
        float h = _dht->readHumidity();
        float t = _dht->readTemperature();

        // 检查读取结果是否有效 (isnan 是标准库函数，检查是否为“非数字”)
        if (isnan(h) || isnan(t)) {
            Serial.println("[Env] Failed to read from DHT sensor!");
            return; 
        }

        // 只有数据有效时才更新缓存
        _temp = t;
        _humi = h;
    }
}

float EnvManager::getTemperature() {
    return _temp;
}

float EnvManager::getHumidity() {
    return _humi;
}