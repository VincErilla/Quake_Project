#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <ADXL345_WE.h>
#include <SD.h>
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

bool spi = true;
SPIClass spiAccel(HSPI);
ADXL345_WE accel1 = ADXL345_WE(&spiAccel, CS1, spi);
ADXL345_WE accel2 = ADXL345_WE(&spiAccel, CS2, spi);

File f;
File countFile;
const char* countFileName = "/runCount.txt";
char filename[30];

unsigned long lastSampleTime = 0;
const unsigned long samplingIntervalMicros = 1250; // ~800 Hz
unsigned long startTime = 0;

float t, x1_c, y1_c, z1_c, x2_c, y2_c, z2_c;

const float A1_inv[3][3] = {
  {1.027734, 0.002684, -0.025876},
  {0.002684, 1.005982, -0.043656},
  {-0.025876, -0.043656, 1.169701}
};

const float b1[3] = {0.003836, -0.076270, -0.164598};

const float A2_inv[3][3] = {
  {1.006628, 0.003033, -0.024392},
  {0.003033, 1.003335, -0.050616},
  {-0.024392, -0.050616, 1.166856}
};

const float b2[3] = {0.008780, -0.053935, -0.160464};

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
        // Serial.println(temp.c_str());
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

  setupWiFi();

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

  digitalWrite(LED_RED, HIGH);
  while (digitalRead(START_BUTTON) == LOW) delay(50);
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_BLUE, HIGH);

  int runCount = 0;
  countFile = SD.open(countFileName, FILE_READ);
  if (countFile) {
    runCount = countFile.parseInt();
    countFile.close();
  }
  runCount++;
  countFile = SD.open(countFileName, FILE_WRITE);
  if (countFile) {
    countFile.println(runCount);
    countFile.close();
  }

  sprintf(filename, "/quakeData_%d.csv", runCount);
  f = SD.open(filename, FILE_WRITE);
  if (f) {
    f.println("Time,X1,Y1,Z1,X2,Y2,Z2");
    f.close();
  }

  startTime = micros();
}

void loop() {
  unsigned long now = micros() - startTime;

  if (now - lastSampleTime >= samplingIntervalMicros) {
    lastSampleTime += samplingIntervalMicros;

    xyzFloat e1, e2;
    accel1.getGValues(&e1);
    accel2.getGValues(&e2);

    float x1_raw = e1.x, y1_raw = e1.y, z1_raw = e1.z;
    float x2_raw = e2.x, y2_raw = e2.y, z2_raw = e2.z;

    
    // two point calibration choose one or the other
    float accel1_xmin = -0.94, accel1_ymin = -1.06, accel1_zmin = -1.01;
    float accel1_xmax = 0.99, accel1_ymax = 0.91, accel1_zmax = 0.94;
    
    float accel2_xmin = -0.98, accel2_ymin = -1.03, accel2_zmin = -1.00;
    float accel2_xmax = 1.03, accel2_ymax = 0.96, accel2_zmax = 0.93;

    float accel1_rawxrange = 1.93, accel1_rawyrange = 1.97, accel1_rawzrange = 1.95;
    float referenceRange = 2, referenceLow = -1;

    float accel2_rawxrange = 1.93, accel2_rawyrange = 1.97, accel2_rawzrange = 1.95;

    float x1_cal = (((x1_raw - accel1_xmin) * referenceRange) / accel1_rawxrange) + referenceLow;
    float y1_cal = (((y1_raw - accel1_ymin) * referenceRange) / accel1_rawyrange) + referenceLow;
    float z1_cal = (((z1_raw - accel1_zmin) * referenceRange) / accel1_rawzrange) + referenceLow;

    float x2_cal = (((x2_raw - accel2_xmin) * referenceRange) / accel2_rawxrange) + referenceLow;
    float y2_cal = (((y2_raw - accel2_ymin) * referenceRange) / accel2_rawyrange) + referenceLow;
    float z2_cal = (((z2_raw - accel2_zmin) * referenceRange) / accel2_rawzrange) + referenceLow;
    

    /*
    // Magneto Calibration choose one or the other
    float x1_cal = x1_raw;
    float y1_cal = y1_raw;
    float z1_cal = z1_raw;
    applyCalibration(x1_cal, y1_cal, z1_cal, A1_inv, b1);

    float x2_cal = x2_raw;
    float y2_cal = y2_raw;
    float z2_cal = z2_raw;
    applyCalibration(x2_cal, y2_cal, z2_cal, A2_inv, b2);
    */
    // CorrectedValue = (((RawValue – RawLow) * ReferenceRange) / RawRange) + ReferenceLow
    t = now / 1e6;
    char line[100];
    snprintf(line, sizeof(line), "%.4f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
             t, x1_cal, y1_cal, z1_cal, x2_cal, y2_cal, z2_cal);

    latestData = String(t, 4) + "," +
             String(x1_cal, 2) + "," + String(x2_cal, 2) + "," +
             String(y1_cal, 2) + "," + String(y2_cal, 2) + "," +
             String(z1_cal, 2) + "," + String(z2_cal, 2) + "," +
             String(sqrt((x1_cal*x1_cal + y1_cal*y1_cal + z1_cal*z1_cal)/3.0), 2) + "," +
             String(sqrt((x2_cal*x2_cal + y2_cal*y2_cal + z2_cal*z2_cal)/3.0), 2);


    portENTER_CRITICAL(&bufferMux);
    activeBuffer += line;
    portEXIT_CRITICAL(&bufferMux);
  }

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
