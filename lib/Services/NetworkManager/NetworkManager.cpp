#include <NetworkManager/NetworkManager.h>

NetworkManager::NetworkManager() : _mqttClient(_espClient) {}

void NetworkManager::init(const char* ssid, const char* password, const char* mqttServer, int mqttPort) {
    _ssid = ssid; _password = password;
    _mqttClient.setServer(mqttServer, mqttPort);
    _connectWiFi();
}

void NetworkManager::_connectWiFi() {
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
    if (WiFi.status() != WL_CONNECTED) {
        static unsigned long lastWiFiCheck = 0;
        if (millis() - lastWiFiCheck > 10000) { // 每 10 秒尝试重连一次
            lastWiFiCheck = millis();
            Serial.println("[Net] WiFi not connected, retrying...");
            WiFi.begin(_ssid, _password);
        }
        return; 
    }

    static bool ipPrinted = false;
    if (!ipPrinted) {
        Serial.print("[Net] WiFi Connected! IP: ");
        Serial.println(WiFi.localIP());
        ipPrinted = true;
    }

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