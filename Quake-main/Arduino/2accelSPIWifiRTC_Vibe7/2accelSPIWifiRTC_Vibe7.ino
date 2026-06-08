#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <ADXL345_WE.h>
#include <SD.h>
#include <RTClib.h> // RESTORED: RTC Library
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <WebServer.h>

// WiFi credentials
const char* ssid = "ESP32-Graph";
const char* password = "12345678";

// Web server
WebServer server(80);
String latestData = "";

// SD Card on default SPI
#define SD_CS_PIN 4

// SPI pins for custom bus
#define HSPI_MISO 36
#define HSPI_MOSI 15
#define HSPI_SCK 14
#define CS1 32
#define CS2 20

#define LED_RED 25
#define LED_BLUE 26
#define START_BUTTON 13

// RESTORED: RTC Object (Uses default I2C pins: SDA=21, SCL=22)
RTC_DS3231 rtc;

bool spi = true;
SPIClass spiAccel(HSPI);
ADXL345_WE accel1 = ADXL345_WE(&spiAccel, CS1, spi);
ADXL345_WE accel2 = ADXL345_WE(&spiAccel, CS2, spi);

File f;
char filename[30]; // Modified to comfortably fit standard timestamp naming lengths

unsigned long lastSampleTime = 0;
const unsigned long samplingIntervalMicros = 2000; // ~500 Hz
unsigned long startTime = 0;

float t, x1_c, y1_c, z1_c, x2_c, y2_c, z2_c;

// Precision Calibration Coefficients Matched to Your Lab Bench Tests
const float A1_inv[3][3] = {
  {1.129373, -0.018889, 0.003953}, 
  {-0.018889, 1.220810, -0.015249}, 
  {0.003953, -0.015249, 1.036859}   
};
const float b1[3] = {0.044158, -0.102127, -0.035657};

const float A2_inv[3][3] = {
  {1.312979, -0.010986, 0.029372}, 
  {-0.010986, 1.056986, -0.008222}, 
  {0.029372, -0.008222, 1.052143}   
};
const float b2[3] = {0.015162, -0.031347, -0.024986};

TaskHandle_t sdWriterTaskHandle;
String activeBuffer = "";
String writeBuffer = "";
portMUX_TYPE bufferMux = portMUX_INITIALIZER_UNLOCKED;

void applyCalibration(float& x, float& y, float& z, const float A_inv[3][3], const float b[3]) {
  float x_new = A_inv[0][0] * (x - b[0]) + A_inv[0][1] * (y - b[1]) + A_inv[0][2] * (z - b[2]);
  float y_new = A_inv[1][0] * (x - b[0]) + A_inv[1][1] * (y - b[1]) + A_inv[1][2] * (z - b[2]);
  float z_new = A_inv[2][0] * (x - b[0]) + A_inv[2][1] * (y - b[1]) + A_inv[2][2] * (z - b[2]);
  x = x_new; y = y_new; z = z_new;
}

