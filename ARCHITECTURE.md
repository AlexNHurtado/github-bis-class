# ESP32 REST API Architecture Diagram

## Complete System Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                        USER INTERACTION                          │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                      WEB BROWSER                                 │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │  HTML (index.html)                                       │  │
│  │  - User Interface                                        │  │
│  │  - Buttons and Controls                                  │  │
│  │  - Response Display                                      │  │
│  └──────────────────────────────────────────────────────────┘  │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │  CSS (styles.css)                                        │  │
│  │  - Styling and Layout                                    │  │
│  │  - Responsive Design                                     │  │
│  └──────────────────────────────────────────────────────────┘  │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │  JavaScript (app.js)                                     │  │
│  │  - fetch() API calls                                     │  │
│  │  - Event handlers                                        │  │
│  │  - DOM manipulation                                      │  │
│  └──────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
                              │
                              │ HTTP Request
                              │ (POST /api/led/on)
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                   NODE.JS BACKEND SERVER                         │
│                   (esp32-controller.js)                          │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │  Express.js Framework                                    │  │
│  │  - Route handling                                        │  │
│  │  - Middleware (CORS, JSON)                              │  │
│  │  - Static file serving                                   │  │
│  └──────────────────────────────────────────────────────────┘  │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │  API Endpoints                                           │  │
│  │  - GET  /api/health                                      │  │
│  │  - GET  /api/status                                      │  │
│  │  - POST /api/led/on                                      │  │
│  │  - POST /api/led/off                                     │  │
│  │  - POST /api/led/toggle                                  │  │
│  └──────────────────────────────────────────────────────────┘  │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │  Axios HTTP Client                                       │  │
│  │  - Forward requests to ESP32                            │  │
│  │  - Handle responses                                      │  │
│  │  - Error handling                                        │  │
│  └──────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
                              │
                              │ HTTP Request
                              │ (GET http://ESP32_IP/led/on)
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                      ESP32 DEVICE                                │
│                  (ESP32_REST_Demo.cpp)                           │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │  WiFi Connection                                         │  │
│  │  - Connect to network                                    │  │
│  │  - Get IP address                                        │  │
│  └──────────────────────────────────────────────────────────┘  │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │  WebServer (Port 80)                                     │  │
│  │  - GET  /                                                │  │
│  │  - GET  /led/on                                          │  │
│  │  - GET  /led/off                                         │  │
│  │  - GET  /status                                          │  │
│  │  - POST /led                                             │  │
│  └──────────────────────────────────────────────────────────┘  │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │  GPIO Control                                            │  │
│  │  - digitalWrite(ledPin, HIGH/LOW)                       │  │
│  │  - Read sensor values                                    │  │
│  └──────────────────────────────────────────────────────────┘  │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │  Hardware                                                │  │
│  │  - Built-in LED (GPIO 2)                                │  │
│  │  - WiFi antenna                                          │  │
│  └──────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
```

## Request Flow: Turn LED ON

```
┌──────────┐
│  Step 1  │  User clicks "Turn LED ON" button
└──────────┘
     │
     ▼
┌──────────────────────────────────────────────────────────┐
│ JavaScript:                                               │
│ fetch('/api/led/on', { method: 'POST' })                 │
└──────────────────────────────────────────────────────────┘
     │
     │ HTTP POST /api/led/on
     │ Headers: Content-Type: application/json
     ▼
┌──────────────────────────────────────────────────────────┐
│ Node.js Backend:                                          │
│ app.post('/api/led/on', async (req, res) => {           │
│   // Forward request to ESP32                            │
│   const result = await axios.get('http://ESP32_IP/led/on') │
│ })                                                        │
└──────────────────────────────────────────────────────────┘
     │
     │ HTTP GET http://192.168.1.100/led/on
     ▼
┌──────────────────────────────────────────────────────────┐
│ ESP32:                                                    │
│ void handleLedOn() {                                     │
│   digitalWrite(ledPin, HIGH);                            │
│   server.send(200, "application/json",                   │
│     "{\"success\":true, \"led_state\":true}");          │
│ }                                                         │
└──────────────────────────────────────────────────────────┘
     │
     │ Response: {"success": true, "led_state": true}
     ▼
┌──────────────────────────────────────────────────────────┐
│ Node.js Backend:                                          │
│ // Receive ESP32 response                                │
│ res.json({                                               │
│   success: true,                                         │
│   led_state: true,                                       │
│   ...                                                     │
│ })                                                        │
└──────────────────────────────────────────────────────────┘
     │
     │ Response: {"success": true, "led_state": true, ...}
     ▼
┌──────────────────────────────────────────────────────────┐
│ JavaScript:                                               │
│ const data = await response.json();                      │
│ displayResponse(data);                                   │
│ updateLEDStatus(true);                                   │
└──────────────────────────────────────────────────────────┘
     │
     ▼
┌──────────┐
│  Step 8  │  UI updates to show LED is ON
└──────────┘
```

## Data Flow Diagram

```
┌─────────────┐       ┌──────────────┐       ┌──────────────┐
│   Browser   │       │   Backend    │       │    ESP32     │
│             │       │    API       │       │              │
└──────┬──────┘       └──────┬───────┘       └──────┬───────┘
       │                     │                      │
       │ POST /api/led/on    │                      │
       │────────────────────>│                      │
       │                     │                      │
       │                     │ GET /led/on          │
       │                     │─────────────────────>│
       │                     │                      │
       │                     │                      │ Turn LED ON
       │                     │                      │ digitalWrite()
       │                     │                      │
       │                     │ {"success": true}    │
       │                     │<─────────────────────│
       │                     │                      │
       │ {"success": true}   │                      │
       │<────────────────────│                      │
       │                     │                      │
       │ Update UI           │                      │
       │                     │                      │
```

## Network Ports

```
┌─────────────────────────────────────────────────────┐
│                 Network Configuration                │
└─────────────────────────────────────────────────────┘

Browser                 Backend API              ESP32
   │                       │                       │
   │     Port 3000         │       Port 80         │
   │◄─────────────────────►│◄─────────────────────►│
   │                       │                       │
   │  http://localhost:3000/api/led/on            │
   │                       │                       │
   │                 http://192.168.1.100/led/on   │
   │                       │                       │
```

## HTTP Methods Used

```
┌──────────┬─────────────────────────────────────────┐
│  Method  │  Usage                                   │
├──────────┼─────────────────────────────────────────┤
│   GET    │  Retrieve data (status, health check)   │
│          │  - Safe (no side effects)               │
│          │  - Cacheable                            │
│          │  Example: GET /api/status               │
├──────────┼─────────────────────────────────────────┤
│   POST   │  Trigger actions (LED control)          │
│          │  - Can have side effects                │
│          │  - Not cacheable                        │
│          │  - Can include JSON body                │
│          │  Example: POST /api/led/on              │
└──────────┴─────────────────────────────────────────┘
```

## JSON Data Format

### Request Example (POST with body):
```json
POST /api/led/control
Content-Type: application/json

{
  "state": true
}
```

### Response Example:
```json
HTTP/1.1 200 OK
Content-Type: application/json

{
  "success": true,
  "led_state": true,
  "message": "LED turned ON",
  "device": "ESP32",
  "ip": "192.168.1.100",
  "timestamp": 1234567890
}
```

## CORS (Cross-Origin Resource Sharing)

```
Browser                           Backend Server
   │                                    │
   │  OPTIONS /api/led/on (Preflight)  │
   │───────────────────────────────────>│
   │                                    │
   │  Access-Control-Allow-Origin: *    │
   │  Access-Control-Allow-Methods: GET,POST
   │<───────────────────────────────────│
   │                                    │
   │  POST /api/led/on (Actual Request)│
   │───────────────────────────────────>│
   │                                    │
   │  Response with data                │
   │<───────────────────────────────────│
```

## Technology Stack

```
┌─────────────────────────────────────────────────┐
│                   Frontend                       │
│  HTML5, CSS3, JavaScript (ES6+)                 │
│  Fetch API, DOM Manipulation                    │
└─────────────────────────────────────────────────┘
                    │
                    ▼
┌─────────────────────────────────────────────────┐
│                   Backend                        │
│  Node.js, Express.js                            │
│  Axios, CORS, dotenv                            │
└─────────────────────────────────────────────────┘
                    │
                    ▼
┌─────────────────────────────────────────────────┐
│                   Hardware                       │
│  ESP32 (Espressif)                              │
│  Arduino Framework                              │
│  WebServer Library                              │
└─────────────────────────────────────────────────┘
```

## File Structure

```
microexamples/
│
├── ESP32_REST_Demo.cpp          # ESP32 firmware
│
├── esp32-controller.js          # Node.js backend
├── package.json                 # Node.js dependencies
├── .env                         # Configuration
│
├── public/                      # Web interface
│   ├── index.html              # HTML structure
│   ├── styles.css              # Styling
│   └── app.js                  # JavaScript logic
│
├── led-toggle.js               # CLI controller (interactive)
├── led-simple.js               # CLI controller (simple)
│
├── REST_API_GUIDE.md           # Detailed documentation
├── QUICK_START.md              # Quick start guide
└── ARCHITECTURE.md             # This file
```

## Security Considerations

```
┌──────────────────────────────────────────────────┐
│  Current Setup (Development)                     │
│  - No authentication                             │
│  - No encryption (HTTP, not HTTPS)              │
│  - Open CORS policy                              │
│  - Local network only                            │
└──────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────┐
│  Production Considerations                        │
│  - Add JWT authentication                        │
│  - Use HTTPS (SSL/TLS)                          │
│  - Restrict CORS origins                         │
│  - Rate limiting                                 │
│  - Input validation                              │
│  - API keys                                      │
└──────────────────────────────────────────────────┘
```

## Scalability Path

```
Current:  Browser → Backend → Single ESP32

Future:   
┌─────────┐      ┌──────────┐      ┌────────┐
│ Browser │ ───► │ Backend  │ ───► │ ESP32  │
└─────────┘      │   API    │      │   #1   │
                 │          │      └────────┘
                 │ + Redis  │      ┌────────┐
                 │ + MongoDB│ ───► │ ESP32  │
                 │          │      │   #2   │
                 └──────────┘      └────────┘
                                   ┌────────┐
                              ───► │ ESP32  │
                                   │   #N   │
                                   └────────┘
```