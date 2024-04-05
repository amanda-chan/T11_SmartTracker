#include <ESP32Firebase.h>
#include <WiFi.h>
#include <M5StickCPlus.h>

// WiFi credentials - change to your credentials
const char* ssid = "ssid";
const char* password =  "password";

// Your Firebase project reference url
#define REFERENCE_URL "https://csc2106-t11-default-rtdb.asia-southeast1.firebasedatabase.app/" 

WiFiClient client;
Firebase firebase(REFERENCE_URL);

void setup() {
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
  Serial.print("Start WiFi ..");

  // Loop until the device successfully connects to the WiFi network.
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print("Connecting ..");
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

}

void loop() {
  // Your loop code here
}