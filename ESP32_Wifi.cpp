#include <WiFi.h>
#include <WebServer.h>

// Replace with your network credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Define the web server on port 80 (standard HTTP)
WebServer server(80);

// Pin for the built-in LED (typically GPIO 2 for ESP32 Dev Boards)
const int ledPin = 2;
// State tracker for the LED
int ledState = LOW;

/**
 * @brief Handles requests to the root path ("/")
 */
void handleRoot() {
  Serial.println("HTTP Request received for root path");
  
  String html = "<html><head><title>ESP32 Wi-Fi Test</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>body{font-family: Arial; text-align: center;} .btn{padding: 15px 30px; font-size: 20px; border-radius: 8px; margin-top: 20px; display: inline-block; text-decoration: none;} .on{background-color: #4CAF50; color: white;} .off{background-color: #f44336; color: white;}</style>";
  html += "</head><body>";
  html += "<h1>ESP32 Wi-Fi Connectivity Success!</h1>";
  html += "<p>Web Server is Active at IP: ";
  html += WiFi.localIP().toString();
  html += "</p>";
  html += "<p>Signal Strength: ";
  html += WiFi.RSSI();
  html += " dBm</p>";
  
  if (ledState == HIGH) {
    html += "<p>LED Status: ON</p>";
    html += "<a class='btn off' href='/off'>Turn LED OFF</a>";
  } else {
    html += "<p>LED Status: OFF</p>";
    html += "<a class='btn on' href='/on'>Turn LED ON</a>";
  }

  html += "</body></html>";
  server.send(200, "text/html", html);
}

/**
 * @brief Handles the LED ON command
 */
void handleLedOn() {
  digitalWrite(ledPin, HIGH);
  ledState = HIGH;
  server.sendHeader("Location", "/", true); // Redirect back to root
  server.send(302, "text/plain", "");
}

/**
 * @brief Handles the LED OFF command
 */
void handleLedOff() {
  digitalWrite(ledPin, LOW);
  ledState = LOW;
  server.sendHeader("Location", "/", true); // Redirect back to root
  server.send(302, "text/plain", "");
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); // Start with LED off

  Serial.println("\nConnecting to Wi-Fi...");
  if (strlen(password) == 0) {
    WiFi.begin(ssid); // Connect to open network
  } else {
    WiFi.begin(ssid, password); // Connect to secured network
  }

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Set up server routes
  server.on("/", handleRoot);
  server.on("/on", handleLedOn);
  server.on("/off", handleLedOff);
  
  // Add a catch-all handler for debugging
  server.onNotFound([]() {
    Serial.println("Page not found request received");
    server.send(404, "text/plain", "404: Not found");
  });
  
  server.begin();

  Serial.println("HTTP Server started. Use your browser to access the IP above.");
  Serial.print("Try accessing: http://");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Handle incoming client requests continuously
  server.handleClient();
  
  // Check WiFi connection status periodically
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 30000) { // Check every 30 seconds
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi connection lost!");
    } else {
      Serial.print("WiFi OK - IP: ");
      Serial.println(WiFi.localIP());
    }
    lastCheck = millis();
  }
}
