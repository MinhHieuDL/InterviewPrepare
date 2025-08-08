#include "sensor.h"

// ------------------------------
// Transform Functions
// ------------------------------
float adc_to_voltage(uint16_t adc) {
    const float v_ref = 3.3f;
    const int resolution = 1023; // 10-bit ADC
    return ((float)adc) * (v_ref / resolution);
}

float voltage_to_temperature(float voltage) {
    // Simple linear temp sensor: 10mV/degC with 500mV offset
    return (voltage - 0.5f) * 100.0f;
}

float apply_calibration(float tempC) {
    const float offset = 1.25f;
    return tempC + offset;
}

float clamp_range(float tempC) {
    if (tempC < 0.0f) return 0.0f;
    if (tempC > 100.0f) return 100.0f;
    return tempC;
}

// Sensor processing pipeline
float process_temperature(uint16_t adc_val) {
    float voltage = adc_to_voltage(adc_val);
    float temp = voltage_to_temperature(voltage);
    temp = apply_calibration(temp);
    return clamp_range(temp);
}