#ifndef __TEMPERATURESENSORS_H__
#define __TEMPERATURESENSORS_H__

void setupTempSensors();

void reportTemperatures();

String reportAllJSON();

void handleTempList();

float getTemp();
#endif
