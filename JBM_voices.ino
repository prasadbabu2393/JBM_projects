// // ESP32 Input Pin Debounce Control
// // Input Pin: 35 (detects HIGH to LOW transition)
// // Output Pin 27: ON for debounce duration
// // Output Pin 23: ON for 5 seconds only

// // Global variables - easily adjustable
// const unsigned long DEBOUNCE_TIME = 60000; // 3 minutes in milliseconds (3 * 60 * 1000)
// const unsigned long PIN23_ON_TIME = 15000;   // 5 seconds in milliseconds

// // Pin definitions
// const int INPUT_PIN = 35;
// const int OUTPUT_PIN_27 = 27;
// const int OUTPUT_PIN_23 = 25;

// // State tracking variables
// int lastInputState = HIGH;
// int currentInputState = HIGH;
// unsigned long debounceStartTime = 0;
// unsigned long pin23StartTime = 0;
// bool debounceActive = false;
// bool pin23Active = false;

// void setup() {
//   // Initialize serial for debugging
//   Serial.begin(115200);
//   Serial.println("ESP32 Debounce Control Started");
  
//   // Configure pins
//   pinMode(INPUT_PIN, INPUT_PULLUP);  // Input with internal pull-up
//   pinMode(OUTPUT_PIN_27, OUTPUT);
//   pinMode(OUTPUT_PIN_23, OUTPUT);
  
//   // Ensure outputs are LOW initially
//   digitalWrite(OUTPUT_PIN_27, LOW);
//   digitalWrite(OUTPUT_PIN_23, LOW);
  
//   Serial.println("Pin Configuration:");
//   Serial.println("Input Pin: 35");
//   Serial.println("Output Pin 27: Full debounce duration");
//   Serial.println("Output Pin 23: 5 seconds only");
//   Serial.printf("Debounce Time: %lu ms (%.1f minutes)\n", DEBOUNCE_TIME, DEBOUNCE_TIME/60000.0);
//   Serial.printf("Pin 23 ON Time: %lu ms\n", PIN23_ON_TIME);
// }

// void loop() {
//   currentInputState = digitalRead(INPUT_PIN);
//   unsigned long currentTime = millis();
  
//   // Detect HIGH to LOW transition
//   if (lastInputState == HIGH && currentInputState == LOW && !debounceActive) {
//     // State changed from HIGH to LOW - start debounce period
//     debounceActive = true;
//     pin23Active = true;
//     debounceStartTime = currentTime;
//     pin23StartTime = currentTime;
    
//     // Turn ON both outputs
//     digitalWrite(OUTPUT_PIN_27, HIGH);
//     digitalWrite(OUTPUT_PIN_23, HIGH);
    
//     Serial.println("\n=== Input Triggered (HIGH -> LOW) ===");
//     Serial.println("Pin 27: ON");
//     Serial.println("Pin 23: ON");
//     Serial.printf("Debounce period started for %lu ms\n", DEBOUNCE_TIME);
//   }
  
//   // Handle Pin 23 timing (5 seconds only)
//   if (pin23Active && (currentTime - pin23StartTime >= PIN23_ON_TIME)) {
//     digitalWrite(OUTPUT_PIN_23, LOW);
//     pin23Active = false;
//     Serial.println("Pin 23: OFF (5 seconds elapsed)");
//   }
  
//   // Handle debounce timing (full duration for Pin 27)
//   if (debounceActive && (currentTime - debounceStartTime >= DEBOUNCE_TIME)) {
//     digitalWrite(OUTPUT_PIN_27, LOW);
//     debounceActive = false;
//     Serial.println("Pin 27: OFF (Debounce period completed)");
//     Serial.println("=== Ready for next trigger ===\n");
//   }
  
//   lastInputState = currentInputState;
  
//   // Small delay to prevent excessive CPU usage
//   delay(10);
// }



//////--------------    with web ui  ---------

// ESP32 Input Pin Debounce Control with SoftAP and Web Interface
// Input Pin: 35 (detects HIGH to LOW transition)
// Output Pin 27: ON for debounce duration
// Output Pin 23: ON for 5 seconds only

#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>
#include <Preferences.h>

