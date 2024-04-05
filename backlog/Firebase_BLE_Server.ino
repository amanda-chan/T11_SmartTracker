#include <BLEDevice.h>
#include <BLEServer.h>
#include <ESP32Firebase.h>
#include <WiFi.h>
#include <M5StickCPlus.h>
#include <vector>

// WiFi credentials - change to your credentials
const char* ssid = "ssid";
const char* password =  "password";

// Your Firebase project reference url
#define REFERENCE_URL "https://csc2106-t11-default-rtdb.asia-southeast1.firebasedatabase.app/" 

WiFiClient client;
Firebase firebase(REFERENCE_URL);

// Initiate a vector list to store all the uuids, shelves and inventory from the database
std::vector<String> uuidList;
std::vector<String> shelvesList;

// Unique BLE server name
#define bleServerName "CSC2106-BLE#T11"

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define SERVICE_UUID "f12dac05-634a-4885-a041-9b42578566bf"

bool deviceConnected = false;

// Global variables to store received message
String receivedMessage;

// Vector to store incoming data
std::vector<String> dataVector;

// Setup callbacks onConnect and onDisconnect
class MyServerCallbacks: public BLEServerCallbacks {
public:
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("MyServerCallbacks::Connected...");

    // Start advertising again to allow more clients to joiin
    pServer -> startAdvertising();
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("MyServerCallbacks::Disconnected...");
    
    // Restart advertising
    pServer -> startAdvertising();
  }
};


// Determine what to do when u receive information from the BLE beacons
class MessageCallbacks : public BLECharacteristicCallbacks {
public:
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        Serial.print("Received message: ");
        Serial.println(value.c_str());
        
        // Call the message received callback function
        onMessageReceived(String(value.c_str()));
    }

    // Callback function when a message is received
    void onMessageReceived(const String& message) {
        // Push the received message into the vector
        dataVector.push_back(message);
    }
};

// Function to process the received message
void processReceivedMessage() {
    if (!dataVector.empty()) {
        // Process the next message in the vector
        String message = dataVector.front();
        dataVector.erase(dataVector.begin()); // Remove the processed message
        storeData(message);
    }
}

void getUUIDs() {
  // Get the data from the "uuid_list" node
  String data = firebase.getString("uuid_list");

  // Clear the vector to avoid appending duplicate UUIDs
  uuidList.clear();

  // Define a delimiter to split the string in the UUID list
  char delimiter[] = "\":\"";

  // Use strtok to split the string by delimiter
  char *temp_uuids = strtok((char *)data.c_str(), delimiter);

  // Loop through each pair
  while (temp_uuids != NULL) {
      // Extract UUID from the temp_uuids
      char *uuid = temp_uuids;

      // If it's not the comma separator
      if (strcmp(uuid, ",") != 0) {
          // Store the UUID in the vector
          uuidList.push_back(uuid);
      }

      // Move to the next uuid
      temp_uuids = strtok(NULL, delimiter);
  }
}

void printUUIDList() {
  Serial.println("UUID List:");
  for (const String& uuid : uuidList) {
    Serial.println(uuid);
  }
}

void getShelves() {
  // Get the data from the "shelves" node
  String shelvesData = firebase.getString("shelf_list");

  shelvesList.clear();

  // Define a delimiter to split the string in the shelves list
  char delimiter[] = "\":\"";

  // Use strtok to split the string by delimiter
  char *temp_shelves = strtok((char *)shelvesData.c_str(), delimiter);

  bool firstElement = true;

  // Loop through each pair
  while (temp_shelves != NULL) {
      // Extract shelf from the temp_shelves
      char *shelf = temp_shelves;

      // If it's the first element, add it to the list regardless
      if (firstElement) {
          shelvesList.push_back(shelf);
          firstElement = false; // Update flag for subsequent shelves
      } else {
          // If it's not the first element, check if it's not the comma separator
          if (strcmp(shelf, ",") != 0) {
              shelvesList.push_back(shelf);
          }
      }

      // Move to the next shelf
      temp_shelves = strtok(NULL, delimiter);
  }
}

