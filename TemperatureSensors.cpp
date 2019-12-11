#include <SPI.h>

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

#define CS0 15
#define CS1 16
#define CS2  2

void setup() {
    SPI.begin();
    pinMode(CS0, OUTPUT);
    pinMode(CS1, OUTPUT);
    pinMode(CS2, OUTPUT);
    digitalWrite(CS0, HIGH);
    digitalWrite(CS1, HIGH);
    digitalWrite(CS2, HIGH);

    Serial.begin(115200);
    Serial.println("Hi...");
}

void loop() {
    Serial.print(readCelsius(CS0));
    Serial.print(" ");
    Serial.print(readCelsius(CS1));
    Serial.print(" ");
    Serial.print(readCelsius(CS2));
    Serial.print(" ");
    float t = analogRead(A0);
    t = (t - 360) * 1.454545455;
    Serial.print(t);
    Serial.println();
    delay(1000);
}
