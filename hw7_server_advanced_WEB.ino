#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <math.h>    // pow() 함수 사용을 위해
#include <WiFi.h>    // Wi-Fi 라이브러리
#include <WebServer.h> // 웹 서버 라이브러리

// --- 기본 설정 ---
#define TARGET_SERVER_NAME "MyESP32_Distance_Server" // 위 서버 코드의 이름과 동일하게!
const double txPower = -68.5;             // 네가 측정한 1m 평균 RSSI 값
const double pathLossExponentN = 4.0;       // 네가 설정한 경로 손실 지수 (환경에 맞게 조절 필수!)
int scanTimeBLE = 2;                      // BLE 스캔 시간 (초)
BLEScan* pBLEScan;

// --- Wi-Fi 설정 (네트워크 정보로 변경 필수!) ---
const char* ssid = "AndroidHotspot6948";       // <<<<<<<<<<< 너의 실제 와이파이 SSID 입력!
const char* password = "tksdkr12##"; // <<<<<<<<<<< 너의 실제 와이파이 비밀번호 입력!
WebServer webServer(80);                   // 웹 서버 객체 (포트 80)

// --- 전역 변수 ---
volatile double currentDistance = -1.0;   // 현재 추정 거리 (-1.0은 아직 측정 안됨 또는 오류)
volatile int lastRssiValue = 0;           // 마지막으로 수신된 RSSI 값 (웹 표시용)
volatile bool serverFoundInLastScan = false; // 마지막 스캔에서 서버를 찾았는지 여부

// BLE 스캔 콜백 클래스
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        if (advertisedDevice.haveName() && advertisedDevice.getName() == TARGET_SERVER_NAME) {
            serverFoundInLastScan = true; // 서버 찾음 플래그 설정
            if (advertisedDevice.haveRSSI()) {
                lastRssiValue = advertisedDevice.getRSSI();
                currentDistance = pow(10.0, (txPower - (double)lastRssiValue) / (10.0 * pathLossExponentN));
                
                Serial.print("Found Server. RSSI: "); Serial.print(lastRssiValue);
                Serial.print(" dBm, Calculated Distance: "); Serial.print(currentDistance, 2); Serial.println(" m");
            } else {
                currentDistance = -2.0; // RSSI 값 없음
                lastRssiValue = 0;
                Serial.println("Found Server, but no RSSI value available.");
            }
        }
    }
};

// 웹 서버 루트 경로 핸들러
void handleRoot() {
    String html = "<!DOCTYPE HTML><html><head><title>ESP32 Distance</title>";
    html += "<meta http-equiv='refresh' content='3'>"; // 3초마다 자동 새로고침
    html += "<style>body{font-family:sans-serif; text-align:center; margin-top:40px;}";
    html += "h1{color:#007bff;} .val{font-size:2.5em; font-weight:bold; margin:10px;}";
    html += ".rssi{font-size:1.2em; color:gray;}</style></head><body>";
    html += "<h1>ESP32 BLE Distance Monitor</h1>";

    if (currentDistance == -2.0) {
        html += "<p class='val'>No RSSI from Server</p>";
    } else if (currentDistance >= 0) {
        html += "<p class='val'>" + String(currentDistance, 2) + " m</p>";
        html += "<p class='rssi'>RSSI: " + String(lastRssiValue) + " dBm</p>";
    } else { // currentDistance == -1.0 (초기값 또는 스캔 중)
        html += "<p class='val'>Scanning for server...</p>";
         if(lastRssiValue !=0) { // 이전 RSSI 값이 있다면 참고용으로 표시
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
    pBLEScan->setInterval(131); // 스캔 간격 (ms) - 조금 특이한 값으로 시도
    pBLEScan->setWindow(101); // 스캔 윈도우 (ms)

    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
    WiFi.mode(WIFI_STA); // Station 모드 명시
    WiFi.begin(ssid, password);

    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 20) { // 약 10초간 연결 시도
        delay(500);
        Serial.print(".");
        retries++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi Connected!");
        Serial.print("IP Address: http://");
        Serial.println(WiFi.localIP());

        webServer.on("/", HTTP_GET, handleRoot); // 루트 경로에 대한 핸들러 지정
        webServer.begin();                       // 웹 서버 시작
        Serial.println("Web Server started. Waiting for requests...");
    } else {
        Serial.println("\nFailed to connect to WiFi. Web Server will not be available.");
        // Wi-Fi 연결 실패 시, 웹 서버 없이 BLE 스캔만 계속하도록 할 수 있음
    }
}

void loop() {
    serverFoundInLastScan = false; // 매 스캔 사이클 시작 시 플래그 리셋
    // Serial.println("Starting new BLE scan cycle..."); // 너무 많은 로그 방지

    BLEScanResults* pFoundDevices = pBLEScan->start(scanTimeBLE, false); // 스캔 실행 (결과는 콜백에서 처리)

    if (!serverFoundInLastScan && currentDistance != -2.0) { 
        // 이번 스캔에서 서버를 못 찾았고, 이전에 RSSI가 없었던 게 아니라면 (-2.0)
        // (즉, 서버를 놓쳤다면) currentDistance를 초기값(-1.0)으로 리셋해서 "Scanning..."으로 표시
        // currentDistance = -1.0; // 이 줄을 활성화하면, 서버를 놓쳤을 때 웹페이지에 바로 "Scanning..." 표시
                                // 하지만 너무 자주 바뀌면 보기 불편할 수 있음. 마지막 유효값을 유지하는게 나을수도.
        Serial.println("Target server NOT found in this scan cycle.");
    }
    
    pBLEScan->clearResults(); // 이전 스캔 결과 정리 (메모리 관리)

    if (WiFi.status() == WL_CONNECTED) {
        webServer.handleClient(); // 웹 서버 클라이언트 요청 처리
    }

    // 전체 루프 주기를 고려하여 delay 설정
    // scanTimeBLE (현재 2초) + 이 delay가 대략적인 업데이트 주기
    delay(200); // 1초 대기 (웹 서버 처리 및 다음 스캔 준비)
}