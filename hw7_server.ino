#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEAdvertising.h>

// BLE 서버 이름 정의
#define bleServerName "MyESP32_Distance_Server"

// 서비스 UUID (고유해야 함, 필요시 https://www.uuidgenerator.net/ 에서 생성)
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
// 캐릭터리스틱 UUID (이 예제에서는 단순 광고만 사용하므로 캐릭터리스틱은 사용 안 함)
// #define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// txPower 값: 클라이언트가 거리 계산 시 사용할 서버의 송신 전력 (dBm).
// 이 값은 실제 환경에서 1m 거리에서 측정한 RSSI 값으로 보정하는 것이 좋음.
// 여기서는 예시 값으로 -59dBm 사용.
const int8_t txPowerLevel = -59; 

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE Server (Distance Project)...");

  BLEDevice::init(bleServerName); // BLE 장치 초기화 및 이름 설정

  BLEServer *pServer = BLEDevice::createServer(); // BLE 서버 생성

  // BLE 서비스 생성 (필요하다면 캐릭터리스틱도 여기에 추가)
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pService->start(); // 서비스 시작

  // BLE 광고 설정
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID); // 광고에 서비스 UUID 포함
  pAdvertising->setScanResponse(true);
  // Advertising TX power level (optional, for some clients to pick up)
  // pAdvertising->setMinPreferred(0x06); 
  // pAdvertising->setMinPreferred(0x12);

  // ESP32의 실제 송신 전력을 설정하려면 다음 함수들을 사용할 수 있지만 (ESP-IDF 레벨),
  // Arduino에서는 보통 Advertising 데이터에 직접 넣거나, 클라이언트에서 가정된 값을 사용.
  // esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P7); // 예: 광고 전력 설정
  // esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P7); // 예: 기본 전력 설정

  BLEDevice::startAdvertising(); // 광고 시작
  Serial.println("BLE Server started advertising.");
  Serial.print("Device Address: ");
  Serial.println(BLEDevice::getAddress().toString().c_str());
  Serial.print("Assumed TX Power for client calculation: ");
  Serial.print(txPowerLevel);
  Serial.println(" dBm");
}

void loop() {
  // 단순 광고 서버는 loop에서 특별히 할 일이 없음
  delay(2000); // 2초마다 CPU에게 잠시 휴식
}