#include "GasSensor.h"

// Constructor: Initializes the gas sensor with calibration parameters and default gas curve
GasSensor::GasSensor(byte analogPin, float rlValue, float roCleanAirFactor, float* curve)
  : analogPin(analogPin),                    // ADC pin connected to sensor
    rlValue(rlValue),                        // Load resistance (RL) in kΩ - used for Rs calculation
    roCleanAirFactor(roCleanAirFactor),      // Ro calibration factor from datasheet (typical ~10)
    state(IDLE),                             // Initial state
    lastTime(0),                             // Timestamp for non-blocking timing
    readIndex(0),                            // Counter for reading samples
    calibIndex(0),                           // Counter for calibration samples
    rsSum(0)                                 // Accumulator for resistance sum
{
    // Use default gas curve if none provided
    // These coefficients are used in logarithmic PPM calculation
    if (curve == nullptr) {
        gasCurve[0] = 2.3;     // Vertical offset in log equation
        gasCurve[1] = 0.21;    // X-intercept reference point
        gasCurve[2] = -0.47;   // Slope (negative = inverse relationship)
    } else {
        gasCurve[0] = curve[0];
        gasCurve[1] = curve[1];
        gasCurve[2] = curve[2];
    }

    ro = 10.0;                // Initial Ro estimate (resistance in clean air, in kΩ)
    ppm = 0;                  // Initialized PPM value
    gasLevelState = "Normal"; // Initial gas level state
}

// ===================================
// BLOCKING CALIBRATION FUNCTION
// Purpose: Quickly calibrate sensor in clean air (takes ~10 seconds)
// ===================================
void GasSensor::calibrateBlocking() {
    Serial.println("Calibrating MQ-2 sensor in clean air (blocking mode)...");
    
    float rsTotal = 0;
    int samples = 50;  // Take 50 samples for stable average
    
    for (int i = 0; i < samples; i++) {
        // Read analog value from sensor (0-4095 on most microcontrollers)
        int adcValue = analogRead(analogPin);
        
        Serial.print("Sample ");
        Serial.print(i + 1);
        Serial.print("/");
        Serial.print(samples);
        Serial.print(" - ADC: ");
        Serial.print(adcValue);
        
        // Convert ADC value to sensor resistance (Rs)
        float rs = calculateResistance(adcValue);
        
        Serial.print(" - Rs: ");
        Serial.print(rs);
        Serial.println(" kΩ");
        
        // Only accumulate valid resistance values (positive)
        if (rs > 0) {
            rsTotal += rs;
        }
        
        delay(200); // Wait 200ms between samples (50 samples × 200ms = ~10 seconds)
    }
    
    // Calculate Ro: average Rs divided by the clean air factor from datasheet
    // Ro = Rs(in clean air) / 10 (typical factor, varies by sensor)
    // This calibration assumes sensor is in clean air (21% O2)
    ro = (rsTotal / samples) / roCleanAirFactor;
    
    Serial.println("─────────────────────────────");
    Serial.print("Calibration complete. Ro = ");
    Serial.print(ro);
    Serial.println(" kΩ");
    Serial.println("─────────────────────────────");
    
    state = IDLE;
}

// ===================================
// NON-BLOCKING CALIBRATION (Original version)
// Purpose: Calibrate without blocking - must call updateCalibration() periodically
// ===================================
void GasSensor::startCalibration() {
    Serial.println("Calibrating MQ-2 sensor in clean air...");
    calibIndex = 0;
    ro = 0;
    rsSum = 0;
    state = CALIBRATING;
    lastTime = millis();
}

bool GasSensor::updateCalibration() {
    // Exit if not in calibration state
    if (state != CALIBRATING) return false;

    // Check if 200ms have passed since last sample
    // This timing creates spacing between measurements
    if (millis() - lastTime >= 200) {
        lastTime = millis();

        // Calculate sensor resistance from current ADC reading
        float rs = calculateResistance(analogRead(analogPin));
        
        // Accumulate only valid resistance values
        if (rs > 0) rsSum += rs;

        calibIndex++;

        // After 50 samples collected, calculate final Ro value
        if (calibIndex >= 50) {
            // Ro = average Rs / roCleanAirFactor
            // Example: if average Rs = 100 kΩ and factor = 10, then Ro = 10 kΩ
            ro = (rsSum / 50.0) / roCleanAirFactor;
            Serial.print("Calibration complete. Ro = ");
            Serial.print(ro);
            Serial.println(" KΩ");
            state = IDLE;
            return true;  // Signal that calibration finished
        }
    }

    return false;
}

// ===================================
// READING FUNCTIONS
// ===================================

