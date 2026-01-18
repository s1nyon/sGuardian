#include <IMUHandler/IMUManager.h>

IMUManager::IMUManager() {
    _currentData = {}; 
}

bool IMUManager::init(int interruptPin) {
    Wire.begin();
    Wire.setClock(400000);

    _mpu.initialize();
    pinMode(interruptPin, INPUT);
    if(!_mpu.testConnection()) return false;

    _currentData.devStatus = _mpu.dmpInitialize();

    /* Supply your gyro offsets here, scaled for min sensitivity */
    _mpu.setXGyroOffset(0);
    _mpu.setYGyroOffset(0);
    _mpu.setZGyroOffset(0);
    _mpu.setXAccelOffset(0);
    _mpu.setYAccelOffset(0);
    _mpu.setZAccelOffset(0);

    if(_currentData.devStatus == 0) {
        _mpu.setDMPEnabled(true);
        _currentData.packetSize = _mpu.dmpGetFIFOPacketSize();
        return true;
    }
    else {
        Serial.printf("DMP Init Error: %d\n", _currentData.devStatus);
        return false;
    }
}

void IMUManager::update() {
    _currentData.isDataNew = false;

    if(_mpu.dmpGetCurrentFIFOPacket(_currentData.FIFOBuffer)) {
        parseDMPData();
        _currentData.isDataNew = true;
    }
}

void IMUManager::parseDMPData() {
    Quaternion q;
    VectorInt16 aa;
    VectorInt16 aaReal;
    VectorInt16 aaWorld;
    VectorFloat gravity;
    float ypr[3];

    // 1. 解析姿态角
    _mpu.dmpGetQuaternion(&q, _currentData.FIFOBuffer);
    _mpu.dmpGetGravity(&gravity, &q);
    _mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
    
    _currentData.ypr[0] = ypr[0] * 180 / M_PI;
    _currentData.ypr[1] = ypr[1] * 180 / M_PI;
    _currentData.ypr[2] = ypr[2] * 180 / M_PI;

    // 2. 解析世界坐标系加速度 (排除重力，且不随传感器翻转而改变轴向)
    _mpu.dmpGetAccel(&aa, _currentData.FIFOBuffer);
    _mpu.dmpGetLinearAccel(&aaReal, &aa, &gravity);
    _mpu.dmpGetLinearAccelInWorld(&aaWorld, &aaReal, &q);

    _currentData.aaWorld.x = aaWorld.x;
    _currentData.aaWorld.y = aaWorld.y;
    _currentData.aaWorld.z = aaWorld.z;

    // 3. 计算合加速度模长 (跌倒检测最重要的原始指标)
    _currentData.totalLinearAcc = sqrt(pow(aaWorld.x, 2) + pow(aaWorld.y, 2) + pow(aaWorld.z, 2));
}

const IMUData& IMUManager::getIMUData() const {
    return _currentData;
}