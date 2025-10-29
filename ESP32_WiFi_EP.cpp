#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>

// --- Configuration ---
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// --- IMPORTANT: PLACEHOLDER ENDPOINTS ---
// These MUST be replaced with the HTTPS URL of your own server-side Proxy API 
// that is securely connected to your MongoDB database.
const char* API_SAVE_ENDPOINT = "https://YOUR-CUSTOM-PROXY-API.com/api/v1/data/save";
const char* API_FETCH_ENDPOINT = "https://YOUR-CUSTOM-PROXY-API.com/api/v1/data/latest";
const int API_TIMEOUT_MS = 10000;

// Hardware/State
WebServer server(80);
float current_temperature = 25.5; // Simulate a sensor reading

// --- MODEL (Data & External Service Interaction) ---

/**
 * @brief Sends sensor data via HTTP POST to the Proxy API for permanent storage in MongoDB.
 * @return true if the API call was successful (HTTP 200/201), false otherwise.
 */
bool saveDataToMongoAPI(float temperature) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("MODEL: Wi-Fi not connected. Cannot send data.");
    return false;
  }

  HTTPClient http;
  
  // Prepare JSON payload
  String payload = "{\"device_id\":\"ESP32_001\",\"temperature\":" + String(temperature, 1) + "}";

  Serial.print("MODEL: Sending data to Proxy API for MongoDB: ");
  Serial.println(payload);

  http.begin(API_SAVE_ENDPOINT); 
  http.addHeader("Content-Type", "application/json");
  // In a real scenario, you would add an Authorization header here!
  // http.addHeader("Authorization", "Bearer YOUR_API_TOKEN");

  int httpResponseCode = http.POST(payload);

  if (httpResponseCode > 0) {
    // Check for success codes (2xx)
    if (httpResponseCode == HTTP_CODE_OK || httpResponseCode == HTTP_CODE_CREATED) {
      Serial.printf("MODEL: API Success Code: %d (Data saved to MongoDB Proxy)\n", httpResponseCode);
      http.end();
      return true;
    } else {
      Serial.printf("MODEL: API Failed (Response Code %d). Check Proxy logs.\n", httpResponseCode);
      // Read error message from server if available: Serial.println(http.getString());
      http.end();
      return false;
    }
  } else {
    Serial.printf("MODEL: API Connection Error: %s\n", http.errorToString(httpResponseCode).c_str());
    http.end();
    return false;
  }
}

/**
 * @brief Fetches the latest temperature data from the MongoDB database via the Proxy API.
 * * @return A String containing the fetched temperature value (or a status message on failure).
 */
String fetchDataFromMongoAPI() {
  if (WiFi.status() != WL_CONNECTED) return "ERROR: Wi-Fi Disconnected";

  HTTPClient http;
  String latestData = "Data Fetch Failed";
  
  // Adding device ID parameter for the API to know which device's data to retrieve
  String url = String(API_FETCH_ENDPOINT) + "?device=ESP32_001";
  http.begin(url);
  http.setTimeout(API_TIMEOUT_MS);

  int httpResponseCode = http.GET();
  
  if (httpResponseCode == HTTP_CODE_OK) {
    String response = http.getString();
    Serial.print("MODEL: Fetched raw data from Proxy: ");
    Serial.println(response);
    
    // --- SIMULATED/REQUIRED PARSING ---
    // In a real application, you MUST use a library like ArduinoJson to parse 
    // the JSON response from your API and extract the actual temperature.
    
    // Since we are using a placeholder API, we simulate a successful fetch 
    // with a random value framed as the "database temperature."
    float simulatedDbTemp = random(200, 350) / 10.0;
    latestData = "Success! (Simulated Temp: " + String(simulatedDbTemp, 1) + "°C)";
    
  } else {
    latestData = "GET ERROR (" + String(httpResponseCode) + ") - Check API URL/Server Status";
    Serial.println("MODEL: " + latestData);
  }
  
  http.end();
  return latestData;
}

// --- VIEW (HTML Generation) ---

/**
 * @brief Generates the full HTML page content.
 * @param currentTemp The current local sensor reading.
 * @param latestDbStatus The status message or data retrieved from the API/database.
 * @return The complete HTML string.
 */
