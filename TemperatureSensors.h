#ifndef __TEMPERATURESENSORS_H__
#define __TEMPERATURESENSORS_H__

void setup_Temperature();
void loop_Temperature();

void setupTempSensors();

void reportTemperatures();

String reportAllJSON();

void handleTempList();

float getTemp();
#endif
