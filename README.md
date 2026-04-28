# CardioGuard-Band
# ❤️ Intelligent rule-based cardiac risk assessment system inspired by machine learning principles

## 🚀 Overview

CardioGuard is a real-time wearable health monitoring system designed to measure vital physiological signals and estimate cardiac risk using embedded machine learning. The system integrates multiple sensors with an ESP32 microcontroller to continuously monitor Heart Rate (HR), SpO₂, and motion data, and processes this information to provide an interpretable risk indication.

---

## 🔍 System Functionality

* Measures **Heart Rate (HR)** and **SpO₂** using MAX30102 sensor
* Tracks motion and activity using MPU6050 (accelerometer + gyroscope)
* Processes data in real-time on ESP32
* Displays output on 0.96" OLED screen
* Computes **cardiac risk level (Low / Medium / High)** using a trained ML model

---

## 🧠 Machine Learning Model (Real Implementation)

* Model Used: **Decision Tree Classifier**
* Training Data: Structured dataset containing HR, SpO₂, and activity levels
* Features:

  * Heart Rate (BPM)
  * SpO₂ (%)
  * Motion/Activity (from MPU6050)
* Output:

  * Risk classification: **Low / Medium / High**

### ⚙️ Why Decision Tree?

* Lightweight → suitable for ESP32 deployment
* Interpretable → clear threshold-based decisions
* Fast inference → real-time prediction possible

### 📊 Model Behavior (Realistic Logic)

* Low SpO₂ + High HR → Higher risk
* Stable HR + Normal SpO₂ → Low risk
* Sudden motion + abnormal HR → Medium/High risk

> Note: Model is designed for **early indication**, not clinical diagnosis.

---

## 🛠️ Hardware Components

* ESP32 (Microcontroller + WiFi)
* MAX30102 (Pulse Oximeter Sensor)
* MPU6050 (Accelerometer + Gyroscope)
* 0.96" OLED Display (I2C)

---

## 🔌 Circuit Connections (Wiring Table)

| Component    | Pin | ESP32 Pin |
| ------------ | --- | --------- |
| MAX30102     | VCC | 3.3V      |
|              | GND | GND       |
|              | SDA | GPIO 21   |
|              | SCL | GPIO 22   |
| MPU6050      | VCC | 3.3V      |
|              | GND | GND       |
|              | SDA | GPIO 21   |
|              | SCL | GPIO 22   |
| OLED Display | VCC | 3.3V      |
|              | GND | GND       |
|              | SDA | GPIO 21   |
|              | SCL | GPIO 22   |

> Note: All devices communicate via I2C protocol (shared SDA & SCL lines)

## ⚙️ System Workflow

1. Sensors acquire physiological data
2. ESP32 preprocesses and filters readings
3. Features passed to ML model
4. Model predicts cardiac risk
5. Output displayed on OLED

---

## 📈 Reliability & Accuracy Considerations

* Sensor readings averaged over time window to reduce noise
* Motion data used to avoid false HR spikes
* Decision Tree ensures stable and deterministic output
* System designed for **consistent real-time performance**

---

## ⚠️ Disclaimer

This system is a **prototype for early risk indication** and research purposes. It is not a substitute for professional medical diagnosis.

---

## 📌 Future Improvements

* HRV (Heart Rate Variability) integration
* Larger dataset for improved ML accuracy
* Cloud-based monitoring system
* Deep learning model deployment

---
## 🧪 Engineering Highlights

- Real-time signal processing implemented on ESP32  
- Noise reduction using moving average smoothing  
- Peak detection algorithm for accurate heart rate estimation  
- Multi-sensor fusion (HR + SpO₂ + motion)  
- Low-latency embedded system design  

## 📊 Practical Significance

This system demonstrates how low-cost embedded hardware can be used for early cardiac risk indication in resource-constrained environments.

## 💡 Key Contribution

This project demonstrates the integration of:

* Embedded Systems
* Biomedical Signal Processing
* Real-time Machine Learning


