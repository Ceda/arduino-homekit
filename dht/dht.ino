#include <DHT.h>
#include <DHT_U.h>

#include <Arduino.h>
#include <arduino_homekit_server.h>
#include <homekit/types.h>
#include "wifi_setup.h"

#define DHTPIN  2
#define DHTTYPE DHT11

#define LOG_D(fmt, ...)   printf_P(PSTR(fmt "\n") , ##__VA_ARGS__);

DHT dht(DHTPIN, DHTTYPE);

void setup() {
    Serial.begin(115200);
    wifi_connect(); // in wifi_setup.h

    dht.begin();

    my_homekit_setup();
}

void loop() {
    my_homekit_loop();
    delay(10);
}

//==============================
// HomeKit setup and loop
//==============================

// access your HomeKit characteristics defined in accessory.c

extern "C" homekit_server_config_t accessory_config;
extern "C" homekit_characteristic_t temperature;
extern "C" homekit_characteristic_t humidity;

static uint32_t next_heap_millis = 0;
static uint32_t next_report_millis = 0;

void my_homekit_setup() {
    arduino_homekit_setup(&accessory_config);
}

void my_homekit_loop() {
    arduino_homekit_loop();
    const uint32_t t = millis();

    if (t > next_report_millis) {
      // report sensor values every 10 seconds
      next_report_millis = t + 10 * 1000;
      my_homekit_report();
    }

    if (t > next_heap_millis) {
        // show heap info every 5 seconds
        next_heap_millis = t + 5 * 1000;
        LOG_D("Free heap: %d, HomeKit clients: %d",
            ESP.getFreeHeap(), arduino_homekit_connected_clients_count());
    }
}

void my_homekit_report() {
  float temperature_value = dht.readTemperature();
  float humidity_value    = dht.readHumidity();

  temperature.value.float_value = temperature_value;
  humidity.value.float_value = humidity_value;

  LOG_D("Current temperature: %.1f", temperature_value);
  LOG_D("Current humidity: %.1f", humidity_value);
  
  homekit_characteristic_notify(&temperature, temperature.value);
  homekit_characteristic_notify(&humidity, humidity.value);
}
