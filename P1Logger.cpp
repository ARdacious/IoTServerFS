#include <WebSocketsServer.h>
#include "TemperatureSensors.h"
#include <Arduino.h>

#include <Ticker.h>  //Ticker Library
 
Ticker blinker;
Ticker tempUpdater;
 
#define LED                2  //On board LED
 
//=======================================================================
void blinkLed()
{
  digitalWrite(LED, !(digitalRead(LED)));  //Invert Current State of LED  
}

#include "dsmr.h"

#define DBG_OUTPUT_PORT Serial
#define DSMR_ENABLE   D6          // DSMR enable pin

P1Reader reader(&DBG_OUTPUT_PORT, DSMR_ENABLE);

unsigned long last;

/**
 * Define the data we're interested in, as well as the datastructure to
 * hold the parsed data. This list shows all supported fields, remove
 * any fields you are not using from the below list to make the parsing
 * and printing code smaller.
 * Each template argument below results in a field of the same name.
 */
using MyData = ParsedData<
//  /* String */ identification,
//  /* String */ p1_version,
//  /* String */ timestamp,
//  /* String */ equipment_id,
  /* FixedValue */ energy_delivered_tariff1,
  /* FixedValue */ energy_delivered_tariff2,
  /* FixedValue */ energy_returned_tariff1,
  /* FixedValue */ energy_returned_tariff2,
//  /* String */ electricity_tariff,
  /* FixedValue */ power_delivered,
  /* FixedValue */ power_returned,
  /* FixedValue */ electricity_threshold,
  /* uint8_t */ electricity_switch_position,
  /* uint32_t */ electricity_failures,
  /* uint32_t */ electricity_long_failures,
//  /* String */ electricity_failure_log,
  /* uint32_t */ electricity_sags_l1,
  /* uint32_t */ electricity_sags_l2,
  /* uint32_t */ electricity_sags_l3,
  /* uint32_t */ electricity_swells_l1,
  /* uint32_t */ electricity_swells_l2,
  /* uint32_t */ electricity_swells_l3,
//  /* String */ message_short,
//  /* String */ message_long,
  /* FixedValue */ voltage_l1,
  /* FixedValue */ voltage_l2,
  /* FixedValue */ voltage_l3,
  /* FixedValue */ current_l1,
  /* FixedValue */ current_l2,
  /* FixedValue */ current_l3,
  /* FixedValue */ power_delivered_l1,
  /* FixedValue */ power_delivered_l2,
  /* FixedValue */ power_delivered_l3,
  /* FixedValue */ power_returned_l1,
  /* FixedValue */ power_returned_l2,
  /* FixedValue */ power_returned_l3,
  /* uint16_t */ gas_device_type,
//  /* String */ gas_equipment_id,
  /* uint8_t */ gas_valve_position,
//  /* TimestampedFixedValue */ gas_delivered,
  /* uint16_t */ thermal_device_type,
//  /* String */ thermal_equipment_id,
  /* uint8_t */ thermal_valve_position,
//  /* TimestampedFixedValue */ thermal_delivered,
  /* uint16_t */ water_device_type,
//  /* String */ water_equipment_id,
  /* uint8_t */ water_valve_position,
//  /* TimestampedFixedValue */ water_delivered,
  /* uint16_t */ slave_device_type,
//  /* String */ slave_equipment_id,
  /* uint8_t */ slave_valve_position
//  /* TimestampedFixedValue */ slave_delivered
>;

/**
 * This illustrates looping over all parsed fields using the
 * ParsedData::applyEach method.
 *
 * When passed an instance of this Printer object, applyEach will loop
 * over each field and call Printer::apply, passing a reference to each
 * field in turn. This passes the actual field object, not the field
 * value, so each call to Printer::apply will have a differently typed
 * parameter.
 *
 * For this reason, Printer::apply is a template, resulting in one
 * distinct apply method for each field used. This allows looking up
 * things like Item::name, which is different for every field type,
 * without having to resort to virtual method calls (which result in
 * extra storage usage). The tradeoff is here that there is more code
 * generated (but due to compiler inlining, it's pretty much the same as
 * if you just manually printed all field names and values (with no
 * cost at all if you don't use the Printer).
 */
struct Printer {
  template<typename Item>
  void apply(Item &i) {
    if (i.present()) {
      Serial.print(Item::name);
      Serial.print(F(": "));
      Serial.print(i.val());
      Serial.print(Item::unit());
      Serial.println();
    }
  }
};

WebSocketsServer webSocket(81);

void broadcast(const char *msg) {
  webSocket.broadcastTXT(msg);
}
void broadcast(String &msg) {
  webSocket.broadcastTXT(msg);
}

String report;

struct JsonifyReport {
  template<typename Item>
  void apply(Item &i) {
    if (i.present()) {
      Serial.print(Item::name);
      //broadcast(String(Item::name).c_str());
      //broadcast(String(i.val()).c_str());
      report += "\"";
      report += Item::name;
      report += "\":" + String(i.val()) + ", ";
      //Serial.print(Item::unit());
      //Serial.println();
    }
  }
};


int errorCount = 0;

int mode = 1;
unsigned long ts;
unsigned long t=0;

char pendingCmd = 0;



void handleCommand(char ch) {
    if (ch == '?') {
      Serial.print("Speed: ");
      Serial.println();
    }
    else if (ch == 'o') {
      // turn off
      webSocket.broadcastTXT("Mode off!");
      mode = 0;
    }
    else if (ch == 'a') {
      webSocket.broadcastTXT("Mode automatic!");
      mode = 1;
      ts = millis();
    }
    else if (ch == 'f') {
      webSocket.broadcastTXT("Mode full!");
      mode = 0;
    }
    else if (ch == 'h') {
      webSocket.broadcastTXT("Mode half!");
      mode = 0;
    }
    else if (ch == 'p') {
      webSocket.broadcastTXT("Pump!");
      mode = 0;
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

void broadcastData() {
  MyData data;
  String err;
  reader.enable(true);
  delay(10);

  if (reader.available()) {
    if (reader.parse(&data, &err)) {
      // Parse succesful, print result
      data.applyEach(Printer());
      String json;  
      report = "{";
      JsonifyReport r = JsonifyReport();
      data.applyEach(r);
      report.remove(report.length()-2,1);
      report += "}";
      webSocket.broadcastTXT(report);
    }
  }
  else {
    webSocket.broadcastTXT("No data available");
  }
}

void updateProgress(size_t i, size_t j) {
  blinkLed();
}

void setup_P1Logger_IO() {
  // switch off DMSR
  pinMode(DSMR_ENABLE , OUTPUT);
  digitalWrite(DSMR_ENABLE, LOW);
  //digitalWrite(DSMR_ENABLE, HIGH);
  // start a read right away
  reader.enable(true);}

void setup_P1Logger() {
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
  //tempUpdater.attach_ms_scheduled(2000, broadcastTemp);
}

void loop_P1Logger() {
  webSocket.loop();
  reader.loop();
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