void handleRoot() {
  server.send(200, "text/html", R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <meta charset="UTF-8">
      <title>Accelerometer Live Graphs</title>
      <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    </head>
    <body>
      <h2>Accelerometer Data</h2>
      <canvas id="chartX" width="800" height="250"></canvas>
      <canvas id="chartY" width="800" height="250"></canvas>
      <canvas id="chartZ" width="800" height="250"></canvas>
      <canvas id="chartRMS" width="800" height="250"></canvas>

      <script>
        const createChart = (ctx, label1, label2, color1, color2) => {
          return new Chart(ctx, {
            type: 'line',
            data: {
              labels: [],
              datasets: [
                { label: label1, borderColor: color1, data: [], fill: false },
                { label: label2, borderColor: color2, data: [], fill: false }
              ]
            },
            options: {
              animation: false,
              responsive: true,
              scales: {
                x: { title: { display: true, text: 'Time (s)' }},
                y: { title: { display: true, text: 'g' }}
              }
            }
          });
        };

        const charts = {
          x: createChart(document.getElementById('chartX').getContext('2d'), 'X1', 'X2', 'red', 'blue'),
          y: createChart(document.getElementById('chartY').getContext('2d'), 'Y1', 'Y2', 'green', 'purple'),
          z: createChart(document.getElementById('chartZ').getContext('2d'), 'Z1', 'Z2', 'orange', 'cyan'),
          rms: createChart(document.getElementById('chartRMS').getContext('2d'), 'RMS1', 'RMS2', 'black', 'gray')
        };

        async function fetchData() {
          const res = await fetch("/data");
          const txt = await res.text();
          const vals = txt.trim().split(',');
          if (vals.length == 9) {
            const time = parseFloat(vals[0]);
            const [x1, x2, y1, y2, z1, z2, rms1, rms2] = vals.slice(1).map(parseFloat);

            for (let key in charts) {
              charts[key].data.labels.push(time);
              if (charts[key].data.labels.length > 100) charts[key].data.labels.shift();
            }

            charts.x.data.datasets[0].data.push(x1);
            charts.x.data.datasets[1].data.push(x2);
            charts.y.data.datasets[0].data.push(y1);
            charts.y.data.datasets[1].data.push(y2);
            charts.z.data.datasets[0].data.push(z1);
            charts.z.data.datasets[1].data.push(z2);
            charts.rms.data.datasets[0].data.push(rms1);
            charts.rms.data.datasets[1].data.push(rms2);

            ['x', 'y', 'z', 'rms'].forEach(k => {
              if (charts[k].data.datasets[0].data.length > 100) {
                charts[k].data.datasets[0].data.shift();
                charts[k].data.datasets[1].data.shift();
              }
              charts[k].update();
            });
          }
        }
        setInterval(fetchData, 500);
      </script>
    </body>
    </html>
  )rawliteral");
}

void handleData() {
  server.send(200, "text/plain", latestData);
}

void setupWiFi() {
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
  Serial.println("HTTP server started");
}

