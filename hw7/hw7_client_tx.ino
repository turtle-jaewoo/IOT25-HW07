#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>


#define TARGET_SERVER_NAME "MyESP32_Distance_Server" 

int scanTime = 2;
BLEScan* pBLEScan;

long rssiSum = 0;
int rssiCount = 0;
const int SAMPLES_FOR_AVERAGE = 20; 

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
     
      if (advertisedDevice.haveName() && advertisedDevice.getName() == TARGET_SERVER_NAME) {
        
        if (advertisedDevice.haveRSSI()) {
          int currentRssi = advertisedDevice.getRSSI();
          Serial.print("Found Target Server: ");
          Serial.print(advertisedDevice.getName().c_str());
          Serial.print(" | RSSI: ");
          Serial.print(currentRssi);
          Serial.println(" dBm");

          
          rssiSum += currentRssi;
          rssiCount++;
          if (rssiCount >= SAMPLES_FOR_AVERAGE) {
            double averageRssi = (double)rssiSum / rssiCount;
            Serial.print(">>> Average RSSI over ");
            Serial.print(rssiCount);
            Serial.print(" samples: ");
            Serial.print(averageRssi, 2);
            Serial.println(" dBm <<<  THIS IS YOUR approx. txPower at 1m");
            rssiSum = 0; 
            rssiCount = 0; 
          }

        } else {
          Serial.println("Found Target Server, but no RSSI value available.");
        }
        
      }
    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("RSSI Logger for txPower Calibration");
  Serial.println("Place this ESP32 Client 1 meter away from the ESP32 Server.");
  Serial.println("Collect multiple RSSI readings and average them to determine your txPower.");
  Serial.println("----------------------------------------------------------------------");

  BLEDevice::init(""); 
  pBLEScan = BLEDevice::getScan(); 
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); 
  pBLEScan->setInterval(100);    
  pBLEScan->setWindow(99);     
}

void loop() {
  Serial.println("Starting BLE scan to find server and log RSSI...");
  BLEScanResults* pFoundDevices = pBLEScan->start(scanTime, false); 
  pBLEScan->clearResults(); 
  
  delay(2000); 
}