// SoftAP credentials
const char* AP_SSID = "JBM_Voices";
const char* AP_PASSWORD = "12345678";

// Global variables - easily adjustable
unsigned long DEBOUNCE_TIME = 60000; // Default: 1 minute in milliseconds
const unsigned long PIN23_ON_TIME = 15000;   // 5 seconds in milliseconds

// Pin definitions
const int INPUT_PIN = 35;
const int OUTPUT_PIN_27 = 27;
const int OUTPUT_PIN_23 = 25;

// State tracking variables
int lastInputState = HIGH;
int currentInputState = HIGH;
unsigned long debounceStartTime = 0;
unsigned long pin23StartTime = 0;
bool debounceActive = false;
bool pin23Active = false;

// Web server and preferences
WebServer server(80);
Preferences preferences;

// HTML web page with firmware update and debounce time configuration
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Voice Debounce Control</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            max-width: 600px;
            margin: 50px auto;
            padding: 20px;
            background-color: #f0f0f0;
        }
        .container {
            background-color: white;
            padding: 30px;
            border-radius: 10px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
        }
        h1 {
            color: #333;
            text-align: center;
        }
        .section {
            margin: 30px 0;
            padding: 20px;
            background-color: #f9f9f9;
            border-radius: 5px;
        }
        .section h2 {
            color: #555;
            margin-top: 0;
        }
        label {
            display: block;
            margin: 10px 0 5px 0;
            font-weight: bold;
        }
        input[type="number"], input[type="file"] {
            width: 100%;
            padding: 10px;
            margin-bottom: 10px;
            border: 1px solid #ddd;
            border-radius: 4px;
            box-sizing: border-box;
        }
        button {
            background-color: #4CAF50;
            color: white;
            padding: 12px 24px;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            font-size: 16px;
            width: 100%;
            margin-top: 10px;
        }
        button:hover {
            background-color: #45a049;
        }
        .info {
            background-color: #e7f3fe;
            padding: 15px;
            border-left: 4px solid #2196F3;
            margin: 15px 0;
        }
        .success {
            background-color: #d4edda;
            color: #155724;
            padding: 10px;
            border-radius: 4px;
            margin: 10px 0;
            display: none;
        }
        .error {
            background-color: #f8d7da;
            color: #721c24;
            padding: 10px;
            border-radius: 4px;
            margin: 10px 0;
            display: none;
        }
        #progress {
            width: 100%;
            height: 30px;
            background-color: #f0f0f0;
            border-radius: 4px;
            overflow: hidden;
            margin: 10px 0;
            display: none;
        }
        #progressBar {
            height: 100%;
            background-color: #4CAF50;
            width: 0%;
            text-align: center;
            line-height: 30px;
            color: white;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Voice Debounce Control</h1>
        
        <div class="section">
            <h2> Debounce Configuration</h2>
            <div class="info">
                <strong>Current Debounce Time:</strong> <span id="currentTime">%DEBOUNCE_TIME%</span> ms 
                (<span id="currentMinutes">%DEBOUNCE_MINUTES%</span> minutes)
            </div>
            <form id="configForm">
                <label>New Debounce Time (milliseconds):</label>
                <input type="number" id="debounceTime" name="debounceTime" min="1000" max="3600000" step="1000" value="%DEBOUNCE_TIME%" required>
                <small>Range: 1,000 ms (1 sec) to 3,600,000 ms (60 min)</small>
                <button type="submit">Save Configuration</button>
            </form>
            <div id="configSuccess" class="success"></div>
            <div id="configError" class="error"></div>
        </div>

        <div class="section">
            <h2> Firmware Update</h2>
            <form id="uploadForm" enctype="multipart/form-data">
                <label>Select Firmware File (.bin):</label>
                <input type="file" id="firmwareFile" name="firmware" accept=".bin" required>
                <div id="progress">
                    <div id="progressBar">0%</div>
                </div>
                <button type="submit">Upload Firmware</button>
            </form>
            <div id="uploadSuccess" class="success"></div>
            <div id="uploadError" class="error"></div>
        </div>
    </div>

    <script>
        // Handle configuration form submission
        document.getElementById('configForm').addEventListener('submit', function(e) {
            e.preventDefault();
            const debounceTime = document.getElementById('debounceTime').value;
            
            fetch('/setDebounce', {
                method: 'POST',
                headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                body: 'debounceTime=' + debounceTime
            })
            .then(response => response.text())
            .then(data => {
                document.getElementById('configSuccess').textContent = 'Configuration saved successfully! New debounce time: ' + debounceTime + ' ms';
                document.getElementById('configSuccess').style.display = 'block';
                document.getElementById('configError').style.display = 'none';
                
                // Update displayed values
                document.getElementById('currentTime').textContent = debounceTime;
                document.getElementById('currentMinutes').textContent = (debounceTime / 60000).toFixed(2);
                
                setTimeout(() => {
                    document.getElementById('configSuccess').style.display = 'none';
                }, 5000);
            })
            .catch(error => {
                document.getElementById('configError').textContent = 'Error saving configuration: ' + error;
                document.getElementById('configError').style.display = 'block';
                document.getElementById('configSuccess').style.display = 'none';
            });
        });

        // Handle firmware upload
        document.getElementById('uploadForm').addEventListener('submit', function(e) {
            e.preventDefault();
            
            const fileInput = document.getElementById('firmwareFile');
            const file = fileInput.files[0];
            
            if (!file) {
                alert('Please select a firmware file');
                return;
            }

            const formData = new FormData();
            formData.append('firmware', file);

            document.getElementById('progress').style.display = 'block';
            document.getElementById('uploadError').style.display = 'none';
            document.getElementById('uploadSuccess').style.display = 'none';

            const xhr = new XMLHttpRequest();
            
            xhr.upload.addEventListener('progress', function(e) {
                if (e.lengthComputable) {
                    const percentComplete = (e.loaded / e.total) * 100;
                    document.getElementById('progressBar').style.width = percentComplete + '%';
                    document.getElementById('progressBar').textContent = Math.round(percentComplete) + '%';
                }
            });

            xhr.addEventListener('load', function() {
                if (xhr.status === 200) {
                    document.getElementById('uploadSuccess').textContent = 'Firmware uploaded successfully! Device will reboot...';
                    document.getElementById('uploadSuccess').style.display = 'block';
                    document.getElementById('progressBar').style.width = '100%';
                    document.getElementById('progressBar').textContent = '100%';
                } else {
                    document.getElementById('uploadError').textContent = 'Upload failed: ' + xhr.statusText;
                    document.getElementById('uploadError').style.display = 'block';
                }
            });

            xhr.addEventListener('error', function() {
                document.getElementById('uploadError').textContent = 'Upload error occurred';
                document.getElementById('uploadError').style.display = 'block';
            });

            xhr.open('POST', '/update');
            xhr.send(formData);
        });
    </script>
