#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEAdvertising.h>


#define bleServerName "MyESP32_Distance_Server"


#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"


const int8_t txPowerLevel = -59; 

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE Server (Distance Project)...");

  BLEDevice::init(bleServerName); 

  BLEServer *pServer = BLEDevice::createServer(); 

  
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pService->start();

  
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID); 
  pAdvertising->setScanResponse(true);
  

  BLEDevice::startAdvertising(); 
  Serial.println("BLE Server started advertising.");
  Serial.print("Device Address: ");
  Serial.println(BLEDevice::getAddress().toString().c_str());
  Serial.print("Assumed TX Power for client calculation: ");
  Serial.print(txPowerLevel);
  Serial.println(" dBm");
}

void loop() {
  
  delay(2000); 
}
