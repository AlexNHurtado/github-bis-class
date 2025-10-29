#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Define UUIDs for the Service and Characteristic
// Use a UUID generator online to create unique ones for production
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLECharacteristic *pCharacteristic; // Pointer to the characteristic object

// Connection callback class
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      Serial.println("A client connected via BLE!");
    };

    void onDisconnect(BLEServer* pServer) {
      Serial.println("Client disconnected. Starting advertising again.");
      BLEDevice::startAdvertising(); // Restart advertising to allow re-connection
    }
};

// Characteristic callback class for handling Read/Write events
class MyCharacteristicCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        Serial.print("Received Value: ");
        for (int i = 0; i < rxValue.length(); i++)
          Serial.print(rxValue[i]);

        Serial.println();
      }
    }
    
    void onRead(BLECharacteristic *pCharacteristic) {
      // Data to be sent when the client reads the characteristic
      pCharacteristic->setValue("Hello from ESP32!");
      Serial.println("Characteristic was read by the client.");
    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Starting ESP32 BLE Server...");

  // 1. Initialize BLE and set device name
  BLEDevice::init("ESP32 BLE Demo");

  // 2. Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // 3. Create the Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // 4. Create the Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_WRITE |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  // Set the characteristic's initial value and callbacks
  pCharacteristic->setValue("Initial Value");
  pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());

  // Add a standard descriptor (optional, but good practice)
  pCharacteristic->addDescriptor(new BLE2902());

  // 5. Start the Service
  pService->start();

  // 6. Start advertising (making the device discoverable)
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06); // Help iOS connect faster
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  
  Serial.println("BLE advertising started. Use a mobile app to scan for 'ESP32 BLE Demo'.");
}

void loop() {
  // Put a slight delay to allow background BLE operations
  delay(100); 
}
