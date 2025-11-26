#pragma once

#include <Arduino.h>
#include <ESP32Servo.h>

class GateServo {
private:
  byte pin;
  String state;
  Servo gate;

public:
  GateServo(byte pin);
  void begin();
  void open();
  void close();
  void stop();
  String getState();
};
