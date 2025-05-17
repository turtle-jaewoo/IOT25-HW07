#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

// 찾고자 하는 BLE 서버의 이름 (서버 코드의 bleServerName과 동일하게)
#define TARGET_SERVER_NAME "MyESP32_Distance_Server" 

int scanTime = 2; // 스캔 시간 (초) - 필요에 따라 조절 가능
BLEScan* pBLEScan;

// 이 변수들은 이 코드에서는 사용하지 않지만, 나중에 거리 계산 코드에 txPower를 넣을 때 참고용으로 남겨둘 수 있어.
// const double txPower_placeholder = -59.0; // 여기에 네가 측정한 평균 RSSI값을 넣을 거야!
// const double pathLossExponentN_placeholder = 2.0;

long rssiSum = 0;
int rssiCount = 0;
const int SAMPLES_FOR_AVERAGE = 20; // 평균을 계산할 샘플 수 (선택 사항)

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      // 이름으로 우리가 찾는 서버인지 확인
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
            rssiSum = 0; // 리셋
            rssiCount = 0; // 리셋
          }

        } else {
          Serial.println("Found Target Server, but no RSSI value available.");
        }
        // 이 코드에서는 거리 계산은 하지 않아. RSSI 측정에만 집중!
      }
    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("RSSI Logger for txPower Calibration");
  Serial.println("Place this ESP32 Client 1 meter away from the ESP32 Server.");
  Serial.println("Collect multiple RSSI readings and average them to determine your txPower.");
  Serial.println("----------------------------------------------------------------------");

  BLEDevice::init(""); // 클라이언트 초기화
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
  
  delay(2000); // 다음 스캔까지 2초 대기 (너무 빠르면 RSSI가 불안정할 수 있음, 필요시 조절)
}