#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <math.h> // pow() 함수 사용을 위해

// --- 기본 설정 ---
#define TARGET_SERVER_NAME "MyESP32_Distance_Server" // 너의 BLE 서버 이름과 동일하게!
const double txPower = -65.5;             // 네가 측정한 1m 평균 RSSI 값
const double pathLossExponentN = 4.0;       // 네가 설정한 경로 손실 지수 (환경에 맞게 조절 필수!)
int scanTimeBLE = 1;                      // BLE 스캔 시간 (초)
BLEScan* pBLEScan;

// --- LED 설정 ---
#define LED_PIN 2                         // ESP32 내장 LED는 보통 GPIO2
const double proximityAlertThreshold = 0.5; // 50cm (0.5m) 근접 알림 기준
bool ledState = false;                    // 현재 LED 깜빡임 상태

// --- 전역 변수 ---
volatile double currentDistance = -1.0;   // 현재 추정 거리

// BLE 스캔 콜백 클래스
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        if (advertisedDevice.haveName() && advertisedDevice.getName() == TARGET_SERVER_NAME) {
            if (advertisedDevice.haveRSSI()) {
                int rssi = advertisedDevice.getRSSI();
                currentDistance = pow(10.0, (txPower - (double)rssi) / (10.0 * pathLossExponentN));

                // 시리얼 모니터에 거리 값 출력 (최소화된 정보)
                Serial.print("D: "); // "Distance: " 약자
                Serial.print(currentDistance, 2); // 소수점 둘째 자리까지
                Serial.println("m");

                // LED 제어 로직
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
                // Serial.println("No RSSI"); // RSSI 없을 때 메시지 (선택 사항)
            }
        }
    }
};

void setup() {
    Serial.begin(115200); // 시리얼 통신 다시 활성화!
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
    // Serial.println("Scanning..."); // 매번 스캔 시작 메시지는 너무 많을 수 있으니 주석 처리
    // BLEScanResults* pFoundDevices = // 이 변수는 콜백 방식에서는 직접 사용 안 함
            pBLEScan->start(scanTimeBLE, false);
    
    pBLEScan->clearResults();
    
    delay(500); // 0.5초 후 다음 스캔 (너무 짧으면 시리얼 출력이 너무 빠를 수 있음)
}