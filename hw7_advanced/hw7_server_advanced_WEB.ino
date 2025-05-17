#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <math.h>    
#include <WiFi.h>   
#include <WebServer.h> 

#define TARGET_SERVER_NAME "MyESP32_Distance_Server" 
const double txPower = -68.5;             
const double pathLossExponentN = 4.0;       
int scanTimeBLE = 2;                      
BLEScan* pBLEScan;

const char* ssid = "AndroidHotspot6948";      
const char* password = "tksdkr12##"; 
WebServer webServer(80);                  

volatile double currentDistance = -1.0;   
volatile int lastRssiValue = 0;          
volatile bool serverFoundInLastScan = false; 


class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        if (advertisedDevice.haveName() && advertisedDevice.getName() == TARGET_SERVER_NAME) {
            serverFoundInLastScan = true; 
            if (advertisedDevice.haveRSSI()) {
                lastRssiValue = advertisedDevice.getRSSI();
                currentDistance = pow(10.0, (txPower - (double)lastRssiValue) / (10.0 * pathLossExponentN));
                
                Serial.print("Found Server. RSSI: "); Serial.print(lastRssiValue);
                Serial.print(" dBm, Calculated Distance: "); Serial.print(currentDistance, 2); Serial.println(" m");
            } else {
                currentDistance = -2.0; 
                lastRssiValue = 0;
                Serial.println("Found Server, but no RSSI value available.");
            }
        }
    }
};


void handleRoot() {
    String html = "<!DOCTYPE HTML><html><head><title>ESP32 Distance</title>";
    html += "<meta http-equiv='refresh' content='3'>"; 
    html += "<style>body{font-family:sans-serif; text-align:center; margin-top:40px;}";
    html += "h1{color:#007bff;} .val{font-size:2.5em; font-weight:bold; margin:10px;}";
    html += ".rssi{font-size:1.2em; color:gray;}</style></head><body>";
    html += "<h1>ESP32 BLE Distance Monitor</h1>";

    if (currentDistance == -2.0) {
        html += "<p class='val'>No RSSI from Server</p>";
    } else if (currentDistance >= 0) {
        html += "<p class='val'>" + String(currentDistance, 2) + " m</p>";
        html += "<p class='rssi'>RSSI: " + String(lastRssiValue) + " dBm</p>";
    } else { // currentDistance == -1.0 
        html += "<p class='val'>Scanning for server...</p>";
         if(lastRssiValue !=0) { 
            html += "<p class='rssi'>(Last known RSSI: " + String(lastRssiValue) + " dBm)</p>";
        }
    }
    html += "</body></html>";
    webServer.send(200, "text/html", html);
}

void setup() {
    Serial.begin(115200);
    Serial.println("\nESP32 BLE Client - Web Server Version");

    BLEDevice::init("");
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(131); 
    pBLEScan->setWindow(101); 

    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
    WiFi.mode(WIFI_STA); 
    WiFi.begin(ssid, password);

    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 20) { 
        delay(500);
        Serial.print(".");
        retries++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi Connected!");
        Serial.print("IP Address: http://");
        Serial.println(WiFi.localIP());

        webServer.on("/", HTTP_GET, handleRoot); 
        webServer.begin();                       
        Serial.println("Web Server started. Waiting for requests...");
    } else {
        Serial.println("\nFailed to connect to WiFi. Web Server will not be available.");
       
    }
}

void loop() {
    serverFoundInLastScan = false; 
    // Serial.println("Starting new BLE scan cycle..."); 

    BLEScanResults* pFoundDevices = pBLEScan->start(scanTimeBLE, false); 

    if (!serverFoundInLastScan && currentDistance != -2.0) { 
        
        Serial.println("Target server NOT found in this scan cycle.");
    }
    
    pBLEScan->clearResults(); 

    if (WiFi.status() == WL_CONNECTED) {
        webServer.handleClient();
    }

   
    delay(200); 
}
