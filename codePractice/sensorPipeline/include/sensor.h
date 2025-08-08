#pragma once
#include <stdint.h>

// Transform Functions
float adc_to_voltage(uint16_t adc);
float voltage_to_temperature(float voltage);
float apply_calibration(float tempC);
float clamp_range(float tempC);

// Sensor processing pipeline
float process_temperature(uint16_t adc_val);