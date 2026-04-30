# CardioGuard Band 💗

> **Wearable SpO₂, Heart Rate & Rule-Based Cardiac Risk Classifier**  
> Built as part of **ME696 at IIT Guwahati** 🎓

---

## 🚀 Overview

CardioGuard is a real-time wearable health monitoring system that measures vital physiological signals and estimates cardiac risk using an embedded rule-based classifier — derived from a Decision Tree trained on a structured CSV dataset. The system integrates multiple sensors with an ESP32 microcontroller to continuously monitor Heart Rate (HR), SpO₂, and motion data, and provides an interpretable risk indication on-device.

---

## 🔍 What It Does

- Measures **Heart Rate (HR)** and **SpO₂** using MAX30102 sensor
- Tracks motion and activity using MPU6050 (accelerometer + gyroscope)
- Processes data in real-time on ESP32
- Displays output on 0.96" OLED screen
- Classifies **cardiac risk level → LOW / MODERATE / HIGH**
- Detects **health status → HEALTHY / SICK / DISEASE**
- Syncs live data to **Blynk IoT dashboard**

---

## 🧠 AI / ML Approach

### Workflow
1. Collected a structured CSV dataset with features: HR (BPM), SpO₂ (%), Activity level
2. Trained a **Decision Tree Classifier** (Python / scikit-learn) on this dataset offline
3. Extracted the key decision thresholds from the trained tree
4. **Deployed those thresholds as a rule-based classifier on ESP32** (embedded C++)

### Why this approach?
- ESP32 cannot run a full ML runtime — threshold extraction is the standard embedded ML deployment method
- Decision Trees naturally produce interpretable if/else rules → perfect for edge deployment
- Output is deterministic and real-time with zero cloud dependency

### Model Logic (derived from Decision Tree)

| Condition | Risk Level |
|---|---|
| SpO₂ < 90% OR HR > 110 bpm | 🔴 HIGH |
| SpO₂ < 95% OR HR > 95 bpm | 🟡 MODERATE |
| SpO₂ ≥ 95% AND HR 60–95 bpm | 🟢 LOW |

> **Note:** This system is designed for **early risk indication** only — not clinical diagnosis.

---

## ⚙️ Tech Stack

`ESP32` | `Embedded C++` | `I2C Sensors` | `OLED UI` | `Blynk IoT` | `scikit-learn (offline training)` | `Python (model training)`

---

## 🛠️ Hardware Components

| Component | Function |
|---|---|
| ESP32 | Microcontroller + WiFi |
| MAX30102 | Pulse Oximeter (HR + SpO₂) |
| MPU6050 | Accelerometer + Gyroscope (motion) |
| 0.96" OLED (I2C) | Real-time display |

---

## 🔌 Circuit Connections

| Component | Pin | ESP32 Pin |
|---|---|---|
| MAX30102 | VCC | 3.3V |
| | GND | GND |
| | SDA | GPIO 21 |
| | SCL | GPIO 22 |
| MPU6050 | VCC | 3.3V |
| | GND | GND |
| | SDA | GPIO 21 |
| | SCL | GPIO 22 |
| OLED Display | VCC | 3.3V |
| | GND | GND |
| | SDA | GPIO 21 |
| | SCL | GPIO 22 |

> All devices share I2C bus (SDA + SCL lines)

---

## ⚙️ System Workflow

```
Sensors → ESP32 preprocessing → Feature extraction → Rule-based classifier → OLED display + Blynk sync
```

1. MAX30102 & MPU6050 acquire physiological + motion data
2. ESP32 filters and smooths readings (moving average)
3. Peak detection extracts BPM; ratio method estimates SpO₂
4. Rule-based classifier (from trained Decision Tree) predicts risk level
5. Output shown on OLED and sent to Blynk dashboard

---

## 📈 Signal Processing Highlights

- Moving average smoothing for BPM and SpO₂ (reduces motion artifacts)
- Peak detection algorithm for accurate heart rate estimation from IR signal
- Buffered SpO₂ computation using AC/DC ratio method (Beer-Lambert law)
- MPU6050 activity detection to flag motion-artifact conditions
- Step counter via accelerometer magnitude thresholding

---

## 🔧 Setup Instructions

1. Clone this repo
2. Create a `secrets.h` file in the same folder as `CardioGuard.ino`:
```cpp
#define WIFI_SSID "your_wifi_name"
#define WIFI_PASS "your_password"
#define BLYNK_AUTH "your_blynk_token"
```
3. Install required Arduino libraries:
   - `MAX30105` (SparkFun)
   - `MPU6050_light`
   - `Adafruit SSD1306`
   - `BlynkSimpleEsp32`
4. Flash to ESP32 via Arduino IDE
5. Open Blynk app and configure virtual pins V0–V10

---

## 📌 Future Improvements

- HRV (Heart Rate Variability) integration
- Larger, validated dataset for improved classifier accuracy
- Cloud-based monitoring and trend analysis
- Deep learning model (TensorFlow Lite) for on-device inference
- PCB design for compact wearable form factor

---

## 🧪 Key Engineering Highlights

- Real-time signal processing on resource-constrained microcontroller
- End-to-end hardware + software integration
- Edge AI — no cloud compute needed for inference
- Multi-sensor fusion (HR + SpO₂ + motion)
- Low-cost design suitable for remote/underserved healthcare settings

---

## ⚠️ Disclaimer

This system is a **prototype for early risk indication and research purposes only**. It is not a substitute for professional medical diagnosis.

---

## 📄 License

MIT License — free to use, modify, and distribute with attribution.
