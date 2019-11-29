#ifndef __P1LOGGER_H__
#define __P1LOGGER_H__

void setupPWM();

void updatePWM();

void setup_P1Logger_IO();

void setup_P1Logger();

void loop_P1Logger();

void broadcast(const char* message);
void broadcast(String &message);

#endif
