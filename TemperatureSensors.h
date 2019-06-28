#ifndef __TEMPERATURESENSORS_H__
#define __TEMPERATURESENSORS_H__

void setupTempSensors();

void reportTemperatures();

String reportJSON();

float getTargetTemp();
#endif