void writeToSDTask(void* pvParameters) {
  static unsigned long lastWriteTime = 0;
  while (true) {
    String temp;
    portENTER_CRITICAL(&bufferMux);
    writeBuffer = activeBuffer;
    activeBuffer = "";
    portEXIT_CRITICAL(&bufferMux);
    temp = writeBuffer;

    unsigned long now = millis();
    if (temp.length() >= 1024 || now - lastWriteTime >= 2000) {
      File f = SD.open(filename, FILE_APPEND);
      if (f) {
        f.write((const uint8_t*)temp.c_str(), temp.length());
        f.close();
        lastWriteTime = now;
      }
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void setup() {
  Serial.begin(115200); 
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(START_BUTTON, INPUT);

  // Initialize Default I2C Wire Bus for RTC Module
  Wire.begin();

  setupWiFi();

  // Initialize RTC Board
  if (!rtc.begin()) {
    Serial.println("Error: DS3231 RTC Module not detected!");
    while (1) {
      digitalWrite(LED_RED, HIGH); delay(100);
      digitalWrite(LED_RED, LOW); delay(100);
    }
  }

  // NOTE: If the backup cell battery runs out or is replaced, uncomment the 
  // line below ONCE to programmatically update the clock with your computer compile time:
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("SD card init failed!");
    while (1) {
      digitalWrite(LED_RED, HIGH); delay(250);
      digitalWrite(LED_RED, LOW); delay(250);
    }
  }

  spiAccel.begin(HSPI_SCK, HSPI_MISO, HSPI_MOSI, -1);
  if (!accel1.init() || !accel2.init()) {
    Serial.println("Accelerometer init failed!");
    while (1) {
      digitalWrite(LED_RED, HIGH); digitalWrite(LED_BLUE, HIGH); delay(250);
      digitalWrite(LED_RED, LOW); digitalWrite(LED_BLUE, LOW); delay(250);
    }
  }

  accel1.setDataRate(ADXL345_DATA_RATE_1600);
  accel2.setDataRate(ADXL345_DATA_RATE_1600);
  accel1.setRange(ADXL345_RANGE_16G);
  accel2.setRange(ADXL345_RANGE_16G);

  xTaskCreatePinnedToCore(writeToSDTask, "SDWriterTask", 4096, NULL, 1, &sdWriterTaskHandle, 0);

  // Hold setup until physical user button confirmation
  digitalWrite(LED_RED, HIGH);
  while (digitalRead(START_BUTTON) == LOW) delay(50);
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_BLUE, HIGH);

  // RESTORED: Generate unique filename using RTC calendar parameters
  DateTime nowTime = rtc.now();
  snprintf(filename, sizeof(filename), "/Q_%04d%02d%02d_%02d%02d.csv", 
           nowTime.year(), nowTime.month(), nowTime.day(), 
           nowTime.hour(), nowTime.minute());

  // Print filename out to Serial console for active terminal tracking verification
  Serial.print("Target file initialized: ");
  Serial.println(filename);

  f = SD.open(filename, FILE_WRITE);
  if (f) {
    f.println("Time,X1,Y1,Z1,X2,Y2,Z2");
    f.close();
  }

  startTime = micros();
}

int debugCounter = 0;
int loopThrottleCounter = 0; // Throttle counter for our real-time 100 Hz output

void loop() {
  unsigned long now = micros() - startTime;

  // --- ANTI-STALL GUARDRAIL ---
  if (now - lastSampleTime > 8000) {
    lastSampleTime = now - 2000;
  }

  // --- 500 Hz CRITICAL SAMPLING ENGINE ---
  while (now - lastSampleTime >= 2000) {
    lastSampleTime += 2000; 

    xyzFloat e1, e2;
    accel1.getGValues(&e1);
    accel2.getGValues(&e2);

    float x1_raw = e1.x, y1_raw = e1.y, z1_raw = e1.z;
    float x2_raw = e2.x, y2_raw = e2.y, z2_raw = e2.z;

    // Apply Matrix Calibration Calculations
    float x1_cal = x1_raw; float y1_cal = y1_raw; float z1_cal = z1_raw;
    applyCalibration(x1_cal, y1_cal, z1_cal, A1_inv, b1);

    float x2_cal = x2_raw; float y2_cal = y2_raw; float z2_cal = z2_raw;
    applyCalibration(x2_cal, y2_cal, z2_cal, A2_inv, b2);

    // Track strict execution timeline to feed MATLAB's FFT uniform structures
    t = lastSampleTime / 1000000.0;

    // DECIMATION CONTROL ENGINE: Throttles serial output from 500 Hz down to 100 Hz
    loopThrottleCounter++;
    if (loopThrottleCounter >= 5) {
      Serial.printf("%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f\n", t, x1_cal, y1_cal, z1_cal, x2_cal, y2_cal, z2_cal);
      loopThrottleCounter = 0; // Reset scale counter
    }

    // Capture everything at full 500 Hz resolution to write to the SD card
    char line[100];
    snprintf(line, sizeof(line), "%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f\n",
             t, x1_cal, y1_cal, z1_cal, x2_cal, y2_cal, z2_cal);

    // Update Web Graphics String (Throttled internally to 10 Hz)
    static unsigned long lastWebUpdateTime = 0;
    if (millis() - lastWebUpdateTime >= 100) {
      lastWebUpdateTime = millis();

      latestData = String(t, 4) + "," +
                   String(x1_cal, 2) + "," + String(x2_cal, 2) + "," +
                   String(y1_cal, 2) + "," + String(y2_cal, 2) + "," +
                   String(z1_cal, 2) + "," + String(z2_cal, 2) + "," +
                   String(sqrt((x1_cal*x1_cal + y1_cal*y1_cal + z1_cal*z1_cal)/3.0), 2) + "," +
                   String(sqrt((x2_cal*x2_cal + y2_cal*y2_cal + z2_cal*z2_cal)/3.0), 2);
    }

    // Thread-Safe Push to Core 0 Buffer
    portENTER_CRITICAL(&bufferMux);
    activeBuffer += line;
    portEXIT_CRITICAL(&bufferMux);
  }

  // --- SHUTDOWN SEQUENCE MANEUVER ---
  if (now > 2000000 && digitalRead(START_BUTTON) == HIGH) {
    digitalWrite(LED_BLUE, LOW); delay(100);
    digitalWrite(LED_RED, HIGH); digitalWrite(LED_BLUE, HIGH); delay(100);
    digitalWrite(LED_RED, LOW); digitalWrite(LED_BLUE, LOW); delay(100);
    digitalWrite(LED_RED, HIGH); digitalWrite(LED_BLUE, HIGH); delay(100);
    digitalWrite(LED_RED, LOW); digitalWrite(LED_BLUE, LOW);
    exit(0);
  }

  server.handleClient();
}