#ifndef __WATERCOOLER_H__
#define __WATERCOOLER_H__

void setupPWM();

void updatePWM();

void setup_WaterCooler_IO();

void setup_WaterCooler();

void loop_WaterCooler();

void broadcast(const char* message);
void broadcast(String &message);

#endif
