#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <math.h> 

#define TARGET_SERVER_NAME "MyESP32_Distance_Server"

const double txPower = -80.0;     
const double pathLossExponentN = 4.0; 

int scanTime = 2; 
BLEScan* pBLEScan;

#define RSSI_FILTER_SIZE 5
int rssiSamples[RSSI_FILTER_SIZE];
int rssiSampleIndex = 0;
bool filterBufferFilled = false;

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      // Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str()); 
      
      
      if (advertisedDevice.haveName() && advertisedDevice.getName() == TARGET_SERVER_NAME) {
        Serial.print("Found Target Server: ");
        Serial.println(advertisedDevice.getName().c_str());
        Serial.print("Device Address: ");
        Serial.println(advertisedDevice.getAddress().toString().c_str());
        
        if (advertisedDevice.haveRSSI()) {
                int rawRssi = advertisedDevice.getRSSI();

               
                rssiSamples[rssiSampleIndex] = rawRssi;
                rssiSampleIndex = (rssiSampleIndex + 1) % RSSI_FILTER_SIZE;

                if (!filterBufferFilled && rssiSampleIndex == 0) {
                    filterBufferFilled = true; 
                }

                int filteredRssi;
                if (filterBufferFilled) { 
                    long sumRssi = 0;
                    for (int i = 0; i < RSSI_FILTER_SIZE; i++) {
                        sumRssi += rssiSamples[i];
                    }
                    filteredRssi = sumRssi / RSSI_FILTER_SIZE;
                } else {
                    
                    long sumRssi = 0;
                    for(int i = 0; i < rssiSampleIndex; i++) { 
                        sumRssi += rssiSamples[i];
                    }
                    if (rssiSampleIndex > 0) {
                         filteredRssi = sumRssi / rssiSampleIndex;
                    } else {
                         filteredRssi = rawRssi; 
                    }
                }

                Serial.print("Raw RSSI: "); Serial.print(rawRssi);
                Serial.println(" dBm");

               
                double distance = pow(10.0, (txPower - (double)filteredRssi) / (10.0 * pathLossExponentN));

                Serial.print("Estimated Distance: ");
                Serial.print(distance, 2); 
                Serial.println(" m");
            } else {
                Serial.println("No RSSI value available for this device.");
            }
            Serial.println("-----------------------");
        }
    }
};


void setup() {
  Serial.begin(115200);
  Serial.println("Scanning for BLE Server (Distance Project Client)...");

  BLEDevice::init(""); 
  pBLEScan = BLEDevice::getScan(); 
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks()); 
  pBLEScan->setActiveScan(true); 
  pBLEScan->setInterval(100);   
  pBLEScan->setWindow(99);    

  for (int i = 0; i < RSSI_FILTER_SIZE; i++) {
      rssiSamples[i] = -100; 
  }
}

void loop() {
  Serial.println("Starting BLE scan...");
  
  BLEScanResults* pfoundDevices = pBLEScan->start(scanTime, false); 
  
  pBLEScan->clearResults(); 
  
  delay(3000);
}
