#include "GateServo.h"

GateServo::GateServo(byte pin) {
  this->pin = pin;
  this->state = "closed";
}

void GateServo::begin() {
  gate.attach(pin, 500, 2500);     // Std config, default at 50 Hz, 500 - 2500 min and max pulse
}

void GateServo::open() {
  Serial.println("Abriendo válvula...");

  gate.write(110);      // ClockWise
  delay(700);

  state = "open";
  stop();

  Serial.println("Válvula abierta.");
}

void GateServo::close() {
  Serial.println("Cerrando válvula...");

  gate.write(70);       // CounterClockWise
  delay(700);

  state = "closed";
  stop();

  Serial.println("Válvula cerrada.");
}

void GateServo::stop() {
  gate.write(90);       // Neutral position (stop)
}

String GateServo::getState() {
  return state;
}
