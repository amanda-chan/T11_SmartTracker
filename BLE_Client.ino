#include "BLEDevice.h"
#include <M5StickCPlus.h>

// change the BLE Server name to connect to
#define bleServerName "CSC2106-BLE#01-IOT"

/* UUID's of the service, characteristic that we want to read*/
// BLE Service
static BLEUUID bleServiceUUID("0123a567-0123-4567-89ab-0123456789ab");

// BLE Characteristics
static BLEUUID coordinatesCharacteristicUUID("0123a567-0123-4567-89ab-0123456789bc");
static BLERemoteCharacteristic* coordinatesCharacteristic;

// Coordinates data
String coordinatesData;

// Flags for connection status
static boolean doConnect = false;
static boolean connected = false;
static BLEAddress *pServerAddress;

// Function to connect to the BLE server
bool connectToServer(BLEAddress pAddress) {
  BLEClient* pClient = BLEDevice::createClient();
  pClient->connect(pAddress);
  
  BLERemoteService* pRemoteService = pClient->getService(bleServiceUUID);
  if (pRemoteService == nullptr) {
    return false;
  }
  
  coordinatesCharacteristic = pRemoteService->getCharacteristic(coordinatesCharacteristicUUID);
  if (coordinatesCharacteristic == nullptr) {
    return false;
  }

  return true;
}

// Function to read accelerometer data
void readAccelerometerData() {
  float x = M5.IMU.getAccelData(0);
  float y = M5.IMU.getAccelData(1);
  float z = M5.IMU.getAccelData(2);
  // Format data into a string
  coordinatesData = String(x, 4) + "," + String(y, 4) + "," + String(z, 4);
}

void setup() {
  Serial.begin(115200);
  M5.begin();

  // Init BLE device
  BLEDevice::init("");

  // Retrieve a Scanner and set the callback to be informed when a new device is detected
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(30); // Scan for 30 seconds

  // Set up the accelerometer
  M5.IMU.Init();

  // Set up the OLED display
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0, 2);
  M5.Lcd.printf("BLE Client", 0);
}

void loop() {
  // If the flag "doConnect" is true then connect to the server
  if (doConnect == true) {
    if (connectToServer(*pServerAddress)) {
      Serial.println("Connected to the BLE Server.");
      connected = true;
    } else {
      Serial.println("Failed to connect to the server. Restart device to scan for nearby BLE servers again.");
    }
    doConnect = false;
  }

  // If connected to the server, send accelerometer data
  if (connected) {
    readAccelerometerData();
    if (coordinatesCharacteristic != nullptr) {
      coordinatesCharacteristic->writeValue(coordinatesData.c_str(), coordinatesData.length());
      Serial.println("Sent accelerometer data: " + coordinatesData);
    } else {
      Serial.println("Failed to find coordinates characteristic.");
    }
    delay(1000); // Send data every second (adjust as needed)
  }

  // Handle BLE events
  BLEDevice::handleEvents();
}

// Callback function for receiving advertisement from BLE devices
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.getName() == bleServerName) {
      advertisedDevice.getScan()->stop(); // Stop scanning
      pServerAddress = new BLEAddress(advertisedDevice.getAddress());
      doConnect = true; // Set flag to connect to the server
      Serial.println("Device found. Connecting...");
    }
  }
};