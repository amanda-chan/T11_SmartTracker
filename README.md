# T11_SmartTracker

## For the BLE beacons:

### Instructions for Changing WiFi Credentials, RSSI Thresholds, and Shelf Name in Firebase_BLE_Shelf.ino

If you need to change the WiFi credentials, RSSI thresholds, or shelf name in the provided code, follow these steps:

1. **WiFi Credentials:**
    - Locate the following lines in the code:
        ```cpp
        // WiFi credentials - change to your credentials
        const char* ssid = "ssid";
        const char* password =  "password";
        ```
    - Replace `"ssid"` with your WiFi network name (SSID) and `"password"` with your WiFi password.

2. **RSSI Thresholds:**
    - Find the lines:
        ```cpp
        // Define the RSSI threshold
        #define SHELF_THRESHOLD -60
        #define TOP_SHELF_THRESHOLD -44
        ```
    - Adjust the values assigned to `SHELF_THRESHOLD` and `TOP_SHELF_THRESHOLD` according to your requirements. These thresholds determine when a device is considered to be on a particular shelf based on its received signal strength indicator (RSSI).

3. **Shelf Name:**
    - Look for the line:
        ```cpp
        // Define a unique shelf name
        #define shelfName "makeup"
        ```
    - Change `"makeup"` to your desired shelf name. This name will be used to identify the shelf in the database.

4. **Optional: Firebase Project Reference URL:**
    - If you're using a different Firebase project, you can change the reference URL:
        ```cpp
        // Your Firebase project reference url
        #define REFERENCE_URL "https://csc2106-t11-default-rtdb.asia-southeast1.firebasedatabase.app/"
        ```
    - Replace the URL `"https://csc2106-t11-default-rtdb.asia-southeast1.firebasedatabase.app/"` with the URL of your Firebase project.

5. Save the changes to the file.

6. Compile and upload the modified code to your device.


### Instructions for Changing Inventory Name and UUID in BLE_Inventory.INO

If you need to change the inventory name or UUID in the `BLE_Inventory.INO` file, follow these steps:

1. Open the `BLE_Inventory.INO` file in your preferred code editor or Arduino IDE.

2. Locate the following lines in the code:

    ```cpp
    // Unique inventory name
    #define inventoryName "lipstick123"
    
    // See the following for generating UUIDs:
    // https://www.uuidgenerator.net/
    #define INVENTORY_UUID "c0bd4818-9ba8-4ab9-8586-42c8c09500c2"
    ```

3. To change the inventory name, modify the value assigned to `inventoryName`. Replace `"lipstick123"` with your desired inventory name enclosed in double quotes.

4. To change the UUID, modify the value assigned to `INVENTORY_UUID`. Ensure that the UUID follows the UUID format (e.g., `"xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"`). You can generate a new UUID using tools like [UUID Generator](https://www.uuidgenerator.net/).

5. Save the changes to the file.

6. Compile and upload the modified code to your M5StickC Plus device.


## For the web server:
1. Download Platform IO IDE extension on vscode.
![image](https://github.com/amanda-chan/T11_SmartTracker/assets/64049670/37509c04-a3ed-40bf-a2a0-b46c75c6c9ae)
2. Open the extension and select new project. Place the main.cpp file into src and dashboard.html into data folders.
![image](https://github.com/amanda-chan/T11_SmartTracker/assets/64049670/80d1270e-1f5e-45c4-a0e4-29015aef6347)
3. Download the following libraries from PIO Home.
![image](https://github.com/amanda-chan/T11_SmartTracker/assets/64049670/e6e5ef14-02a3-4bd5-9cc3-cd55da8ce9f6)
4. Click into the extension toolbar and upload filesystem image into m5stick
![image](https://github.com/amanda-chan/T11_SmartTracker/assets/64049670/90ea1f37-26ca-4738-ae62-089ab01806ff)
5. Build and upload.
