#include <SPI.h>
#include <ESP8266WebServer.h>
#include "P1Logger.h"

extern ESP8266WebServer server;

#define CS0 15
#define CS1 16
#define CS2  2

double readCelsius(uint8_t cs) {
    uint16_t v;

    digitalWrite(cs, LOW);
    v = SPI.transfer(0x00);
    v <<= 8;
    v |= SPI.transfer(0x00);
    digitalWrite(cs, HIGH);

    if (v & 0x4) {
        // uh oh, no thermocouple attached!
        Serial.println("No thermo attached");
        return NAN; 
    }
    else if (v == 0x0) {
        Serial.println("No Max6675 detected");
        return NAN;
    }

    v >>= 3;

    return v*0.25;
}

void setupTempSensors() {
    SPI.begin();
    pinMode(CS0, OUTPUT);
    pinMode(CS1, OUTPUT);
    pinMode(CS2, OUTPUT);
    digitalWrite(CS0, HIGH);
    digitalWrite(CS1, HIGH);
    digitalWrite(CS2, HIGH);
}

void reportTemperatures() {
  String output = "[";
  output += "{\"name\":\"Lower\", \"temp\":" + String(readCelsius(CS0)) + "}";
  output += ',';
  output += "{\"name\":\"Middle\", \"temp\":" + String(readCelsius(CS1)) + "}";
  output += ',';
  output += "{\"name\":\"Upper\", \"temp\":" + String(readCelsius(CS2)) + "}";
  output += ',';
  float t = analogRead(A0);
  t = (t - 360) * 1.454545455;
  output += "{\"name\":\"Probe\", \"temp\":" + String(t) + "}";
  output += "]";
  broadcast(output);
}

void handleTempList() {
  Serial.println("Report All JSON temperature sensors:");
  String output = "[";
  output += "{\"Lower\":" + String(readCelsius(CS0)) + "}";
  output += ',';
  output += "{\"Middle\":" + String(readCelsius(CS1)) + "}";
  output += ',';
  output += "{\"Upper\":" + String(readCelsius(CS2)) + "}";
  output += ',';
  float t = analogRead(A0);
  t = (t - 360) * 1.454545455;
  output += "{\"Probe\":" + String(t) + "}";
  output += "]";
  server.send(200, "text/json", output);
}