// Start a new gas concentration reading cycle (collects 5 samples)
void GasSensor::startReading() {
    readIndex = 0;    // Reset sample counter
    rsSum = 0;        // Reset resistance accumulator
    lastTime = millis();
    state = READING;
    Serial.println("DEBUG: Starting gas reading...");
}

// Non-blocking update function - call repeatedly in main loop
// Returns true when reading cycle is complete
bool GasSensor::updateReading() {
    // Exit if not currently taking readings
    if (state != READING) {
        return false;
    }

    // Sample every 50ms (5 samples × 50ms = 250ms total reading time)
    if (millis() - lastTime >= 50) {
        lastTime = millis();

        // Read current ADC value and convert to sensor resistance
        int adcValue = analogRead(analogPin);
        float rs = calculateResistance(adcValue);
        
        Serial.print("  Sample ");
        Serial.print(readIndex + 1);
        Serial.print("/5 - ADC: ");
        Serial.print(adcValue);
        Serial.print(" Rs: ");
        Serial.println(rs);
        
        // Accumulate only valid resistance values
        if (rs > 0) rsSum += rs;

        readIndex++;

        // After collecting 5 samples, process the data
        if (readIndex >= 5) {
            // Calculate average resistance from 5 samples
            float rs_avg = rsSum / 5.0;
            
            // Calculate the ratio: current Rs divided by calibrated Ro
            // This ratio is used in the logarithmic equation to find PPM
            float ratio = rs_avg / ro;

            // Convert Rs/Ro ratio to PPM using the gas curve formula
            ppm = computePPM(ratio);

            // Classify gas level based on PPM concentration thresholds
            // These levels are typical for combustible gas/CO detection
            if (ppm < 200) gasLevelState = "safe";           // Normal air
            else if (ppm < 1000) gasLevelState = "caution";  // Minor gas present
            else if (ppm < 2000) gasLevelState = "check";    // Moderate concern
            else if (ppm < 5000) gasLevelState = "evacuate"; // High concentration
            else gasLevelState = "emergency";                 // Critical levels

            Serial.println("  → Reading complete");
            
            state = IDLE;
            return true;  // Signal that reading cycle finished
        }
    }

    return false;
}

// ===================================
// RESISTANCE CALCULATION
// ===================================
// Converts ADC value to sensor resistance (Rs) in kΩ
// Formula: Rs = RL × (ADC_max - ADC_value) / ADC_value
// 
// Explanation:
// - The sensor voltage divider: Vout = VCC × RS / (RS + RL)
// - Rearranging: RS = RL × (VCC/Vout - 1) = RL × (ADC_max/ADC_value - 1)
// - For 12-bit ADC (0-4095): RS = RL × (4095 - ADC) / ADC
float GasSensor::calculateResistance(int adcValue) {
    // Return error value if ADC reading is zero (prevents division by zero)
    if (adcValue <= 0) return -1;
    
    // Voltage divider calculation
    // ADC resolution: 4095 (12-bit) - adjust if using 10-bit (1023) or different platform
    return (rlValue * (4095.0 - adcValue)) / adcValue;
}

// ===================================
// PPM CALCULATION
// ===================================
// Converts Rs/Ro ratio to PPM (parts per million) concentration
// Uses logarithmic calibration curve from MQ-2 datasheet
// 
// The MQ-2 sensor characteristic curve is approximated by the equation:
// log10(PPM) = (log10(Rs/Ro) - b) / m + a
// 
// Where:
// - gasCurve[0] (a) = 2.3   (vertical offset)
// - gasCurve[1] (b) = 0.21  (x-intercept)
// - gasCurve[2] (m) = -0.47 (slope)
// 
// Solving for PPM: PPM = 10^(log10(PPM))
int GasSensor::computePPM(float ratio) {
    // Return 0 if ratio is invalid (prevents math errors)
    if (ratio <= 0) return 0;

    // Apply logarithmic formula to convert Rs/Ro ratio to PPM
    // Step 1: Calculate log10(ratio)
    // Step 2: Apply curve coefficients to linearize the relationship
    // Step 3: Raise 10 to this power to get PPM value
    float log_ppm = ((log10(ratio) - gasCurve[1]) / gasCurve[2]) + gasCurve[0];
    float ppm_calc = pow(10, log_ppm);

    // Validate result:
    // - isnan(): checks for NaN (Not a Number) errors
    // - isinf(): checks for infinity (caused by bad calibration)
    // - ppm_calc > 10000: cap at 10000 PPM as upper limit
    if (isnan(ppm_calc) || isinf(ppm_calc) || ppm_calc > 10000) return 0;

    // Convert to integer PPM value
    return (int)ppm_calc;
}