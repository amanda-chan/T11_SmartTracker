#include <BLEDevice.h>
#include <BLEServer.h>
#include <M5StickCPlus.h>

// Unique inventory name
#define inventoryName "lipstick123"

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define INVENTORY_UUID "c0bd4818-9ba8-4ab9-8586-42c8c09500c2"

bool deviceConnected = false;

BLEServer *pServer; // Declare BLE server globally

// Setup callbacks onConnect and onDisconnect
class MyServerCallbacks: public BLEServerCallbacks {
public:
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("MyServerCallbacks::Connected...");
    // Continuously advertise itself
    pServer -> startAdvertising();
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("MyServerCallbacks::Disconnected...");
    
    // Restart advertising
    pServer -> startAdvertising();
  }
};

void setup() {
  // Start serial communication 
  Serial.begin(115200);

  M5.begin();

  // LCD Setup
  M5.Lcd.setRotation(3); 
  M5.Lcd.fillScreen(BLACK);

  M5.Lcd.setCursor(0, 0, 2);
  M5.Lcd.print("BLE Inventory: ");
  M5.Lcd.println(inventoryName);
  
  // Create the BLE Device
  BLEDevice::init(inventoryName);

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(INVENTORY_UUID);
  pServer->getAdvertising()->start();

}

void loop() {

}
