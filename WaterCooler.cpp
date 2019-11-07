#include <WebSocketsServer.h>
#include "TemperatureSensors.h"
#include <Arduino.h>

#include <Ticker.h>  //Ticker Library
 
Ticker blinker;
Ticker tempUpdater;
 
#define LED                2  //On board LED
#define PUMP_PWM          12
#define FAN_PWM            3
#define PELTIER_FAN_PWM   13
 
//=======================================================================
void blinkLed()
{
  digitalWrite(LED, !(digitalRead(LED)));  //Invert Current State of LED  
}

WebSocketsServer webSocket(81);

int fanSpeed = 255;
int pumpSpeed = 255;
int errorCount = 0;

int mode = 1;
unsigned long ts;
unsigned long t=0;

char pendingCmd = 0;


void broadcast(const char *msg) {
  webSocket.broadcastTXT(msg);
}
void broadcast(String &msg) {
  webSocket.broadcastTXT(msg);
}

void init_cycle(void)
{
  webSocket.broadcastTXT("Performing init cycle...");
  /* set full speed */
  analogWrite(FAN_PWM, 1023);
  delay(1500);
  analogWrite(FAN_PWM, 512);
  delay(1500);
  /* turn off */
  analogWrite(FAN_PWM, 0);
  /* set full speed */
  analogWrite(PELTIER_FAN_PWM, 1023);
  delay(1500);
  analogWrite(PELTIER_FAN_PWM, 500);
  delay(1500);
  /* turn off */
  analogWrite(PELTIER_FAN_PWM, 0);
  /* enable pump */
  analogWrite(PUMP_PWM, 1023);
  delay(3000);
  analogWrite(PUMP_PWM, 0);
  webSocket.broadcastTXT("Init cycle done!");
}

void set_speeds(int fan, int pump) {
  if (fan != fanSpeed) {
    analogWrite(FAN_PWM, fan);
    Serial.print("Changed fan speed: ");
    Serial.print(fanSpeed);
    Serial.print(" -> ");
    Serial.println(fan);
    fanSpeed = fan;
  }
  if (pump > 0) {
    pump = max(pump, 400);
  }
  else {
    pump = 0;
    
  }
  if (pump != pumpSpeed) {
    analogWrite(PUMP_PWM, pump);
    Serial.print("Changed pump speed: ");
    Serial.print(pumpSpeed);
    Serial.print(" -> ");
    Serial.println(pump);
    pumpSpeed = pump;
  }
}

void temp_control()
{
  int speed = 0;
  float target = getTargetTemp();
  if (target < 0) {
    // it is either freezing or the error temp of -127.0 is read
    // we dont know, so max cooling
    speed = 1023;
    errorCount++;
    Serial.println("Error reading temperature!!!");
    Serial.print(errorCount);
    Serial.println(" errors");
    setupTempSensors();
  }
  else if (target < 25 && speed > 0) {
    // no need to start cooling, temperature under
    // typical max ambient
    speed = 0;
  }
  else if (target > 26) {
    speed = (int)1023.0/(40.0-28.0)*(target-28.0);
  }
  if (speed < 0) speed = 0;
  if (speed > 1023) speed = 1023;
  set_speeds(speed, speed);
}

void handleCommand(char ch) {
    if (ch == '?') {
      Serial.print("Speed: ");
      Serial.print(fanSpeed);
      /*
      Serial.print(", Fan: ");
      Serial.print(fanRPM);
      Serial.print(" rpm, Pump: ");
      Serial.print(pumpRPM);
      Serial.println(" rpm");
      */
      reportTemperatures();
      Serial.println();
    }
    else if (ch == 'o') {
      // turn off
      webSocket.broadcastTXT("Mode off!");
      mode = 0;
      set_speeds(0,0);
    }
    else if (ch == 'a') {
      webSocket.broadcastTXT("Mode automatic!");
      mode = 1;
      ts = millis();
    }
    else if (ch == 'f') {
      webSocket.broadcastTXT("Mode full!");
      mode = 0;
      set_speeds(1023, 1023);
    }
    else if (ch == 'h') {
      webSocket.broadcastTXT("Mode half!");
      mode = 0;
      set_speeds(512, 512);
    }
    else if (ch == 'p') {
      webSocket.broadcastTXT("Pump!");
      mode = 0;
      set_speeds(0,900);
    }
    else if (ch == 'i') {
      /* cycle for poweer up recognition */
      init_cycle();
    }
    else if (ch == 't') {
      setupTempSensors();
      reportTemperatures();
    }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) 
{ 
  // When a WebSocket message is received
  switch (type) {
    case WStype_DISCONNECTED:             // if the websocket is disconnected
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {              // if a new websocket connection is established
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        webSocket.broadcastTXT("Client connected...");
      }
      break;
    case WStype_TEXT:                     // if new text data is received
      Serial.printf("[%u] get Text: %s\n", num, payload);
      // broadcast received command
      webSocket.broadcastTXT(payload);
      // handle commands
      pendingCmd = payload[0];
      break;
  }
}

void broadcastTemp() {
  String json;  
  json = reportJSON();
  webSocket.broadcastTXT(json);
  float temp = 0;
  json = "{";
  temp = 1.0 * pumpSpeed / 10.0;
  json += "\"pump-speed\":" + String(temp) + ", ";
  temp = 1.0 * fanSpeed / 10.0;
  json += "\"fan-speed\":" + String(temp);
  json += "}";
  webSocket.broadcastTXT(json);
}

void updateProgress(size_t i, size_t j) {
  blinkLed();
}

void setup_WaterCooler_IO() {
  // pwm freq
  analogWriteFreq(50);
  analogWrite(FAN_PWM, 0);
  analogWrite(PELTIER_FAN_PWM, 0);
  analogWrite(PUMP_PWM, 0);
}

void setup_WaterCooler() {
  // initialize web sockets
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  // setup temp
  setupTempSensors();
  // Initialize Ticker every 0.5s to blink blue led
  pinMode(LED,OUTPUT);
  //Update.onProgress(updateProgress);
  blinker.attach_ms_scheduled(500, blinkLed);
  // broadcast status
  tempUpdater.attach_ms_scheduled(2000, broadcastTemp);
  init_cycle();
}

void loop_WaterCooler() {
  webSocket.loop();
  if (pendingCmd != 0) {
    handleCommand(pendingCmd);
    pendingCmd = 0;
  }
  if (mode == 1) {
    t = ts + 1000;
    if (t < millis()) {
      temp_control();
      //Serial.println(t);
      ts = t;
    }
  }
}


