
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Arduino.h>

#include "P1Logger.h"

// one wire on D1 / GPIO5
#define ONE_WIRE_BUS  5

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

//DeviceAddress ambientAddr = {40, 97, 100, 18, 46, 126, 6, 12};

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
    json += "\"temp" + String(i) + "\":" + String(temp) + ",";
  }
  json.remove(json.length()-1,1);
  json += "}";  
  broadcast(json);
}


float getTemp() {
  return sensors.getTempCByIndex(0);
}

#include <ESP8266WebServer.h>

extern ESP8266WebServer server;

void handleTempList() {
  Serial.println("Report All JSON temperature sensors:");
  String output = "[";
  int c = sensors.getDeviceCount();
  int i;
  float temp;
  sensors.requestTemperatures();
  for (i = 0; i < c; i++) {
    temp = sensors.getTempCByIndex(i);
    if (output != "[") {
      output += ',';
    }
    output += "{\"temp" + String(i) + "\":" + String(temp) + "}";
  }

  output += "]";
  server.send(200, "text/json", output);
}
