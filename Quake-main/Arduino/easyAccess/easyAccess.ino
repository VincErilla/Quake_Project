#include <WiFi.h>
#include <WebServer.h>

// Replace with your network credentials
const char *ssid = "ESP32-Access-Point";
const char *password = "123456789";

// Create an instance of the server
WebServer server(80);

// HTML content
const char *htmlContent = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>OSU Quake</title>
  <link rel="stylesheet" href="styles.css">
  <script defer src="script.js"></script>
</head>
<body>
  <div class="container">
    <header>
      <h1>OSU Quake</h1>
      <p>Real-time vibration and gps data</p>
    </header>
    <div class="controls">
      <button class="export-btn" onclick="exportData()">Export Info</button>
    </div>
    <div class="output-stream">
      <h2>XYZ Data Stream</h2>
      <div class="stream-labels">
        <span>Timestamp</span>
        <span>X (g)</span>
        <span>Y (g)</span>
        <span>Z (g)</span>
        <span>RMS</span>
      </div>
      <div id="xyz-stream" class="stream-content"></div>
    </div>
    <div class="output-stream">
      <h2>GPS Data Stream</h2>
      <div class="stream-labels">
        <span>Timestamp</span>
        <span>Latitude (°)</span>
        <span>Longitude (°)</span>
        <span>Altitude (FAMSL)</span>
        <span>Coordinates (DMS)</span>
      </div>
      <div id="gps-stream" class="stream-content"></div>
    </div>
    <footer>
      <p>Developed at Oklahoma State University 2024</p>
    </footer>
  </div>
</body>
</html>
)rawliteral";

// CSS content
const char *cssContent = R"rawliteral(
/* Reset basic styles */
* {
    margin: 0;
    padding: 0;
    box-sizing: border-box;
    font-family: 'Arial', sans-serif;
}

/* Container styles */
.container {
    display: flex;
    flex-direction: column;
    min-height: 100vh;
    background-color: #ffffff;
    color: #757575;
}

/* Header */
header {
    background-color: #ffffff;
    padding: 30px 20px;
    text-align: center;
    border-bottom: 3px solid #dddddd;
    box-shadow: 0 4px 8px rgba(0, 0, 0, 0.05);
    position: relative;
}

header::before {
    content: '';
    position: absolute;
    top: 0;
    left: 0;
    width: 100%;
    height: 8px;
    background-color: #fe5c00;
}

header h1 {
    font-size: 2.5rem;
    color: #000000;
    margin-bottom: 5px;
}

header p {
    font-size: 1rem;
    color: #757575;
}

/* Controls Section */
.controls {
    display: flex;
    justify-content: center;
    padding: 15px 0;
}

.export-btn {
    background-color: #fe5c00;
    color: #ffffff;
    padding: 10px 20px;
    cursor: pointer;
    transition: background-color 0.3s;
}

.export-btn:hover {
    background-color: #cc4b00;
}

/* Output Stream Section */
.output-stream {
    flex: 1;
    padding: 20px;
    background-color: #f9f9f9;
    border-top: 2px solid #000000;
    margin-bottom: 15px;
}

.output-stream h2 {
    margin-bottom: 10px;
    color: #fe5c00;
    font-size: 1.5rem;
    border-bottom: 2px solid #fe5c00;
}

/* Label Row */
.stream-labels {
    display: grid;
    grid-template-columns: 1fr 1fr 1fr 1fr 1fr;
    gap: 10px;
    background-color: #dddddd;
    padding: 10px;
    margin-bottom: 10px;
    text-align: center;
}

/* Stream Content */
.stream-content {
    max-height: 300px;
    overflow-y: auto;
    background-color: #ffffff;
    padding: 10px;
    border: 1px solid #000000;
}

.stream-row {
    display: grid;
    grid-template-columns: 1fr 1fr 1fr 1fr 1fr;
    gap: 10px;
    padding: 5px 0;
    border-bottom: 1px solid #dddddd;
    text-align: center;
}

/* Footer */
footer {
    background-color: #f9f9f9;
    text-align: center;
    padding: 10px;
    font-size: 0.9rem;
    color: #757575;
    border-top: 2px solid #dddddd;
}
)rawliteral";

// JavaScript content
const char *jsContent = R"rawliteral(
function exportData() {
    alert('Exporting data...');
    // Add your data export logic here
}
)rawliteral";

void setup() {
  // Start the Serial
  Serial.begin(115200);
  
  // Set up the access point
  WiFi.softAP(ssid, password);
  Serial.println("Access Point started");

  // Define routes
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", htmlContent);
  });
  server.on("/styles.css", HTTP_GET, []() {
    server.send(200, "text/css", cssContent);
  });
  server.on("/script.js", HTTP_GET, []() {
    server.send(200, "application/javascript", jsContent);
  });

  // Start the server
  WiFi.begin(ssid, password);
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // Handle client requests
  server.handleClient();
}
