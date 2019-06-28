
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Arduino.h>

#include "WaterCooler.h"

// one wire on tx bus
#define ONE_WIRE_BUS  14

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

/*
Sensor 0: 40, 97, 100, 18, 46, 110, 37, 33,
Sensor 1: 40, 97, 100, 18, 46, 126, 6, 12,
Sensor 2: 40, 97, 100, 18, 46, 109, 233, 29,
Sensor 3: 40, 97, 100, 18, 41, 246, 95, 104,
*/
//DeviceAddress ambientAddr = {40, 97, 100, 18, 46, 126, 6, 12};
DeviceAddress ambientAddr = {40, 255, 130, 13, 96, 23, 3, 88}; // new on a wire
DeviceAddress supplyFlowAddr = {40, 97, 100, 18, 46, 110, 37, 33};
DeviceAddress returnFlowAddr = {40, 97, 100, 18, 41, 246, 95, 104};
DeviceAddress targetAddr = {40, 97, 100, 18, 46, 109, 233, 29};

void setupTempSensors()
{
  broadcast("Detect temperature sensors:");
  sensors.begin();
  int c = sensors.getDeviceCount();
  int i,j;
  DeviceAddress temp;
  for (i = 0; i < c; i++) {
    String msg = "";
    msg += "Sensor ";
    msg += String(i);
    msg += ": ";
    if (sensors.getAddress(temp, i)) {
      for (j=0; j<8; j++) {
        msg += String(temp[j]);
        msg += ", ";
      }
    }
    broadcast(msg);
  }
}

void reportTemperatures()
{
  broadcast("Report temperature sensors:");
  int c = sensors.getDeviceCount();
  int i;
  float temp;
  sensors.requestTemperatures();
  for (i = 0; i < c; i++) {
    String msg = "";
    msg += "Sensor ";
    msg += String(i);
    msg += ": ";
    temp = sensors.getTempCByIndex(i);
    msg += String(temp);
    msg += "C";
    broadcast(msg);
  }
}

void reportAllJSON() {
  Serial.println("Report All JSON temperature sensors:");
  int c = sensors.getDeviceCount();
  int i;
  float temp;
  String json = "{";
  json += "\"count\":" + String(c);
  sensors.requestTemperatures();
  for (i = 0; i < c; i++) {
    temp = sensors.getTempCByIndex(i);
    json += "\"temp" + String(i) + "\":" + String(temp);
  }
  json += "}";  
}

String reportJSON() {
  Serial.println("Report JSON temperature sensors:");
  float temp;
  unsigned long s = millis();
  String json = "{";
  json += "\"start_ms\":" + String(s) + ", ";
  sensors.requestTemperatures();
  temp = sensors.getTempC(ambientAddr);
  json += "\"room-temp\":" + String(temp) + ", ";
  temp = sensors.getTempC(targetAddr);
  json += "\"target-temp\":" + String(temp) + ", ";
  temp = sensors.getTempC(supplyFlowAddr);
  json += "\"supply-temp\":" + String(temp) + ", ";
  temp = sensors.getTempC(returnFlowAddr);
  json += "\"return-temp\":" + String(temp) + ", ";
  json += "\"took_ms\":" + String(-s + millis());
  json += "}";
  return json;
}

float getTargetTemp() {
  return sensors.getTempC(targetAddr);
}

