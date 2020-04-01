#include <WebSocketsServer.h>

#include <OneWire.h>
#include <DallasTemperature.h>
#include <Arduino.h>

#include <ESP8266WebServer.h>


#include <FastLED.h>

// How many leds in your strip?
#define NUM_LEDS 3

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
#define DATA_PIN 3

// Define the array of leds
CRGB leds[NUM_LEDS];

void setupLeds() {                
  // initialize the digital pin as an output.
  pinMode(3, FUNCTION_3);
  pinMode(3, OUTPUT);
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(50);
  leds[0] = CRGB::Blue;
  FastLED.show();
}

extern ESP8266WebServer server;

WebSocketsServer webSocket(81);

void broadcast(const char *msg) {
  webSocket.broadcastTXT(msg);
}
void broadcast(String &msg) {
  webSocket.broadcastTXT(msg);
}

int errorCount = 0;

int mode = 1;
unsigned long ts;
unsigned long t=0;

char pendingCmd = 0;

void broadcastData() {
    webSocket.broadcastTXT("No data available");
}

void reportTemperatures();

void handleCommand(char ch) {
    if (ch == '?') {
      Serial.print("Sous vide: ");
      Serial.println();
    }
    else if (ch == 'o') {
      // turn off
      webSocket.broadcastTXT("Both off!");
      //mode = 0;
      digitalWrite(16, LOW);
      digitalWrite(5, LOW);
      leds[0] = CRGB::Orange;
      leds[1] = CRGB::Black;
      leds[2] = CRGB::Black;
      FastLED.show();

    }
    else if (ch == 'a') {
      webSocket.broadcastTXT("Turn 5 on!");
      //mode = 1;
      //ts = millis();
      digitalWrite(5, HIGH);
      leds[0] = CRGB::Orange;
      leds[1] = CRGB::Red;
      FastLED.show();
    }
    else if (ch == 'b') {
      webSocket.broadcastTXT("Turn 16 on!");
      //mode = 0;
      digitalWrite(16, HIGH);
      leds[0] = CRGB::Orange;
      leds[2] = CRGB::Red;
      FastLED.show();
    }
}

void loop_Temperature() {
  webSocket.loop();
  if (pendingCmd != 0) {
    handleCommand(pendingCmd);
    pendingCmd = 0;
  }
  if (mode == 1) {
    t = ts + 6000;
    if (t < millis()) {
      broadcastData();
      reportTemperatures();
      //Serial.println(t);
      ts = t;
    }
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
void setupTempSensors();

void setup_Temperature() {
  setupLeds();
  // initialize web sockets
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  // setup temp
  setupTempSensors();
}


// one wire on D1 / GPIO5
#define ONE_WIRE_BUS  4

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

float getAnalogTemp() {
  float t = analogRead(A0);
  t = analogRead(A0);
  t = ((840.0-t)/8.846)+25.0;
  return t;
}
void reportTemperatures() {
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
    output += "{\"name\":\"Temp" + String(i) + "\", \"temp\":" + String(temp) + "}";
  }
  if (output != "[") {
    output += ',';
  }
  temp = getAnalogTemp();
  output += "{\"name\":\"Temp" + String(i++) + "\", \"temp\":" + String(temp) + "}";
  output += "]";
  broadcast(output);
}

void handleTempList() {
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
  temp = getAnalogTemp();
  if (output != "[") {
    output += ',';
  }
  output += "{\"temp" + String(i++) + "\":" + String(temp) + "}";
  
  output += "]";
  server.send(200, "text/json", output);
}
