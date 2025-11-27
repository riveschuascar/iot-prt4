#include "GateServo.h"

// Constructor: Initializes the servo-controlled gate/valve
// Parameters:
//   - pin: Digital PWM pin connected to the servo control signal
GateServo::GateServo(byte pin) {
  this->pin = pin;           // Store the PWM pin number
  this->state = "closed";    // Initial state: valve is closed
}

// begin(): Initializes servo communication and sets pulse width parameters
void GateServo::begin() {
  // attach() parameters:
  // - pin: Digital pin controlling the servo
  // - 500: Minimum pulse width in microseconds (closes the valve)
  // - 2500: Maximum pulse width in microseconds (opens the valve)
  // 
  // Standard servo operates at 50 Hz (20ms period):
  // - PWM duty cycle determines servo angle
  // - 500 μs = approximately 0° (full counterclockwise)
  // - 1500 μs = 90° (neutral/stop position)
  // - 2500 μs = 180° (full clockwise)
  gate.attach(pin, 500, 2500);
}

// open(): Opens the valve by rotating servo clockwise
void GateServo::open() {
  Serial.println("Opening valve...");

  // write(110) sends a PWM signal that rotates servo clockwise
  // Value 110 is mapped to a pulse width between 500-2500 microseconds
  // Calculation: pulse = 500 + (110/180) × (2500-500) = ~1722 μs
  // This angle opens the gas flow through the valve
  gate.write(110);           // Clockwise rotation to OPEN position
  
  delay(700);                // Wait 700ms for servo to complete rotation and stabilize
                             // (sufficient time for typical servo response)

  state = "open";            // Update internal state flag
  stop();                    // Move servo to neutral position after reaching target

  Serial.println("Valve opened.");
}

// close(): Closes the valve by rotating servo counterclockwise
void GateServo::close() {
  Serial.println("Closing valve...");

  // write(70) sends a PWM signal that rotates servo counterclockwise
  // Calculation: pulse = 500 + (70/180) × (2500-500) = ~778 μs
  // This angle closes the gas flow through the valve
  gate.write(70);            // Counterclockwise rotation to CLOSE position
  
  delay(700);                // Wait 700ms for servo to complete rotation and stabilize

  state = "closed";          // Update internal state flag
  stop();                    // Move servo to neutral position after reaching target

  Serial.println("Valve closed.");
}

// stop(): Moves servo to neutral position (90°) to stop rotation and reduce power consumption
// Neutral position = write(90)
// - Calculation: pulse = 500 + (90/180) × (2500-500) = 1500 μs
// - At 1500 μs, servo motor stops and holds neutral position
// - This also prevents servo from holding constant torque, saving battery/power
void GateServo::stop() {
  gate.write(90);            // Neutral position - servo motor stops
}

// getState(): Returns the current valve state as a string
// Returns: "open" or "closed" depending on last operation
String GateServo::getState() {
  return state;
}