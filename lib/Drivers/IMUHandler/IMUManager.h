#pragma once
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include "MPU6050_6Axis_MotionApps612.h" // Uncomment this library to work with DMP 6.12 and comment on the above library.


/* OUTPUT FORMAT DEFINITION-------------------------------------------------------------------------------------------
- Use "OUTPUT_READABLE_QUATERNION" for quaternion commponents in [w, x, y, z] format. Quaternion does not 
suffer from gimbal lock problems but is harder to parse or process efficiently on a remote host or software 
environment like Processing.

- Use "OUTPUT_READABLE_EULER" for Euler angles (in degrees) output, calculated from the quaternions coming 
from the FIFO. EULER ANGLES SUFFER FROM GIMBAL LOCK PROBLEM.

- Use "OUTPUT_READABLE_YAWPITCHROLL" for yaw/pitch/roll angles (in degrees) calculated from the quaternions
coming from the FIFO. THIS REQUIRES GRAVITY VECTOR CALCULATION.
YAW/PITCH/ROLL ANGLES SUFFER FROM GIMBAL LOCK PROBLEM.

- Use "OUTPUT_READABLE_REALACCEL" for acceleration components with gravity removed. The accel reference frame
is not compensated for orientation. +X will always be +X according to the sensor.

- Use "OUTPUT_READABLE_WORLDACCEL" for acceleration components with gravity removed and adjusted for the world
reference frame. Yaw is relative if there is no magnetometer present.

-  Use "OUTPUT_TEAPOT" for output that matches the InvenSense teapot demo. 
-------------------------------------------------------------------------------------------------------------------------------*/ 
#define OUTPUT_READABLE_YAWPITCHROLL
//#define OUTPUT_READABLE_QUATERNION
//#define OUTPUT_READABLE_EULER
//#define OUTPUT_READABLE_REALACCEL
#define OUTPUT_READABLE_WORLDACCEL
//#define OUTPUT_TEAPOT

int const INTERRUPT_PIN = 5;  // Define the interruption #0 pin
bool blinkState;

struct IMUData {
    /*---MPU6050 Control/Status Variables---*/
    bool DMPReady = false;  // Set true if DMP init was successful
    uint8_t MPUIntStatus;   // Holds actual interrupt status byte from MPU
    uint8_t devStatus;      // Return status after each device operation (0 = success, !0 = error)
    uint16_t packetSize;    // Expected DMP packet size (default is 42 bytes)
    uint8_t FIFOBuffer[64]; // FIFO storage buffer

    /*---Orientation/Motion Variables---*/ 
    Quaternion q;           // [w, x, y, z]         Quaternion container
    VectorInt16 aa;         // [x, y, z]            Accel sensor measurements
    VectorInt16 gy;         // [x, y, z]            Gyro sensor measurements
    VectorInt16 aaReal;     // [x, y, z]            Gravity-free accel sensor measurements
    VectorInt16 aaWorld;    // [x, y, z]            World-frame accel sensor measurements
    VectorFloat gravity;    // [x, y, z]            Gravity vector
    float euler[3];         // [psi, theta, phi]    Euler angle container
    float ypr[3];           // [yaw, pitch, roll]   Yaw/Pitch/Roll container and gravity vector
    float totalLinearAcc;

    bool isDataNew;
};

class IMUManager {
public:
    IMUManager();
    
    bool init(int interruptPin);

    void update();

    const IMUData& getIMUData() const;


private:
    MPU6050 _mpu;
    IMUData _currentData;

    void parseDMPData();

};