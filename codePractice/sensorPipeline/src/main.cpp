
#include <iostream>
#include <cstdint>

using namespace std;

constexpr float v_ref = 3.3f;
constexpr int resolution = 1023; // 10-bit ADC
constexpr float tempOffsetV = 0.5f; // 500mv offset 
constexpr float degPerVold = 100.0f;
constexpr float callOffsetDegC = 1.25f;
constexpr float minDefC = 0.0f;
constexpr float maxDefC = 100.0f;

auto adc_to_voltage = [](uint16_t adc) {
    return ((float)adc) * (v_ref / resolution);
};

auto voltage_to_temperature = [](float voltage) {
    return (voltage - tempOffsetV) * degPerVold;
};

auto apply_calibration = [](float tempC) {
    return tempC + callOffsetDegC;
};

auto clamp_range = [](float tempC) {
    if (tempC < minDefC) return minDefC;
    if (tempC > maxDefC) return maxDefC;
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

    cout << "Processed Temperature Readings" << '\n';
    for (auto adc : adc_samples) {
        float temp = process_temperature(adc);
        cout << "ADC: " << adc << " ->  Temp: " << temp << '\n';
    }

    return 0;
}
