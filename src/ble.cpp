#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include "ble.h"

BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

// sample RMS values
float extensorRms = 0.001;
float flexorRms = 0.002;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define SERVICE_UUID "63b803e2-9201-47ee-968b-1405602a1b8e"
#define CHARACTERISTIC_UUID "46bfca8b-b8d8-40b1-87e7-c22116324c01"

char buf[100];

class MyServerCallbacks : public BLEServerCallbacks
{
private:
    void onConnect(BLEServer *pServer)
    {
        deviceConnected = true;
        BLEDevice::startAdvertising();
    };

    void onDisconnect(BLEServer *pServer)
    {
        deviceConnected = false;
    }
};

class MyCallbacks : public BLECharacteristicCallbacks
{
private:
    void onRead(BLECharacteristic *pCharacteristic)
    {
        Serial.println("onRead");
        sprintf(buf, "E%f,F%f", extensorRms, flexorRms);
        pCharacteristic->setValue(buf);
    }

    void onWrite(BLECharacteristic *pCharacteristic)
    {
        Serial.println("onWrite");
        std::string value = pCharacteristic->getValue();
        pCharacteristic->setValue(value);
        pCharacteristic->notify();
    }

    void onNotify(BLECharacteristic *pCharacteristic)
    {
        Serial.println("onNotify");
    }
};

void SetUpBLE()
{
    // Create the BLE Device
    BLEDevice::init("SAMPLE ESP32 DEVICE");

    // Create the BLE Server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // Create the BLE Service
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // Create a BLE Characteristic
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_NOTIFY |
            BLECharacteristic::PROPERTY_INDICATE);

    pCharacteristic->setCallbacks(new MyCallbacks());

    // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
    // Create a BLE Descriptor
    pCharacteristic->addDescriptor(new BLE2902());

    // Start the service
    pService->start();

    // Start advertising
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x0); // set value to 0x00 to not advertise this parameter
    BLEDevice::startAdvertising();
    Serial.println("Waiting a client connection to notify...");
}

void UpdateBLEConnection()
{
    // disconnecting
    if (!deviceConnected && oldDeviceConnected)
    {
        delay(500);                  // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected)
    {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
}