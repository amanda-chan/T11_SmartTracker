#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <M5StickCPlus.h>
#include "SPIFFS.h"
#include <ArduinoJson.h>
#include <ESP32Firebase.h>
#include <map>

/* Put your SSID & Password */
const char *ssid = "ssid";
const char *password = "password";

// Your Firebase project reference url
#define REFERENCE_URL "https://csc2106-t11-default-rtdb.asia-southeast1.firebasedatabase.app/"
Firebase firebase(REFERENCE_URL);

WebServer server(80);

// Fetch the data from the database, and store each inventory separately
std::vector<std::vector<std::string>> retrieveDatabaseData()
{
  // Fetch data from Firebase and convert it to a C-style string
  const char *firebaseData = firebase.getString("/inventory").c_str();

  // Convert the C-style string to an std::string
  std::string input(firebaseData);

  // Parse the input string
  std::vector<std::vector<std::string>> result;

  // Temporary vector to store elements for each item
  std::vector<std::string> tempList;

  // Parse the input string
  std::string temp;
  bool insideItem = false;
  for (char c : input)
  {
    if (c == '{')
    {
      if (!temp.empty())
      { // Store the previous item
        tempList.push_back(temp);
        temp.clear();
      }
      insideItem = true;
    }
    else if (c == '}')
    {
      insideItem = false;
      // Store the last part of the item
      tempList.push_back(temp);
      temp.clear();
      // Add tempList to the result
      result.push_back(tempList);
      // Clear tempList for the next item
      tempList.clear();
    }
    else
    {
      temp += c;
    }
  }

  return result;
}

// To extract the result for the HTML
String extractData()
{
  // Get the result from the database
  std::vector<std::vector<std::string>> result = retrieveDatabaseData();

  // To store the variable for html
  String rowDataHTML;

  // Print the result to the serial monitor
  for (auto &item : result)
  {
    Serial.print("[");
    bool isFirstElement = true;
    for (auto &value : item)
    {
      // Remove the comma
      if (value[0] == ',')
      {
        value = value.substr(1);
      }

      // When it is the first element (E.g. "abc":)
      if (isFirstElement)
      {
        // Extract the name of the inventory
        value = value.substr(1, value.size() - 3);
        // Add to the html row data
        rowDataHTML = rowDataHTML + "<tr><td>" + value.c_str() + "</td>";
        //
        Serial.println(value.c_str());
        // Update that it is the next variable
        isFirstElement = false;
      }
      // When it second element (E.g. "location":"unknown","shelf":"unknown")
      else
      {
        // Find the comma index
        size_t commaIndex = value.find(',');
        // Serial.println(commaIndex);

        // Extract the location portion
        // 12 - ignore first 11 characters "location": and start from the 12th characters
        // commaIndex-13 - Length to extract (until commaIndex, exclude "location":" and the final ")
        String location = value.substr(12, commaIndex - 13).c_str();

        // Extract the shelf portion
        // commaIndex+10 - Start after comma and "shelf":"
        // value.size()-commaIndex-11 - Length to extract (whole second element length, ignore everything before comma, ignore "shelf: and the final ")
        String shelf = value.substr(commaIndex + 10, value.size() - commaIndex - 11).c_str();

        // Add to the html row data
        rowDataHTML = rowDataHTML + "<td>" + shelf + "</td><td>" + location + "</td></tr>";

        // Debug code
        Serial.println(location);
        Serial.println(shelf);
      }
    }
    return rowDataHTML;
  }
}

// Read the HTML current file
String readHTMLContent()
{
    // Check if SPIFFS is initialized
  if (!SPIFFS.begin())
  {
    Serial.println("Failed to initialize SPIFFS");
    server.send(500, "text/plain", "Failed to initialize SPIFFS");
  }

  // Check if the file exists
  if (SPIFFS.exists("/dashboard.html"))
  {
    Serial.println("HTML file exists");
  }
  else
  {
    Serial.println("HTML file does not exist");
    server.send(500, "text/plain", "HTML file not found");
  }

  // Read the HTML file
  File htmlFile = SPIFFS.open("/dashboard.html", "r");
  if (!htmlFile)
  {
    Serial.println("Failed to open dashboard.html file");
    server.send(500, "text/plain", "Failed to open HTML file");
  }

  // Print a new line
  Serial.println();

  // Reset file pointer to the beginning
  htmlFile.seek(0);

  // Read the contents of the HTML file as a String
  String htmlContent = htmlFile.readString();
  htmlFile.close();

  return htmlContent;
}

// Output the HTML file and replace the data accordingly.
void dashboard()
{
  // Variable to store the formatted HTML with extracted data
  String rowDataHTML = extractData();

  // Read the HTML file
  String htmlContent = readHTMLContent();

  // Debugging
  Serial.println(rowDataHTML);

  // Replace the placeholder {$rowData} with actual data
  htmlContent.replace("$rowCode$", String(rowDataHTML));

  // Send the modified HTML content as the server response
  server.send(200, "text/html", htmlContent);
  Serial.println("Sensor data sent successfully");
}

// Handle not found endpoints
void handle_NotFound()
{
  server.send(404, "text/plain", "Not found");
}

void loop()
{
  server.handleClient();
}

void setup()
{
  // Initialise serial communication
  Serial.begin(115200);

  // Initialise M5StickC Plus
  // put your setup code here, to run once:
  M5.begin();

  // Initialise IMU (Inertial Measurement Unit)
  int x = M5.IMU.Init(); // Return 0 is ok, return -1 is unknown
  if (x != 0)
    Serial.println("IMU initialisation fail!");

  // Set LCD Rotation and clear screen
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(BLACK);
  // M5.Lcd.setCursor(0, 0, 2);
  M5.Lcd.printf("RESTful API", 0);

  // Testing Firebase connection
  if (firebase.setString("testingConnection", "Connection is working"))
  {
    Serial.println("String value set to Firebase successfully.");
    M5.Lcd.setCursor(0, 20, 2);
    M5.Lcd.printf("Connected to Firebase successfully.", 0);
    firebase.deleteData("testingConnection"); // Delete from database once successful
  }
  else
  {
    M5.Lcd.setCursor(0, 20, 2);
    M5.Lcd.printf("Unable to connect to Firebase.", 0);
    Serial.println("Error setting string value to Firebase.");
  }

  // Initialize SPIFFS
  SPIFFS.begin();

  if (!SPIFFS.begin(true))
  {
    Serial.println("SPIFFS initialization failed");
    return;
  }

  // Connect to WiFi Network
  WiFi.begin(ssid, password);

  // Setting the hostname
  WiFi.setHostname("group01-stick");

  // Wait for Wifi Connection
  Serial.print("Start WiFi ..");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print("Connecting ..");
  }

  // Display IP Address on M5Stick LCD
  M5.Lcd.setCursor(0, 40, 2);
  M5.Lcd.print("IP: ");
  M5.Lcd.println(WiFi.localIP());

  // Route for handling sensor data request
  server.on("/dashboard", HTTP_GET, dashboard);

  // Start server
  server.begin();
  Serial.println("HTTP server started");
  server.onNotFound(handle_NotFound); // Handle not found endpoints

  // Start HTTP server
  server.begin();
  Serial.println("HTTP server started");
  Serial.print("Connected to the WiFi network. IP: ");
  Serial.println(WiFi.localIP());
}
