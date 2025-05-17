#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <math.h> // pow() 함수 사용을 위해

// 찾고자 하는 BLE 서버의 이름
#define TARGET_SERVER_NAME "MyESP32_Distance_Server"

// 거리 계산을 위한 파라미터 (서버의 txPower와 환경에 따른 n값)
const double txPower = -80.0;      // 서버에서 가정한 Tx Power (dBm). 실제 값으로 보정 필요!
const double pathLossExponentN = 4.0; // 경로 손실 지수 (n). 환경에 따라 2.0 ~ 4.0. 보정 필요!

int scanTime = 2; // 스캔 시간 (초)
BLEScan* pBLEScan;

#define RSSI_FILTER_SIZE 5 // 이동 평균 필터 크기 (조절 가능)
int rssiSamples[RSSI_FILTER_SIZE];
int rssiSampleIndex = 0;
bool filterBufferFilled = false; // 필터 버퍼가 다 찼는지 확인용

// BLE 스캔 콜백 클래스
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      // Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str()); // 모든 장치 보기 (디버깅용)
      
      // 이름으로 우리가 찾는 서버인지 확인
      if (advertisedDevice.haveName() && advertisedDevice.getName() == TARGET_SERVER_NAME) {
        Serial.print("Found Target Server: ");
        Serial.println(advertisedDevice.getName().c_str());
        Serial.print("Device Address: ");
        Serial.println(advertisedDevice.getAddress().toString().c_str());
        
        if (advertisedDevice.haveRSSI()) {
                int rawRssi = advertisedDevice.getRSSI();

                // 이동 평균 필터 적용
                rssiSamples[rssiSampleIndex] = rawRssi;
                rssiSampleIndex = (rssiSampleIndex + 1) % RSSI_FILTER_SIZE;

                if (!filterBufferFilled && rssiSampleIndex == 0) {
                    filterBufferFilled = true; // 버퍼가 한번 다 찼음을 표시
                }

                int filteredRssi;
                if (filterBufferFilled) { // 버퍼가 다 찼을 때만 평균 계산
                    long sumRssi = 0;
                    for (int i = 0; i < RSSI_FILTER_SIZE; i++) {
                        sumRssi += rssiSamples[i];
                    }
                    filteredRssi = sumRssi / RSSI_FILTER_SIZE;
                } else {
                    // 버퍼가 다 차기 전에는 그냥 현재 값 또는 이전 값 사용 (혹은 더 나은 초기화)
                    // 간단하게는 그냥 rawRssi를 사용하거나, 첫 값으로 배열을 채울 수도 있음
                    long sumRssi = 0;
                    for(int i = 0; i < rssiSampleIndex; i++) { // 현재까지 채워진 만큼만 평균
                        sumRssi += rssiSamples[i];
                    }
                    if (rssiSampleIndex > 0) {
                         filteredRssi = sumRssi / rssiSampleIndex;
                    } else {
                         filteredRssi = rawRssi; // 첫 샘플인 경우
                    }
                }

                Serial.print("Raw RSSI: "); Serial.print(rawRssi);
                Serial.println(" dBm");

                // 필터링된 RSSI로 거리 계산
                double distance = pow(10.0, (txPower - (double)filteredRssi) / (10.0 * pathLossExponentN));

                Serial.print("Estimated Distance: ");
                Serial.print(distance, 2); // 소수점 둘째 자리까지 표시
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

  BLEDevice::init(""); // 클라이언트 초기화 (이름은 없어도 됨)
  pBLEScan = BLEDevice::getScan(); // 스캔 객체 생성
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks()); // 콜백 설정
  pBLEScan->setActiveScan(true); // Active 스캔 (더 많은 정보, 약간 더 많은 전력 소모)
  pBLEScan->setInterval(100);    // 스캔 간격 (ms)
  pBLEScan->setWindow(99);     // 스캔 윈도우 (ms) - setInterval보다 작거나 같아야 함

  for (int i = 0; i < RSSI_FILTER_SIZE; i++) {
      rssiSamples[i] = -100; // 또는 적절한 초기값
  }
}

void loop() {
  Serial.println("Starting BLE scan...");
  // 지정된 시간(scanTime) 동안 스캔 실행, false는 스캔 후 결과 유지 (메모리 주의)
  BLEScanResults* pfoundDevices = pBLEScan->start(scanTime, false); 
  
  pBLEScan->clearResults(); // 메모리 확보를 위해 이전 스캔 결과 삭제
  
  delay(3000); // 다음 스캔까지 5초 대기 (주기 조절 가능)
}