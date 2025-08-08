// sensor_pipeline.c
// C-style version of Sensor Data Processing

#include <stdio.h>
#include "sensor.h"

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