String generateHtmlPage(float currentTemp, String latestDbStatus) {
  String html = R"raw(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <script src="https://cdn.tailwindcss.com"></script>
  <title>ESP32 MVC IoT</title>
  <style>
    :root { font-family: 'Inter', sans-serif; }
  </style>
</head>
<body class="bg-gray-100 p-4 md:p-8 min-h-screen flex items-center justify-center">
  <div class="bg-white p-6 md:p-10 rounded-xl shadow-2xl max-w-lg w-full">
    <h1 class="text-3xl font-bold text-center text-indigo-700 mb-6">
      <svg class="inline w-8 h-8 mr-2 text-indigo-500" fill="none" stroke="currentColor" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg"><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M11 4a2 2 0 11-4 0v1a1 1 0 001 1h4a1 1 0 001-1v-1a2 2 0 11-4 0zM17 13v1a1 1 0 001 1h4a1 1 0 001-1v-1a2 2 0 11-4 0zM11 13v1a1 1 0 001 1h4a1 1 0 001-1v-1a2 2 0 11-4 0z"></path></svg>
      ESP32 MVC Dashboard
    </h1>
    <p class="text-sm text-gray-500 text-center mb-8">
      Wi-Fi Status: <span class="text-green-600 font-semibold">Connected (IP: )raw";
      html += WiFi.localIP().toString() + R"raw()</span>
    </p>

    <div class="space-y-6">
      <!-- Local Sensor Card -->
      <div class="bg-indigo-50 border-l-4 border-indigo-500 rounded-lg p-5 shadow-inner">
        <h2 class="text-xl font-semibold text-indigo-700 mb-2">Local Sensor (Controller)</h2>
        <p class="text-4xl font-extrabold text-indigo-900">
          )raw";
          html += String(currentTemp, 1);
          html += R"raw(&deg;C
        </p>
      </div>

      <!-- Database Fetch Card -->
      <div class="bg-purple-50 border-l-4 border-purple-500 rounded-lg p-5 shadow-inner">
        <h2 class="text-xl font-semibold text-purple-700 mb-2">MongoDB Proxy Status (Model)</h2>
        <p class="text-sm font-semibold text-purple-900 break-words">
          )raw";
          html += latestDbStatus;
          html += R"raw(
        </p>
        <p class="text-xs text-gray-500 mt-2">Requires custom Proxy API to function.</p>
      </div>
      
      <!-- Action Form -->
      <form action="/save" method="POST" class="flex justify-between items-center bg-gray-50 p-4 rounded-lg border">
        <label for="temp_input" class="text-gray-700 font-medium">New Reading to Save:</label>
        <div class="flex space-x-2">
          <input type="number" step="0.1" name="temp_input" value=")raw";
          html += String(currentTemp + 1.0, 1); // Suggest a new reading
          html += R"raw(" class="p-2 border border-gray-300 rounded-md w-24 text-center focus:ring-indigo-500 focus:border-indigo-500">
          <button type="submit" class="px-4 py-2 bg-indigo-600 text-white font-semibold rounded-md hover:bg-indigo-700 transition duration-150 shadow-md">
            Save Data (POST)
          </button>
        </div>
      </form>

      <a href="/" class="block text-center mt-4 text-sm text-indigo-500 hover:text-indigo-700 transition duration-150">
        Refresh Dashboard (GET)
      </a>
    </div>
  </div>
</body>
</html>)raw";
  return html;
}

// --- CONTROLLER (Routes & Orchestration) ---

/**
 * @brief Handles the root path ("/"). Orchestrates Model fetching and View generation.
 */
void handleRoot() {
  Serial.println("CONTROLLER: Handling GET request for dashboard.");
  
  // 1. (Controller) Read local data (Simulated here)
  current_temperature = random(200, 300) / 10.0; // Randomly update local temp

  // 2. (Controller calls Model) Fetch data from live API
  String latestDbStatus = fetchDataFromMongoAPI(); 
  
  // 3. (Controller calls View) Generate HTML
  String content = generateHtmlPage(current_temperature, latestDbStatus);

  // 4. (Controller) Send response
  server.send(200, "text/html", content);
}

/**
 * @brief Handles the data saving request (HTTP POST). Orchestrates Model saving and redirects.
 */
void handleSaveData() {
  Serial.println("CONTROLLER: Handling POST request to save data.");
  
  // 1. (Controller) Check if the POST request has the required argument
  if (server.hasArg("temp_input")) {
    float tempToSave = server.arg("temp_input").toFloat();

    // 2. (Controller calls Model) Save the data to live API
    bool success = saveDataToMongoAPI(tempToSave);

    if (success) {
      Serial.printf("CONTROLLER: Data saved successfully: %.1f\n", tempToSave);
    } else {
      Serial.println("CONTROLLER: Data save failed.");
    }
  }

  // 3. (Controller) Redirect the user back to the dashboard (GET /)
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "Redirecting...");
}

void setup() {
  Serial.begin(115200);

  // --- Wi-Fi Setup (Controller Function) ---
  Serial.println("\nCONTROLLER: Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nCONTROLLER: Wi-Fi Connected!");
  Serial.print("CONTROLLER: IP Address: ");
  Serial.println(WiFi.localIP());

  // --- Server Setup (Controller Function) ---
  server.on("/", HTTP_GET, handleRoot);       // GET request for the dashboard
  server.on("/save", HTTP_POST, handleSaveData); // POST request to save data
  server.begin();

  Serial.println("CONTROLLER: HTTP Server started. Ready for requests.");
  randomSeed(analogRead(0)); // Initialize random seed for sensor simulation
}

void loop() {
  // --- Loop (Controller Function) ---
  server.handleClient(); // Process incoming client requests
  delay(1);
}