void printShelvesList() {
  Serial.println("Shelves List:");
  for (const String& shelf : shelvesList) {
    Serial.println(shelf);
  }
}

void storeData(const String& receivedMessage) {
  // Find the position of the first underscore
  int firstUnderscore = receivedMessage.indexOf("_");
  String type = receivedMessage.substring(0, firstUnderscore);

  // Find the position of the second underscore
  int secondUnderscore = receivedMessage.indexOf("_", firstUnderscore + 1);
  String itemName = receivedMessage.substring(firstUnderscore + 1, secondUnderscore);

  if (type == "shelf") {
    int thirdUnderscore = receivedMessage.indexOf("_", secondUnderscore + 1);
    String pointName = receivedMessage.substring(secondUnderscore + 1, thirdUnderscore);
    String xyzCoords = receivedMessage.substring(thirdUnderscore + 1);

    // Store data in Firebase under "shelves" category
    char firebasePath[64];
    snprintf(firebasePath, sizeof(firebasePath), "shelves/%s/%s/xyz_coordinates", itemName.c_str(), pointName.c_str());
    if (firebase.setString(firebasePath, xyzCoords)) {
        Serial.printf("Data stored successfully in Firebase: %s -> %s\n", firebasePath, xyzCoords.c_str());
    } else {
        Serial.println("Error storing data in Firebase");
    }
  } else if (type == "inventory") {
    // Store data in Firebase under "inventory" category
    // Add your logic here
  }
}

void setup() {
  // Start serial communication 
  Serial.begin(115200);

  M5.begin();

  // LCD Setup
  M5.Lcd.setRotation(3); 
  M5.Lcd.fillScreen(BLACK);

  // Begin connecting to the WiFi network with the provided SSID (network name) and password.
  WiFi.begin(ssid, password);

  // Setting the hostname of the device for identification on the network.
  WiFi.setHostname("CSC2106_T11");

  // Print message indicating the start of WiFi connection initialization.
  Serial.println("Start WiFi ..");

  // Loop until the device successfully connects to the WiFi network.
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.println("Connecting ..");
  }

  // Print the local IP address of the device on the M5Stack display.
  M5.Lcd.setCursor(0, 0, 2);
  M5.Lcd.print("IP: ");
  M5.Lcd.println(WiFi.localIP());

  // Testing Firebase connection
  if (firebase.setString("testingConnection", "Connection is working")) {
    Serial.println("String value set to Firebase successfully.");
    M5.Lcd.setCursor(0, 20, 2); 
    M5.Lcd.printf("Connected to Firebase successfully", 0);
    firebase.deleteData("testingConnection"); // Delete from database once successful

  } else {
    Serial.println("Error setting string value to Firebase.");
    M5.Lcd.setCursor(0, 20, 2); 
    M5.Lcd.printf("Unable to connect to Firebase", 0);
  }

  // Retrieve and print UUIDs from the Firebase
  getUUIDs();
  printUUIDList();

  // Retrieve and print shelves from the Firebase
  getShelves();
  printShelvesList();


  // Create the BLE Device
  BLEDevice::init(bleServerName);

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *bleService = pServer->createService(SERVICE_UUID);

  // Create BLE characteristics dynamically based on the UUID list
  for (const String& uuid : uuidList) {
      BLECharacteristic *pCharacteristic = new BLECharacteristic(uuid.c_str(), BLECharacteristic::PROPERTY_NOTIFY);
      BLEDescriptor *pDescriptor = new BLEDescriptor(BLEUUID((uint16_t)0x2902));
      bleService->addCharacteristic(pCharacteristic);
      pDescriptor->setValue(uuid.c_str());
      pCharacteristic->addDescriptor(pDescriptor);
      pCharacteristic->setCallbacks(new MessageCallbacks());
  }

  // Start the service
  bleService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");

}

void loop() {
    // Process the received message if a new one is available
    processReceivedMessage();

    // Your other loop code here
}
