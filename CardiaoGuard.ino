#define BLYNK_TEMPLATE_ID   "TMPL3tnG4FwVm"
#define BLYNK_TEMPLATE_NAME "CardioGuard"
#define BLYNK_AUTH_TOKEN    "CvfBWH76odHvmWy9NB_1gjsaQIs5lrrh"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <MAX30105.h>
#include <MPU6050_light.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

char ssid[] = "realme";
char pass[] = "12345678";

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define VPIN_HR            V0
#define VPIN_SPO2          V1
#define VPIN_RISK          V2
#define VPIN_ALERT         V4
#define VPIN_AI_SCORE      V5
#define VPIN_STEPS         V9
#define VPIN_HEALTH_STATUS V10

MAX30105 sensor;
MPU6050 mpu(Wire);
BlynkTimer timer;

// ── Buffers ──────────────────────────────────────────────────
#define BUFFER_SIZE 100
long irBuffer[BUFFER_SIZE];
long redBuffer[BUFFER_SIZE];
int  bufferIndex = 0;
bool bufferFull  = false;

// ── Vitals ───────────────────────────────────────────────────
float bpm  = 0;
float spo2 = 0;
bool  fingerDetected = false;

// ── Peak Detection ───────────────────────────────────────────
long  irPrev  = 0;
long  irPrev2 = 0;
unsigned long lastPeakTime   = 0;
unsigned long peakIntervals[8];
int   peakIdx = 0;

// ── Smoothing ────────────────────────────────────────────────
float smoothBPM  = 72.0;
float smoothSpO2 = 98.0;

// ── Steps ────────────────────────────────────────────────────
int   stepCount   = 0;
float lastAccel   = 0;
unsigned long lastStepTime = 0;

// ═════════════════════════════════════════════════════════════
void displayMessage(String l1, String l2, String l3) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);  display.println(l1);
  display.setCursor(0, 20); display.println(l2);
  display.setCursor(0, 40); display.println(l3);
  display.display();
}

void displayData(int hr, int sp, String riskStr, String health) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(22, 0);
  display.println("CardioGuard");
  display.drawLine(0, 10, 127, 10, WHITE);

  display.setCursor(0, 14);
  display.print("HR  : "); display.print(hr); display.println(" bpm");

  display.setCursor(0, 26);
  display.print("SpO2: "); display.print(sp); display.println(" %");

  display.setCursor(0, 38);
  display.print("Risk: "); display.println(riskStr);

  display.setCursor(0, 50);
  display.print("Hlth: "); display.println(health);

  display.display();
}

// ═════════════════════════════════════════════════════════════
int predictRisk(float pulse, float sp) {
  if (sp < 90 || pulse > 110) return 2;
  if (sp < 95 || pulse > 95)  return 1;
  return 0;
}

// ═════════════════════════════════════════════════════════════
float computeHR(long irValue) {
  long delta1 = irValue - irPrev;
  long delta2 = irPrev  - irPrev2;

  if (delta2 > 0 && delta1 < 0 && irPrev > 60000) {
    unsigned long now      = millis();
    unsigned long interval = now - lastPeakTime;

    if (interval > 350 && interval < 1500) {
      peakIntervals[peakIdx % 8] = interval;
      peakIdx++;
      lastPeakTime = now;

      int count = min(peakIdx, 8);
      unsigned long sum = 0;
      for (int i = 0; i < count; i++) sum += peakIntervals[i];
      float avgInterval = (float)sum / count;
      return 60000.0f / avgInterval;
    }
    lastPeakTime = now;
  }

  irPrev2 = irPrev;
  irPrev  = irValue;
  return -1;
}

// ═════════════════════════════════════════════════════════════
void computeSpO2() {
  if (!bufferFull && bufferIndex < BUFFER_SIZE) return;

  long irMax  = irBuffer[0],  irMin  = irBuffer[0];
  long redMax = redBuffer[0], redMin = redBuffer[0];

  int len = bufferFull ? BUFFER_SIZE : bufferIndex;
  for (int i = 1; i < len; i++) {
    if (irBuffer[i]  > irMax)  irMax  = irBuffer[i];
    if (irBuffer[i]  < irMin)  irMin  = irBuffer[i];
    if (redBuffer[i] > redMax) redMax = redBuffer[i];
    if (redBuffer[i] < redMin) redMin = redBuffer[i];
  }

  float irAC  = irMax  - irMin;
  float redAC = redMax - redMin;
  float irDC  = (float)(irMax  + irMin) / 2.0f;
  float redDC = (float)(redMax + redMin) / 2.0f;

  if (irDC < 1 || redDC < 1 || irAC < 1) return;

  float R        = (redAC / redDC) / (irAC / irDC);
  float rawSpO2  = 104.0f - 17.0f * R;
  rawSpO2        = constrain(rawSpO2, 85.0f, 100.0f);

  smoothSpO2 = 0.3f * smoothSpO2 + 0.7f * rawSpO2;
  spo2       = smoothSpO2;
}

