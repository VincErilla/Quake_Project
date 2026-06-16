#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <ADXL345_WE.h>
#include <SD.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <WebServer.h>
#include <RTClib.h> 

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

// I2C pins for RTC
#define SDA_PIN 22
#define SCL_PIN 33

#define LED_RED 25
#define LED_BLUE 26
#define START_BUTTON 13

bool spi = true;
SPIClass spiAccel(HSPI);
ADXL345_WE accel1 = ADXL345_WE(&spiAccel, CS1, spi);
ADXL345_WE accel2 = ADXL345_WE(&spiAccel, CS2, spi);

RTC_DS3231 rtc;
File f;
char filename[40]; 

unsigned long lastSampleTime = 0;
const unsigned long samplingIntervalMicros = 2000; // ~500 Hz
unsigned long startTime = 0;

float t;
// Global holding variables for historical backup tracking
float last_x1 = 0.0, last_y1 = 0.0, last_z1 = 1.0;
float last_x2 = 0.0, last_y2 = 0.0, last_z2 = 1.0;

// Precision Ellipsoidal Calibration Coefficients
const float A1_inv[3][3] = {
 {1.224426, 0.022806, 0.017952}, 
 {0.022806, 1.040584, -0.033441}, 
 {0.017952, -0.033441, 1.073030}   
};
const float b1[3] = {-0.022412, -0.045520, -0.094355};

const float A2_inv[3][3] = {
 {1.174304, -0.018755, -0.076771}, 
 {-0.018755, 1.037578, -0.044846}, 
 {-0.076771, -0.044846, 1.042790} 
};
const float b2[3] = {-0.023997, -0.002074, 0.005295};

TaskHandle_t sdWriterTaskHandle;
String activeBuffer = "";
String writeBuffer = "";
portMUX_TYPE bufferMux = portMUX_INITIALIZER_UNLOCKED;

void createFilename() {
  DateTime now = rtc.now();
  snprintf(filename, sizeof(filename), "/q%02d%02d%02d.csv",
           now.hour(), now.minute(), now.second());
}

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
    activeBuffer.reserve(32768); 
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

  activeBuffer.reserve(32768);
  writeBuffer.reserve(32768);

  setupWiFi();
  Wire.begin(SDA_PIN, SCL_PIN);

  if (!rtc.begin(&Wire)) {
    Serial.println("Couldn't find RTC module!");
    while (1);
  }
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, syncing compile date configurations...");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("SD card init failed!");
    while (1) {
      digitalWrite(LED_RED, HIGH); delay(250);
      digitalWrite(LED_RED, LOW); delay(250);
    }
  }

  // OPTIMIZATION: Dropped to 500kHz to preserve clean SPI square-waves over long wiring lines
  spiAccel.begin(HSPI_SCK, HSPI_MISO, HSPI_MOSI, -1);
  spiAccel.setFrequency(500000); 

  if (!accel1.init() || !accel2.init()) {
    Serial.println("Accelerometer init failed!");
    while (1) {
      digitalWrite(LED_RED, HIGH); digitalWrite(LED_BLUE, HIGH); delay(250);
      digitalWrite(LED_RED, LOW); digitalWrite(LED_BLUE, LOW); delay(250);
    }
  }

  accel1.setDataRate(ADXL345_DATA_RATE_800);
  accel2.setDataRate(ADXL345_DATA_RATE_800);
  accel1.setRange(ADXL345_RANGE_16G);
  accel2.setRange(ADXL345_RANGE_16G);

  xTaskCreatePinnedToCore(writeToSDTask, "SDWriterTask", 4096, NULL, 1, &sdWriterTaskHandle, 0);

  digitalWrite(LED_RED, HIGH);
  while (digitalRead(START_BUTTON) == LOW) delay(50);
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_BLUE, HIGH);

  createFilename();
  f = SD.open(filename, FILE_WRITE);
  if (f) {
    f.println("Time,X1,Y1,Z1,X2,Y2,Z2");
    f.close();
  }
  startTime = micros();
}

void loop() {
  unsigned long now = micros() - startTime;

  if (now - lastSampleTime > 8000) {
    lastSampleTime = now - 2000;
  }

  while (now - lastSampleTime >= 2000) {
    lastSampleTime += 2000; 

    xyzFloat e1, e2;
    accel1.getGValues(&e1);
    delayMicroseconds(20);
    accel2.getGValues(&e2);
    delayMicroseconds(20);

    // --- RAW VALUES FAULT INTERCEPTION: SENSOR 1 ---
    float x1_raw, y1_raw, z1_raw;
    // if (e1.x == 0.0 && e1.y == 0.0 && e1.z == 0.0) {
    //   Serial.println("*** SENSOR 1 ALL ZEROS ***");
    //   Serial.printf("RAW EVENT: X=%.6f Y=%.6f Z=%.6f\n", e1.x, e1.y, e1.z);
    //   x1_raw = last_x1; y1_raw = last_y1; z1_raw = last_z1;
    // } else {
    //   x1_raw = e1.x; 
    //   y1_raw = e1.y; 
    //   z1_raw = e1.z;
    //   last_x1 = x1_raw; last_y1 = y1_raw; last_z1 = z1_raw;
    // }

    // --- RAW VALUES FAULT INTERCEPTION: SENSOR 2 ---
    float x2_raw, y2_raw, z2_raw;
    // if (e2.x == 0.0 && e2.y == 0.0 && e2.z == 0.0) {
    //   Serial.println("*** SENSOR 2 ALL ZEROS ***");
    //   Serial.printf("RAW EVENT: X=%.6f Y=%.6f Z=%.6f\n", e2.x, e2.y, e2.z);
    //   x2_raw = last_x2; y2_raw = last_y2; z2_raw = last_z2;
    // } else {
    //   x2_raw = e2.x; 
    //   y2_raw = e2.y; 
    //   z2_raw = e2.z;
    //   last_x2 = x2_raw; last_y2 = y2_raw; last_z2 = z2_raw;
    // }

    // Apply Calibration Matrices safely onto filtered, non-zero raw variables
    float x1_cal = x1_raw; float y1_cal = y1_raw; float z1_cal = z1_raw;
    applyCalibration(x1_cal, y1_cal, z1_cal, A1_inv, b1);

    float x2_cal = x2_raw; float y2_cal = y2_raw; float z2_cal = z2_raw;
    applyCalibration(x2_cal, y2_cal, z2_cal, A2_inv, b2);

    t = lastSampleTime / 1000000.0;

    // Throttled Full 6-Axis Streaming Output Block (~166.67 Hz decimation)
    static int serialThrottleCounter = 0;
    serialThrottleCounter++;
    if (serialThrottleCounter >= 3) { 
        Serial.printf("%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f\n", t, x1_cal, y1_cal, z1_cal, x2_cal, y2_cal, z2_cal);
        serialThrottleCounter = 0; 
    }

    char line[120];
    snprintf(line, sizeof(line), "%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f\n",
             t, x1_cal, y1_cal, z1_cal, x2_cal, y2_cal, z2_cal);

    // Time-Gated Web JSON Generation (10 Hz)
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