</body>
</html>
)rawliteral";

// Load debounce time from flash
void loadDebounceTime() {
    preferences.begin("debounce", false);
    DEBOUNCE_TIME = preferences.getULong("time", 60000); // Default 1 minute
    preferences.end();
    Serial.printf("Loaded Debounce Time from Flash: %lu ms (%.2f minutes)\n", DEBOUNCE_TIME, DEBOUNCE_TIME/60000.0);
}

// Save debounce time to flash
void saveDebounceTime(unsigned long time) {
    preferences.begin("debounce", false);
    preferences.putULong("time", time);
    preferences.end();
    Serial.printf("Saved Debounce Time to Flash: %lu ms (%.2f minutes)\n", time, time/60000.0);
}

// Handle root page
void handleRoot() {
    String html = String(index_html);
    html.replace("%DEBOUNCE_TIME%", String(DEBOUNCE_TIME));
    html.replace("%DEBOUNCE_MINUTES%", String(DEBOUNCE_TIME / 60000.0, 2));
    server.send(200, "text/html", html);
}

// Handle debounce time configuration
void handleSetDebounce() {
    if (server.hasArg("debounceTime")) {
        unsigned long newTime = server.arg("debounceTime").toInt();
        if (newTime >= 1000 && newTime <= 3600000) {
            DEBOUNCE_TIME = newTime;
            saveDebounceTime(DEBOUNCE_TIME);
            server.send(200, "text/plain", "OK");
            Serial.printf("Debounce time updated to: %lu ms\n", DEBOUNCE_TIME);
        } else {
            server.send(400, "text/plain", "Invalid time range");
        }
    } else {
        server.send(400, "text/plain", "Missing parameter");
    }
}

