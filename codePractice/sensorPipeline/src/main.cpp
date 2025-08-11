// sensor_pipeline.c
// C-style version of Sensor Data Processing

#include <stdio.h>
#include <stdint.h>

auto adc_to_voltage = [](uint16_t adc) {
    const float v_ref = 3.3f;
    const int resolution = 1023; // 10-bit ADC
    return ((float)adc) * (v_ref / resolution);
};

auto voltage_to_temperature = [](float voltage) {
    // Simple linear temp sensor: 10mV/degC with 500mV offset
    return (voltage - 0.5f) * 100.0f;
};

auto apply_calibration = [](float tempC) {
    const float offset = 1.25f;
    return tempC + offset;
};

auto clamp_range = [](float tempC) {
    if (tempC < 0.0f) return 0.0f;
    if (tempC > 100.0f) return 100.0f;
    return tempC;
};

// Sensor processing pipeline
auto process_temperature =[](uint16_t adc_val) {
    float voltage = adc_to_voltage(adc_val);
    float temp = voltage_to_temperature(voltage);
    temp = apply_calibration(temp);
    return clamp_range(temp);
};

int main() {
    uint16_t adc_samples[] = { 100, 200, 400, 512, 768, 900, 1023 };
    size_t count = sizeof(adc_samples) / sizeof(adc_samples[0]);

    printf("Processed Temperature Readings:\n");
    for (size_t i = 0; i < count; ++i) {
        float temp = process_temperature(adc_samples[i]);
        printf("ADC: %u  ->  Temp: %.2f°C\n", adc_samples[i], temp);
    }

    return 0;
}
