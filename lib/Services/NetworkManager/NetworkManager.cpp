#include <NetworkManager/NetworkManager.h>

NetworkManager::NetworkManager() : _mqttClient(_espClient) {}

void NetworkManager::init(const char* ssid, const char* password, const char* mqttServer, int mqttPort) {
    _ssid = ssid; _password = password;
    _mqttClient.setServer(mqttServer, mqttPort);
    _connectWiFi();
}

void NetworkManager::_connectWiFi() {
    // 只有在完全没连接且没在连接中的时候才发起请求
    if (WiFi.status() != WL_CONNECTED && WiFi.status() != WL_DISCONNECTED) {
        Serial.println("[Net] Starting WiFi connection...");
        WiFi.begin(_ssid, _password);
    }
}

bool NetworkManager::_connectMQTT() {
    String clientId = "ElderlyS3-" + String(random(0xffff), HEX);
    if (_mqttClient.connect(clientId.c_str())) {
        Serial.println("[Net] MQTT Connected");
        return true;
    }
    return false;
}

void NetworkManager::loop() {
    // 1. 检查 WiFi 状态
    if (WiFi.status() != WL_CONNECTED) {
        static unsigned long lastWiFiCheck = 0;
        if (millis() - lastWiFiCheck > 10000) { // 每 10 秒尝试重连一次
            lastWiFiCheck = millis();
            Serial.println("[Net] WiFi not connected, retrying...");
            // 重新调用 begin 即可，不需要判断复杂的底层原因
            WiFi.begin(_ssid, _password);
        }
        return; 
    }

    // 2. 打印 IP（仅在首次连上时打印）
    static bool ipPrinted = false;
    if (!ipPrinted) {
        Serial.print("[Net] WiFi Connected! IP: ");
        Serial.println(WiFi.localIP());
        ipPrinted = true;
    }

    // 3. 检查 MQTT 状态
    if (!_mqttClient.connected()) {
        unsigned long now = millis();
        if (now - _lastReconnectAttempt > 5000) {
            _lastReconnectAttempt = now;
            if (_connectMQTT()) {
                _lastReconnectAttempt = 0;
            }
        }
    } else {
        _mqttClient.loop();
    }
}

bool NetworkManager::publishData(const char* topic, const JsonDocument& doc) {
    if (!_mqttClient.connected()) return false;
    char buffer[256];
    serializeJson(doc, buffer);
    return _mqttClient.publish(topic, buffer);
}

bool NetworkManager::isConnected() {
    return WiFi.status() == WL_CONNECTED && _mqttClient.connected();
}