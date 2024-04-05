#include <BLEDevice.h>
#include <BLEServer.h>
#include <M5StickCPlus.h>

// Unique BLE server name
#define bleServerName "CSC2106-BLE#T11"

// BLE Service UUID
static BLEUUID bleServiceUUID("f12dac05-634a-4885-a041-9b42578566bf");

// BLE Characteristic UUID for sending message
static BLEUUID messageCharacteristicUUID("c0bd4818-9ba8-4ab9-8586-42c8c09500c2");

// Flags stating if should begin connecting and if the connection is up
static boolean doConnect = false;
static boolean connected = false;

// Address of the peripheral device. Address will be found during scanning...
static BLEAddress *pServerAddress;

// Remote characteristic for sending message
static BLERemoteCharacteristic* messageCharacteristic;

// Edit this accordingly
// Format: type (shelf or inventory), name of shelf/ inventory, which point (applicable to shelf only - pointA, pointB, pointC, pointD)
// e.g. shelf_makeup_pointA
String messageFormat = "shelf_makeup_pointA";

// String message to send
String messageToSend = "";

// Store the accelerometer data
float ax, ay, az;

// Callback function that gets called, when another device's advertisement has been received
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.getName() == bleServerName) { // Check if the name of the advertiser matches
      advertisedDevice.getScan()->stop(); // Scan can be stopped, we found what we are looking for
      pServerAddress = new BLEAddress(advertisedDevice.getAddress()); // Address of advertiser is the one we need
      doConnect = true; // Set indicator, stating that we are ready to connect
      Serial.println("Device found. Connecting!");
    }
    else
      Serial.print(".");
  }
};

// Function to connect to the BLE Server that has the name, Service, and Characteristics
bool connectToServer(BLEAddress pAddress) {
  BLEClient* pClient = BLEDevice::createClient();

  // Connect to the remote BLE Server.
  pClient->connect(pAddress);
  Serial.println(" - Connected to server");

  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService* pRemoteService = pClient->getService(bleServiceUUID);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(bleServiceUUID.toString().c_str());
    return (false);
  }

  // Obtain a reference to the characteristic in the service of the remote BLE server.
  messageCharacteristic = pRemoteService->getCharacteristic(messageCharacteristicUUID);

  if (messageCharacteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID");
    return false;
  }
  Serial.println(" - Found our characteristic");

  return true;
}

// Function to send a message to the BLE Server
void sendMessageToServer(String message) {
  if (connected) {
    messageCharacteristic->writeValue(message.c_str(), message.length());
    Serial.println("Message sent to server: " + message);
  }
}

void setup() {
  // Start serial communication
  Serial.begin(115200);
  Serial.println("Starting BLE Client: Makeup Shelf Point A");

  M5.begin();
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0, 2);
  M5.Lcd.printf("BLE Client: Makeup Shelf Point A", 0);

  // Initialize IMU sensor
  int x = M5.IMU.Init(); //return 0 is ok, return -1 is unknown
  if(x != 0)
    Serial.println("IMU initialisation failed!"); 
    M5.Lcd.setCursor(0, 20, 2);
    M5.Lcd.printf("IMU initialization failed!", 0);

  M5.Lcd.setCursor(0, 20, 2);
  M5.Lcd.printf("IMU initialization successful!", 0);

  // Init BLE device
  BLEDevice::init("");

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 30 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(30);

}

void loop() {
  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server that we want to connect.  Now we connect to it.  Once we are
  // connected we set the connected flag to be true.
  if (doConnect == true) {
    if (connectToServer(*pServerAddress)) {
      Serial.println("Connected to the BLE Server.");
      M5.Lcd.setCursor(0, 40, 2);
      M5.Lcd.printf("Connected to the BLE Server.", 0);
      connected = true;
    } else {
      Serial.println("Failed to connect to the server; Restart device to scan for nearby BLE server again.");
      M5.Lcd.setCursor(0, 40, 2);
      M5.Lcd.printf("Failed to connected to the BLE Server.", 0);
    }
    doConnect = false;
  }

  // Check if connected to the server and if 10 seconds have passed
  // Send a message every 10 seconds
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  if (connected && currentMillis - previousMillis >= 10000) {
      messageToSend = "";
      // Update message with accelerometer data
      updateMessageWithAccelerometerData();
      // Send message to the server
      sendMessageToServer(messageToSend);
      // Update the previousMillis
      previousMillis = currentMillis;
  }

}

bool readAccelerometer() {
  M5.IMU.getAccelData(&ax, &ay, &az);
  return true;
}

// Function to read the accelerometer data and update the message
void updateMessageWithAccelerometerData() {
  // Read accelerometer data
  if (readAccelerometer()) {
    // Format the accelerometer data
    String accelerometerData = String(ax, 3) + "_" + String(ay, 3) + "_" + String(az, 3);
    // Construct the complete message
    messageToSend = messageFormat + "_" + accelerometerData;
  }
}