// ═════════════════════════════════════════════════════════════
void detectSteps() {
  mpu.update();
  float ax  = mpu.getAccX();
  float ay  = mpu.getAccY();
  float az  = mpu.getAccZ();
  float mag = sqrt(ax*ax + ay*ay + az*az);

  if (mag > 1.2f && lastAccel <= 1.2f) {
    if (millis() - lastStepTime > 300) {
      stepCount++;
      lastStepTime = millis();
    }
  }
  lastAccel = mag;
}

// ═════════════════════════════════════════════════════════════
void readSensor() {
  long ir  = sensor.getIR();
  long red = sensor.getRed();

  if (ir > 50000) {
    fingerDetected = true;

    irBuffer[bufferIndex]  = ir;
    redBuffer[bufferIndex] = red;
    bufferIndex++;
    if (bufferIndex >= BUFFER_SIZE) {
      bufferIndex = 0;
      bufferFull  = true;
    }

    float detectedHR = computeHR(ir);
    if (detectedHR > 40 && detectedHR < 200) {
      smoothBPM = 0.3f * smoothBPM + 0.7f * detectedHR;
    }
    bpm = smoothBPM;

    computeSpO2();

  } else {
    fingerDetected = false;
    bpm  = 0;
    spo2 = 0;
    irPrev = irPrev2 = 0;
    peakIdx     = 0;
    bufferIndex = 0;
    bufferFull  = false;
    smoothBPM   = 72.0f;
    smoothSpO2  = 98.0f;
  }
}

// ═════════════════════════════════════════════════════════════
void sendData() {
  detectSteps();

  int hrInt   = (int)round(bpm);
  int spo2Int = (int)round(spo2);

  if (!fingerDetected) {
    displayData(0, 0, "NO FINGER", "NO DATA");
    Blynk.virtualWrite(VPIN_HR,            0);
    Blynk.virtualWrite(VPIN_SPO2,          0);
    Blynk.virtualWrite(VPIN_ALERT,         "NO FINGER");
    Blynk.virtualWrite(VPIN_HEALTH_STATUS, "NO DATA");

  } else {
    int    risk    = predictRisk(hrInt, spo2Int);
    String riskStr = (risk == 0) ? "LOW" :
                     (risk == 1) ? "MOD" : "HIGH";
    String health  = (spo2Int >= 95 && hrInt >= 60 && hrInt <= 100) ? "HEALTHY" :
                     (spo2Int >= 90) ? "SICK" : "DISEASE";

    displayData(hrInt, spo2Int, riskStr, health);

    Blynk.virtualWrite(VPIN_HR,            hrInt);
    Blynk.virtualWrite(VPIN_SPO2,          spo2Int);
    Blynk.virtualWrite(VPIN_RISK,          risk);
    Blynk.virtualWrite(VPIN_ALERT,         riskStr);
    Blynk.virtualWrite(VPIN_AI_SCORE,      (risk==0)?25:(risk==1)?60:90);
    Blynk.virtualWrite(VPIN_STEPS,         stepCount);
    Blynk.virtualWrite(VPIN_HEALTH_STATUS, health);
  }
}

// ═════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);

  // OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED FAIL");
    while (1);
  }
  displayMessage("CardioGuard", "Starting...", "");
  delay(1500);

  // MAX30105
  if (!sensor.begin(Wire, I2C_SPEED_FAST)) {
    displayMessage("MAX30105", "NOT FOUND!", "Check wiring");
    Serial.println("MAX30105 not found");
    while (1);
  }
  sensor.setup(60, 4, 2, 400, 411, 4096);
  sensor.setPulseAmplitudeRed(0x3C);
  sensor.setPulseAmplitudeIR(0x3C);

  // MPU6050
  mpu.begin();
  mpu.calcOffsets(true, true);

  // WiFi + Blynk
  displayMessage("Connecting WiFi", ssid, "Please wait...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  displayMessage("WiFi Connected", "Blynk Ready", "Place finger...");
  delay(1500);

  // Initial buffer fill
  displayMessage("Calibrating...", "Place finger", "on sensor");
  for (int i = 0; i < 50; i++) {
    sensor.check();
    delay(20);
  }

  timer.setInterval(2000L, sendData);
  timer.setInterval(20L,   readSensor);
}

// ═════════════════════════════════════════════════════════════
void loop() {
  sensor.check();
  Blynk.run();
  timer.run();
}