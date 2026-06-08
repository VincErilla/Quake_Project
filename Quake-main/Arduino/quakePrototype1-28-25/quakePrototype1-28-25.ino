#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
// #include <Adafruit_LIS331HH.h>
#include <Adafruit_ADXL375.h>
#include <SD.h>

// Wi-Fi credentials
const char* ssid = "ESP32-Access-Point";
const char* password = "123456789";

// LED pins
#define LED1 26 
#define LED2 25

// Server and SD configurations
WiFiServer server(80);
#define SD_CS_PIN 4
#define ADXL375_CS1 13
#define ADXL375_CS2 14
#define ADXL375_CS3 15
#define ADXL375_CS4 32


Adafruit_ADXL375 accel1 = Adafruit_ADXL375(ADXL375_CS1, &SPI, 12345);
Adafruit_ADXL375 accel2 = Adafruit_ADXL375(ADXL375_CS2, &SPI, 12345);
Adafruit_ADXL375 accel3 = Adafruit_ADXL375(ADXL375_CS3, &SPI, 12345);
Adafruit_ADXL375 accel4 = Adafruit_ADXL375(ADXL375_CS4, &SPI, 12345);

File dataFile;
const char* dataFileName = "/quakeData.csv";

void setup() {
  Serial.begin(115200);

  // Initialize SD card
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("SD card initialization failed!");
    while (1);
  }

  // Initialize Wi-Fi access point
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  server.begin();

  // Open data file
  dataFile = SD.open(dataFileName, FILE_WRITE);
  if (dataFile) {
    dataFile.println("Time, X1, Y1, Z1, RMS1, X2, Y2, Z2, RMS2, X3, Y3, Z3, RMS3, X4, Y4, Z4, RMS4");
    dataFile.flush();
  }

  // Initialize sensors
  if (!accel1.begin() || !accel2.begin() || !accel3.begin() || !accel4.begin()) {
    Serial.println("One or more sensors couldn't start.");
    while (1);
  }

}

