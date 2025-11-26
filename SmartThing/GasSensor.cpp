#include "GasSensor.h"

GasSensor::GasSensor(byte analogPin, float rlValue, float roCleanAirFactor, float* curve)
  : analogPin(analogPin),
    rlValue(rlValue),
    roCleanAirFactor(roCleanAirFactor),
    state(IDLE),
    lastTime(0),
    readIndex(0),
    calibIndex(0),
    rsSum(0)
{
    if (curve == nullptr) {
        gasCurve[0] = 2.3;
        gasCurve[1] = 0.21;
        gasCurve[2] = -0.47;
    } else {
        gasCurve[0] = curve[0];
        gasCurve[1] = curve[1];
        gasCurve[2] = curve[2];
    }

    ro = 10.0;
    ppm = 0;
    gasLevelState = "Normal";
}

// ===================================
// NUEVA FUNCIÓN: Calibración bloqueante
// ===================================
void GasSensor::calibrateBlocking() {
    Serial.println("Calibrando sensor MQ-2 en aire limpio (bloqueante)...");
    
    float rsTotal = 0;
    int samples = 50;
    
    for (int i = 0; i < samples; i++) {
        int adcValue = analogRead(analogPin);
        
        Serial.print("Muestra ");
        Serial.print(i + 1);
        Serial.print("/");
        Serial.print(samples);
        Serial.print(" - ADC: ");
        Serial.print(adcValue);
        
        float rs = calculateResistance(adcValue);
        
        Serial.print(" - Rs: ");
        Serial.print(rs);
        Serial.println(" kΩ");
        
        if (rs > 0) {
            rsTotal += rs;
        }
        
        delay(200); // 200ms entre muestras
    }
    
    ro = (rsTotal / samples) / roCleanAirFactor;
    
    Serial.println("─────────────────────────────");
    Serial.print("Calibración completada. Ro = ");
    Serial.print(ro);
    Serial.println(" kΩ");
    Serial.println("─────────────────────────────");
    
    state = IDLE;
}

// ===================================
// Calibración no bloqueante (original)
// ===================================
void GasSensor::startCalibration() {
    Serial.println("Calibrando sensor MQ-2 en aire limpio...");
    calibIndex = 0;
    ro = 0;
    rsSum = 0;
    state = CALIBRATING;
    lastTime = millis();
}

bool GasSensor::updateCalibration() {
    if (state != CALIBRATING) return false;

    if (millis() - lastTime >= 200) {
        lastTime = millis();

        float rs = calculateResistance(analogRead(analogPin));
        if (rs > 0) rsSum += rs;

        calibIndex++;

        if (calibIndex >= 50) {
            ro = (rsSum / 50.0) / roCleanAirFactor;
            Serial.print("Calibración completada. Ro = ");
            Serial.print(ro);
            Serial.println(" KΩ");
            state = IDLE;
            return true;
        }
    }

    return false;
}

// ===================================
// Lecturas
// ===================================
void GasSensor::startReading() {
    readIndex = 0;
    rsSum = 0;
    lastTime = millis();
    state = READING;
    Serial.println("DEBUG: Iniciando lectura...");
}

bool GasSensor::updateReading() {
    if (state != READING) {
        return false;
    }

    if (millis() - lastTime >= 50) {
        lastTime = millis();

        int adcValue = analogRead(analogPin);
        float rs = calculateResistance(adcValue);
        
        Serial.print("  Muestra ");
        Serial.print(readIndex + 1);
        Serial.print("/5 - ADC: ");
        Serial.print(adcValue);
        Serial.print(" Rs: ");
        Serial.println(rs);
        
        if (rs > 0) rsSum += rs;

        readIndex++;

        if (readIndex >= 5) {
            float rs_avg = rsSum / 5.0;
            float ratio = rs_avg / ro;

            ppm = computePPM(ratio);

            if (ppm < 200) gasLevelState = "seguro";
            else if (ppm < 1000) gasLevelState = "precaucion";
            else if (ppm < 2000) gasLevelState = "revisar";
            else if (ppm < 5000) gasLevelState = "evacuar";
            else gasLevelState = "emergencia";

            Serial.println("  → Lectura completada");
            
            state = IDLE;
            return true;
        }
    }

    return false;
}

float GasSensor::calculateResistance(int adcValue) {
    if (adcValue <= 0) return -1;
    return (rlValue * (4095.0 - adcValue)) / adcValue;
}

int GasSensor::computePPM(float ratio) {
    if (ratio <= 0) return 0;

    float log_ppm = ((log10(ratio) - gasCurve[1]) / gasCurve[2]) + gasCurve[0];
    float ppm_calc = pow(10, log_ppm);

    if (isnan(ppm_calc) || isinf(ppm_calc) || ppm_calc > 10000) return 0;

    return (int)ppm_calc;
}