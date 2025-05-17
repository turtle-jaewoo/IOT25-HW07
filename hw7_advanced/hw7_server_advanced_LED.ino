#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <math.h> 


#define TARGET_SERVER_NAME "MyESP32_Distance_Server" 
const double txPower = -65.5;             
const double pathLossExponentN = 4.0;      
int scanTimeBLE = 1;                    
BLEScan* pBLEScan;

#define LED_PIN 2                         
const double proximityAlertThreshold = 0.5; 
bool ledState = false;                   


volatile double currentDistance = -1.0;  


class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        if (advertisedDevice.haveName() && advertisedDevice.getName() == TARGET_SERVER_NAME) {
            if (advertisedDevice.haveRSSI()) {
                int rssi = advertisedDevice.getRSSI();
                currentDistance = pow(10.0, (txPower - (double)rssi) / (10.0 * pathLossExponentN));

               
                Serial.print("D: "); 
                Serial.print(currentDistance, 2); 
                Serial.println("m");

                
                if (currentDistance > 0 && currentDistance < proximityAlertThreshold) {
                    ledState = !ledState;
                    digitalWrite(LED_PIN, ledState);
                } else {
                    digitalWrite(LED_PIN, LOW);
                    ledState = false;
                }
            } else {
                currentDistance = -1.0;
                digitalWrite(LED_PIN, LOW);
                ledState = false;
                // Serial.println("No RSSI"); 
            }
        }
    }
};

void setup() {
    Serial.begin(115200);
    Serial.println("BLE Client - Minimal LED Alert + Serial Distance");

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    BLEDevice::init("");
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);
}

void loop() {

            pBLEScan->start(scanTimeBLE, false);
    
    pBLEScan->clearResults();
    
    delay(500); 
}