void loop() {
  sensors_event_t event1, event2, event3, event4;
  accel1.getEvent(&event1);
  accel2.getEvent(&event2);
  accel3.getEvent(&event3);
  accel4.getEvent(&event4);

  float x1 = event1.acceleration.x;
  float y1 = event1.acceleration.y;
  float z1 = event1.acceleration.z;
  float x2 = event2.acceleration.x;
  float y2 = event2.acceleration.y;
  float z2 = event2.acceleration.z;
  float x3 = event3.acceleration.x;
  float y3 = event3.acceleration.y;
  float z3 = event3.acceleration.z;
  float x4 = event4.acceleration.x;
  float y4 = event4.acceleration.y;
  float z4 = event4.acceleration.z;

  float rms1 = sqrt((x1 * x1 + y1 * y1 + z1 * z1) / 3.0);
  float rms2 = sqrt((x2 * x2 + y2 * y2 + z2 * z2) / 3.0);
  float rms3 = sqrt((x3 * x3 + y3 * y3 + z3 * z3) / 3.0);
  float rms4 = sqrt((x4 * x4 + y4 * y4 + z4 * z4) / 3.0);
  float t = millis() / 1000.0;

  // Log data to SD card
  if (dataFile) {
    dataFile.printf("%f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f\n",
      t, x1, y1, z1, rms1, x2, y2, z2, rms2, x3, y3, z3, rms3, x4, y4, z4, rms4);
    dataFile.flush();
  }

  WiFiClient client = server.available();
  if (client) {
    String header;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        header += c;

        // Handle JSON data request
        if (c == '\n' && header.indexOf("GET /data") >= 0) {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type: application/json");
          client.println("Connection: close");
          client.println();

          client.printf("{\"time\":%f,\"x1\":%f,\"y1\":%f,\"z1\":%f,\"rms1\":%f,\"x2\":%f,\"y2\":%f,\"z2\":%f,\"rms2\":%f,\"x3\":%f,\"y3\":%f,\"z3\":%f,\"rms3\":%f,\"x4\":%f,\"y4\":%f,\"z4\":%f,\"rms4\":%f}\n",
            t, x1, y1, z1, rms1, x2, y2, z2, rms2, x3, y3, z3, rms3, x4, y4, z4, rms4);
          break;
        }

        // Handle HTML page request
        if (c == '\n' && header.indexOf("GET /") >= 0) {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type:text/html");
          client.println("Connection: close");
          client.println();

          client.println("<!DOCTYPE html>");
          client.println("<html lang=\"en\">");
          client.println("<head>");
          client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
          client.println("<style>");
          client.println("body { font-family: Arial, sans-serif; margin: 0; padding: 0; background-color: #f4f4f4; color: #333; }");
          client.println("header { background-color: #fe5c00; color: white; padding: 20px 10px; text-align: center; }");
          client.println("h1 { margin: 0; }");
          client.println("button { background-color: #fe5c00; color: white; border: none; padding: 10px 20px; cursor: pointer; font-size: 16px; border-radius: 5px; margin: 10px; }");
          client.println("button:hover { background-color: #e54a00; }");
          client.println("table { width: 100%; border-collapse: collapse; margin: 20px 0; }");
          client.println("th, td { border: 1px solid #ddd; padding: 12px; text-align: center; }");
          client.println("th { background-color: #333; color: white; }");
          client.println("tr:nth-child(even) { background-color: #f9f9f9; }");
          client.println("tr:hover { background-color: #f1f1f1; }");
          client.println(".container { padding: 20px; max-width: 1200px; margin: auto; }");
          client.println("</style>");
          client.println("<script>");
          client.println("function fetchData() {");
          client.println("  fetch('/data').then(res => res.json()).then(data => {");
          client.println("    let newRow = `<tr>");
          client.println("      <td>${data.time.toFixed(2)}</td>");
          client.println("      <td>${data.x1.toFixed(2)}</td>");
          client.println("      <td>${data.y1.toFixed(2)}</td>");
          client.println("      <td>${data.z1.toFixed(2)}</td>");
          client.println("      <td>${data.rms1.toFixed(2)}</td>");
          client.println("      <td>${data.x2.toFixed(2)}</td>");
          client.println("      <td>${data.y2.toFixed(2)}</td>");
          client.println("      <td>${data.z2.toFixed(2)}</td>");
          client.println("      <td>${data.rms2.toFixed(2)}</td>");
          client.println("      <td>${data.x3.toFixed(2)}</td>");
          client.println("      <td>${data.y3.toFixed(2)}</td>");
          client.println("      <td>${data.z3.toFixed(2)}</td>");
          client.println("      <td>${data.rms3.toFixed(2)}</td>");
          client.println("      <td>${data.x4.toFixed(2)}</td>");
          client.println("      <td>${data.y4.toFixed(2)}</td>");
          client.println("      <td>${data.z4.toFixed(2)}</td>");
          client.println("      <td>${data.rms4.toFixed(2)}</td>");
          client.println("    </tr>`;");
          client.println("    document.getElementById('dataTable').innerHTML += newRow;");
          client.println("  });");
          client.println("}");
          client.println("setInterval(fetchData, 1000);");
          client.println("function downloadCSV() {");
          client.println("  let csv = '';"); // Start without headers
          client.println("  let rows = document.querySelectorAll('table tr');");
          client.println("  rows.forEach((row, index) => {");
          client.println("    if (index > 0) {"); // Skip the first row (table header)
          client.println("      let cells = row.querySelectorAll('td');");
          client.println("      let rowText = Array.from(cells).map(cell => cell.innerText).join(',');");
          client.println("      csv += rowText + '\\n';");
          client.println("    }");
          client.println("  });");
          client.println("  let blob = new Blob([csv], { type: 'text/csv' });");
          client.println("  let url = window.URL.createObjectURL(blob);");
          client.println("  let a = document.createElement('a');");
          client.println("  a.setAttribute('hidden', '');");
          client.println("  a.setAttribute('href', url);");
          client.println("  a.setAttribute('download', 'data.csv');");
          client.println("  document.body.appendChild(a);");
          client.println("  a.click();");
          client.println("  document.body.removeChild(a);");
          client.println("}");
          client.println("</script>");
          client.println("</head>");
          client.println("<body>");
          client.println("<header>");
          client.println("<h1>Real-Time Accelerometer Data</h1>");
          client.println("</header>");
          client.println("<div class=\"container\">");
          client.println("<button onclick=\"downloadCSV()\">Download Table as CSV</button>");
          client.println("<table>");
          client.println("  <thead>");
          client.println("    <tr>");
          client.println("      <th>Time</th>");
          client.println("      <th>X1</th>");
          client.println("      <th>Y1</th>");
          client.println("      <th>Z1</th>");
          client.println("      <th>RMS1</th>");
          client.println("      <th>X2</th>");
          client.println("      <th>Y2</th>");
          client.println("      <th>Z2</th>");
          client.println("      <th>RMS2</th>");
          client.println("      <th>X3</th>");
          client.println("      <th>Y3</th>");
          client.println("      <th>Z3</th>");
          client.println("      <th>RMS3</th>");
          client.println("      <th>X4</th>");
          client.println("      <th>Y4</th>");
          client.println("      <th>Z4</th>");
          client.println("      <th>RMS4</th>");
          client.println("    </tr>");
          client.println("  </thead>");
          client.println("  <tbody id=\"dataTable\">");
          client.println("  </tbody>");
          client.println("</table>");
          client.println("</div>");
          client.println("</body>");
          client.println("</html>");
          break;
        }
      }
    }
    client.stop();
  }
}
