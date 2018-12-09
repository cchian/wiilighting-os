
#include <driver/adc.h> //https://esp-idf.readthedocs.io/en/v2.0/api/peripherals/adc.html

void EnvSensor_Task(void *p) { //loop6
  //pinMode(temperature_sensor_pin,INPUT);
  adc1_config_width(ADC_WIDTH_12Bit);
  adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_0db);
  
  while (1) {
    temperature_sensor = map(adc1_get_voltage(ADC1_CHANNEL_6), 0, 4095, 0, 300);
    delay(500);
  }
}