// Handle firmware update
void handleUpdate() {
    HTTPUpload& upload = server.upload();
    
    if (upload.status == UPLOAD_FILE_START) {
        Serial.printf("Update: %s\n", upload.filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
            Update.printError(Serial);
        }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
            Update.printError(Serial);
        }
    } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) {
            Serial.printf("Update Success: %u bytes\nRebooting...\n", upload.totalSize);
        } else {
            Update.printError(Serial);
        }
    }
}

// Handle update completion
void handleUpdateEnd() {
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    delay(1000);
    ESP.restart();
}

void setup() {
    // Initialize serial for debugging
    Serial.begin(115200);
    Serial.println("\n\Voice Debounce Control Started");
    
    // Load saved debounce time from flash
    loadDebounceTime();
    
    // Configure pins
    pinMode(INPUT_PIN, INPUT_PULLUP);  // Input with internal pull-up
    pinMode(OUTPUT_PIN_27, OUTPUT);
    pinMode(OUTPUT_PIN_23, OUTPUT);
    
    // Ensure outputs are LOW initially
    digitalWrite(OUTPUT_PIN_27, LOW);
    digitalWrite(OUTPUT_PIN_23, LOW);
    
    Serial.println("Pin Configuration:");
    Serial.println("Input Pin: 35");
    Serial.println("Output Pin 27: Full debounce duration");
    Serial.println("Output Pin 23: 5 seconds only");
    Serial.printf("Debounce Time: %lu ms (%.2f minutes)\n", DEBOUNCE_TIME, DEBOUNCE_TIME/60000.0);
    Serial.printf("Pin 23 ON Time: %lu ms\n", PIN23_ON_TIME);
    
    // Setup SoftAP
    Serial.println("\nStarting SoftAP...");
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
    Serial.printf("AP SSID: %s\n", AP_SSID);
    Serial.printf("AP Password: %s\n", AP_PASSWORD);
    
    // Setup web server routes
    server.on("/", HTTP_GET, handleRoot);
    server.on("/setDebounce", HTTP_POST, handleSetDebounce);
    server.on("/update", HTTP_POST, handleUpdateEnd, handleUpdate);
    
    server.begin();
    Serial.println("Web server started");
    Serial.println("Access at: http://" + IP.toString() + "/");
}

void loop() {
    // Handle web server
    server.handleClient();
    
    // Original debounce logic
    currentInputState = digitalRead(INPUT_PIN);
    unsigned long currentTime = millis();
    
    // Detect HIGH to LOW transition
    if (lastInputState == HIGH && currentInputState == LOW && !debounceActive) {
        // State changed from HIGH to LOW - start debounce period
        debounceActive = true;
        pin23Active = true;
        debounceStartTime = currentTime;
        pin23StartTime = currentTime;
        
        // Turn ON both outputs
        digitalWrite(OUTPUT_PIN_27, HIGH);
        digitalWrite(OUTPUT_PIN_23, HIGH);
        
        Serial.println("\n=== Input Triggered (HIGH -> LOW) ===");
        Serial.println("Pin 27: ON");
        Serial.println("Pin 23: ON");
        Serial.printf("Debounce period started for %lu ms\n", DEBOUNCE_TIME);
    }
    
    // Handle Pin 23 timing (5 seconds only)
    if (pin23Active && (currentTime - pin23StartTime >= PIN23_ON_TIME)) {
        digitalWrite(OUTPUT_PIN_23, LOW);
        pin23Active = false;
        Serial.println("Pin 23: OFF (5 seconds elapsed)");
    }
    
    // Handle debounce timing (full duration for Pin 27)
    if (debounceActive && (currentTime - debounceStartTime >= DEBOUNCE_TIME)) {
        digitalWrite(OUTPUT_PIN_27, LOW);
        debounceActive = false;
        Serial.println("Pin 27: OFF (Debounce period completed)");
        Serial.println("=== Ready for next trigger ===\n");
    }
    
    lastInputState = currentInputState;
    
    // Small delay to prevent excessive CPU usage
    delay(10);
}