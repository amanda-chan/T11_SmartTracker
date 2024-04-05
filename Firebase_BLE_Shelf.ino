#include <BLEDevice.h>
#include <BLEServer.h>
#include <M5StickCPlus.h>
#include <ESP32Firebase.h>
#include <WiFi.h>

// WiFi credentials - change to your credentials
const char* ssid = "ssid";
const char* password =  "password";

// Define a structure to hold the update information
struct LocationUpdate {
    std::string deviceName;
    std::string shelf;
    std::string location;
};

// Create a vector to store the location updates
std::vector<LocationUpdate> updateVector;

// Define the RSSI threshold
#define SHELF_THRESHOLD -60
#define TOP_SHELF_THRESHOLD -44

// Define a unique shelf name
#define shelfName "makeup"

// Your Firebase project reference url
#define REFERENCE_URL "https://csc2106-t11-default-rtdb.asia-southeast1.firebasedatabase.app/" 

WiFiClient client;
Firebase firebase(REFERENCE_URL);

// Initiate a vector list to store all the inventory uuids
std::vector<String> uuidList;

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    // Extract UUID
    std::string deviceUUID = advertisedDevice.getServiceUUID().toString();

    // Convert std::string to String
    String deviceUUIDString = String(deviceUUID.c_str());

    // Check if the UUID is in the UUID list
    if (std::find(uuidList.begin(), uuidList.end(), deviceUUIDString) != uuidList.end()) {
      // UUID found in the list, print the device details
      std::string deviceName = advertisedDevice.getName();
      int rssi = advertisedDevice.getRSSI();
      Serial.printf("Advertised Device found: Name: %s, RSSI: %d\n", deviceName.c_str(), rssi);
      std::string currentShelf;
      std::string currentLocation;
      // Determine the current location based on RSSI
      if (rssi > SHELF_THRESHOLD) {
        currentShelf = shelfName;
        if (rssi > TOP_SHELF_THRESHOLD) {
          currentLocation = "Top Part";
        } else {
          currentLocation = "Bottom Part";
        }
      } else {
        currentShelf = "unknown";
        currentLocation = "unknown";
      }

      // Print the current shelf and location
      Serial.printf("Current Shelf: %s, Current Location: %s\n", currentShelf.c_str(), currentLocation.c_str());

      // Enqueue the location update with the device name
      updateVector.push_back({deviceName, currentShelf, currentLocation});
    }
  }
};

// Function to retrieve the last known shelf from the database
std::string getLastKnownShelfFromDatabase(const std::string& deviceName) {
    String path = "inventory/" + String(deviceName.c_str()) + "/shelf";
    return firebase.getString(path).c_str();
}

// Function to update the shelf in the database
void updateShelfInDatabase(const std::string& deviceName, const std::string& shelf) {
    String path = "inventory/" + String(deviceName.c_str()) + "/shelf";
    if (!firebase.setString(path, String(shelf.c_str()))) {
        Serial.println("Failed to update shelf in database");
    } else {
        Serial.println("Shelf updated successfully");
    }
}

// Function to retrieve the last known location from the database
std::string getLastKnownLocationFromDatabase(const std::string& deviceName) {
    String path = "inventory/" + String(deviceName.c_str()) + "/location";
    return firebase.getString(path).c_str();
}

// Function to update the location in the database
void updateLocationInDatabase(const std::string& deviceName, const std::string& location) {
    String path = "inventory/" + String(deviceName.c_str()) + "/location";
    if (!firebase.setString(path, String(location.c_str()))) {
        Serial.println("Failed to update location in database");
    } else {
        Serial.println("Location updated successfully");
    }
}

void processLocationUpdates() {
    for (const auto& update : updateVector) {
        // Retrieve the last known shelf and location from the database using the device name
        std::string lastKnownShelf = getLastKnownShelfFromDatabase(update.deviceName);
        std::string lastKnownLocation = getLastKnownLocationFromDatabase(update.deviceName);

        // Update the database if the shelf or location has changed
        if (update.shelf == "unknown" && update.location == "unknown" && lastKnownShelf == shelfName) {
            updateShelfInDatabase(update.deviceName, update.shelf);
            updateLocationInDatabase(update.deviceName, update.location);

        } else if (update.shelf != "unknown" && update.shelf != lastKnownShelf) {
            updateShelfInDatabase(update.deviceName, update.shelf);

        } else if (update.location != "unknown" && update.location != lastKnownLocation) {
          updateLocationInDatabase(update.deviceName, update.location);

        }
    }
    // Clear the vector after processing all updates
    updateVector.clear();
}

void getUUIDs() {
  // Get the data from the "inventory_uuid" node
  String data = firebase.getString("inventory_uuid");

  // Clear the vector to avoid appending duplicate UUIDs
  uuidList.clear();

  // Define a delimiter to split the string in the UUID list
  char delimiter[] = "\":\"";

  // Use strtok to split the string by delimiter
  char *temp_uuids = strtok((char *)data.c_str(), delimiter);

  bool firstElement = true;

  // Loop through each pair
  while (temp_uuids != NULL) {
      // Extract UUID from the temp_uuids
      char *uuid = temp_uuids;

      // If it's the first element, add it to the list regardless
      if (firstElement) {
        uuidList.push_back(uuid);
        firstElement = false;
      } else {
        // If it's not the comma separator
        if (strcmp(uuid, ",") != 0) {
            // Store the UUID in the vector
            uuidList.push_back(uuid);
        }
      }

      // Move to the next uuid
      temp_uuids = strtok(NULL, delimiter);
  }
}

void printUUIDList() {
  Serial.println("Inventory UUID List:");
  for (const String& uuid : uuidList) {
    Serial.println(uuid);
  }
}


void setup() {
  // Start serial communication
  Serial.begin(115200);
  Serial.println("Starting BLE Scanner: Makeup Shelf");

  M5.begin();
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0, 2);
  M5.Lcd.printf("BLE Scanner: Makeup Shelf", 0);

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

  // Testing Firebase connection
  if (firebase.setString("testingConnection", "Connection is working")) {
    Serial.println("String value set to Firebase successfully.");
    M5.Lcd.setCursor(0, 20, 2);
    M5.Lcd.printf("Connected to Firebase successfully.", 0);
    firebase.deleteData("testingConnection"); // Delete from database once successful
  } else {
    M5.Lcd.setCursor(0, 20, 2);
    M5.Lcd.printf("Unable to connect to Firebase.", 0);
    Serial.println("Error setting string value to Firebase.");
  }

  // Retrieve and print inventory UUIDs from the Firebase
  getUUIDs();
  printUUIDList();

  // Init BLE device
  BLEDevice::init("");
}

void loop() {
  // Continuously scan for devices
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  
  while (true) {
    pBLEScan->start(5); // Scan for 5 seconds
    delay(1000); // Delay before the next scan

    // Process location updates after each scan
    processLocationUpdates();
  }
}
