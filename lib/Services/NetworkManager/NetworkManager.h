#pragma once
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

class NetworkManager {
public:
    NetworkManager();
    void init(const char* ssid, const char* password, const char* mqttServer, int mqttPort);
    
    void loop(); 
    
    bool publishData(const char* topic, const JsonDocument& doc);
    bool isConnected();

private:
    const char* _ssid;
    const char* _password;
    WiFiClient _espClient;
    PubSubClient _mqttClient;
    
    unsigned long _lastReconnectAttempt = 0;
    void _connectWiFi();
    bool _connectMQTT();
};