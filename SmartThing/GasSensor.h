#pragma once
#include <Arduino.h>

class GasSensor {
public:
    GasSensor(byte analogPin, float rlValue, float roCleanAirFactor, float* curve = nullptr);
    
    // Calibración bloqueante (NUEVO)
    void calibrateBlocking();
    
    // Calibración no bloqueante (anterior)
    void startCalibration();
    bool updateCalibration();
    
    void startReading();
    bool updateReading();
    
    int getPPM() { return ppm; }
    String getGasLevel() { return gasLevelState; }
    float getRo() { return ro; }

private:
    float calculateResistance(int adcValue);
    int computePPM(float rs_ro_ratio);
    
    enum State { IDLE, CALIBRATING, READING };
    State state;
    byte analogPin;
    float rlValue;
    float roCleanAirFactor;
    float gasCurve[3];
    float ro;
    unsigned long lastTime;
    int calibIndex;
    int readIndex;
    float rsSum;
    int ppm;
    String gasLevelState;
};