# sGuardian - 智能老人守护系统 (Smart Elderly Guardian)

sGuardian 是一个基于 ESP32-S3 和 MPU6050 的智能穿戴设备固件项目，旨在为独居老人提供实时的跌倒检测、运动监测以及云端报警功能。

## 🚀 主要功能

*   **跌倒检测 (Fall Detection)**: 采用基于状态机的智能算法，结合加速度冲击检测和静止姿态分析，能够有效识别意外跌倒事件，并区分误触。
    *   支持检测冲击、倒地静止以及后续的恢复/长时间未起状态。
*   **计步器 (Step Counter)**: 内置计步算法，实时记录用户的日常步数。
*   **实时云端上报**: 通过 WiFi 连接 MQTT 服务器，实时上传设备状态、步数和报警信息。
*   **多任务实时系统**: 基于 FreeRTOS 的双核架构设计（Core 0 处理传感器算法，Core 1 处理网络通信），确保高并发下的系统稳定性与响应速度。

## 🛠️ 硬件准备

*   **主控开发板**: ESP32-S3 DevKitC-1 (或其他兼容的 ESP32-S3 开发板)
*   **传感器模块**: MPU6050 (6轴加速度计 + 陀螺仪)
*   **其他**: 杜邦线、电源线

### 🔌 硬件接线说明

请按照下表将 MPU6050 连接至 ESP32-S3：

| MPU6050 引脚 | ESP32-S3 引脚 | 说明 |
| :--- | :--- | :--- |
| VCC | 3.3V | 电源正极 |
| GND | GND | 电源地 |
| **SDA** | **GPIO 8** | I2C 数据线 |
| **SCL** | **GPIO 9** | I2C 时钟线 |
| **INT** | **GPIO 5** | 中断引脚 (用于 DMP 数据就绪通知) |

> **注意**: 接线定义位于 `src/main.cpp` 的 `setup()` 函数中 (`Wire.begin(8, 9)`) 以及 `imu.init(5)`。

## 💻 软件环境与依赖

本项目使用 **PlatformIO** 进行开发和管理。

*   **平台**: Espressif 32 (`espressif32`)
*   **框架**: Arduino
*   **依赖库** (会自动安装):
    *   `electroniccats/MPU6050`
    *   `knolleary/PubSubClient`
    *   `bblanchon/ArduinoJson`

## ⚙️ 快速开始

### 1. 克隆项目
```bash
git clone <repository_url>
cd sGuardian
```

### 2. 配置网络
打开 `src/main.cpp` 文件，找到 `NetworkTask` 函数，修改 WiFi 和 MQTT 服务器配置：

```cpp
// src/main.cpp Line 43
// ⚠️ 请务必修改为您自己的 WiFi SSID 和密码
net.init("Your_SSID", "Your_Password", "broker.emqx.io", 1883);
```

### 3. 编译与上传
1.  使用 VS Code 打开项目文件夹。
2.  等待 PlatformIO 初始化并下载依赖。
3.  将开发板通过 USB 连接至电脑。
4.  点击底部状态栏的 **Build** (✓) 编译项目。
5.  点击 **Upload** (→) 将固件烧录至开发板。

> **提示**: 代码中在 `setup()` 初增加了 `delay(2500)`，以便 ESP32-S3 的 USB 串口有足够时间初始化，复位后请耐心等待几秒。

### 4. 数据监控
*   **串口监视器**: 打开 Serial Monitor (波特率 115200)，可以看到实时的传感器数据、步数和网络状态日志。
*   **MQTT 客户端**: 使用 MQTTX 或其他工具订阅 Topic，查看上报数据。

## 📊 数据协议 (MQTT)

设备会向 MQTT Broker 发布 JSON 格式的数据。

*   **Topic**: `elderly/sensor/data`
*   **发布频率**: 
    *   正常状态：每 5 秒一次
    *   跌倒/紧急状态：立即触发

**Payload 示例**:

```json
{
  "device_id": "ESP32_001",
  "steps": 1245,      // 当前步数
  "fall": 0,          // 跌倒状态码: 0=正常, 1=确认跌倒, 2=严重(长时间未起)
  "acc": 9800         // 瞬时总加速度值 (LSB)
}
```

## 📂 项目结构

```text
├── lib/
│   ├── Algorithms/     # 核心算法
│   │   ├── FallDetector/ # 跌倒检测状态机
│   │   └── StepCounter/  # 计步算法
│   ├── Drivers/        # 硬件驱动
│   │   └── IMUHandler/   # MPU6050 DMP 封装管理
│   ├── Models/         # 数据模型
│   │   └── SystemData.h  # 线程安全的单例数据存储
│   └── Services/       # 系统服务
│       └── NetworkManager/ # WiFi & MQTT 管理
├── src/
│   ├── main.cpp        # 程序入口，FreeRTOS 任务定义
│   └── GlobalConfig.h  # 全局配置
└── platformio.ini      # PlatformIO 配置文件
```

## 📝 许可证

MIT License
