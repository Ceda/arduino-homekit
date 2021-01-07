#include <ColorConverterLib.h>

#include <Arduino.h>
#include <arduino_homekit_server.h>
#include <WS2812FX.h>
#include "wifi_info.h"

#define LOG_D(fmt, ...)   printf_P(PSTR(fmt "\n") , ##__VA_ARGS__);

#define LED_PIN          2
#define LED_COUNT       40

WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

#define MODES_COUNT 13
int modeValues[MODES_COUNT] = {
    FX_MODE_STATIC,
    FX_MODE_BREATH,
    FX_MODE_RANDOM_COLOR,
    FX_MODE_MULTI_DYNAMIC,
    FX_MODE_RAINBOW_CYCLE,
    FX_MODE_SCAN,
    FX_MODE_DUAL_SCAN,
    FX_MODE_RUNNING_LIGHTS,
    FX_MODE_TWINKLE,
    FX_MODE_TWINKLE_RANDOM,
    FX_MODE_SPARKLE,
    FX_MODE_FIREWORKS,
    FX_MODE_FIRE_FLICKER};

bool received_sat = false;
bool received_hue = false;

bool is_on = false;
float current_brightness =  100.0;
float current_sat = 0.0;
float current_hue = 0.0;
int rgb_colors[3];

int modeIndex = 0;

void setup() {
	Serial.begin(115200);
	wifi_connect(); // in wifi_info.h

  ws2812fx.init();
  ws2812fx.setBrightness(current_brightness);
  ws2812fx.setColor(WHITE);
  ws2812fx.setSpeed(1000);
  ws2812fx.setMode(modeValues[0]);
  ws2812fx.start();

  rgb_colors[0] = 255;
  rgb_colors[1] = 255;
  rgb_colors[2] = 255;

  delay(1000);

	my_homekit_setup();
}

void loop() {
  ws2812fx.service();

	my_homekit_loop();
	delay(10);
}

//==============================
// HomeKit setup and loop
//==============================

// access your HomeKit characteristics defined in my_accessory.c

extern "C" homekit_server_config_t accessory_config;
extern "C" homekit_characteristic_t cha_on;
extern "C" homekit_characteristic_t cha_bright;
extern "C" homekit_characteristic_t cha_sat;
extern "C" homekit_characteristic_t cha_hue;
extern "C" homekit_characteristic_t cha_mode;
extern "C" homekit_characteristic_t cha_speed;

static uint32_t next_heap_millis = 0;

void my_homekit_setup() {
  cha_on.setter = set_on;
  cha_bright.setter = set_bright;
  cha_sat.setter = set_sat;
  cha_hue.setter = set_hue;
  cha_mode.setter = set_mode;
  cha_speed.setter = set_speed;

	arduino_homekit_setup(&accessory_config);

}

void my_homekit_loop() {
	arduino_homekit_loop();
	const uint32_t t = millis();
	if (t > next_heap_millis) {
		// show heap info every 5 seconds
		next_heap_millis = t + 5 * 1000;
		LOG_D("Free heap: %d, HomeKit clients: %d",
				ESP.getFreeHeap(), arduino_homekit_connected_clients_count());
	}
}

void set_on(const homekit_value_t v) {
    bool on = v.bool_value;
    cha_on.value.bool_value = on; //sync the value

    if(on) {
        is_on = true;
        Serial.println("On");
    } else  {
        is_on = false;
        Serial.println("Off");
    }

    updateColor();
}

void set_hue(const homekit_value_t v) {
    Serial.println("set_hue");
    float hue = v.float_value;
    cha_hue.value.float_value = hue; //sync the value

    current_hue = hue;
    received_hue = true;

    updateColor();
}

void set_mode(const homekit_value_t v) {
    Serial.println("set_mode");
    int mode_index = v.int_value;
    cha_mode.value.int_value = mode_index; //sync the value

    ws2812fx.setMode(modeValues[mode_index]);
}

void set_speed(const homekit_value_t v) {
    Serial.println("set_speed");
    int speed = v.int_value;
    cha_speed.value.int_value = speed; //sync the value

    ws2812fx.setSpeed(speed);

}

void set_sat(const homekit_value_t v) {
    Serial.println("set_sat");
    float sat = v.float_value;
    cha_sat.value.float_value = sat; //sync the value

    current_sat = sat;
    received_sat = true;

    updateColor();

}

void set_bright(const homekit_value_t v) {
    Serial.println("set_bright");
    int bright = v.int_value;
    cha_bright.value.int_value = bright; //sync the value

    current_brightness = bright;

    updateColor();
}

void updateColor()
{
  if(is_on)
  {
    if(received_hue && received_sat)
    {

      HSV2RGB(current_hue, current_sat, current_brightness);

      received_hue = false;
      received_sat = false;
     }

      ws2812fx.setColor(rgb_colors[0], rgb_colors[1], rgb_colors[2]);
      ws2812fx.setBrightness(current_brightness);

  }
  else if(!is_on) //lamp - switch to off
  {
      Serial.println("is_on == false");
      ws2812fx.setBrightness(0);
      ws2812fx.setColor(WHITE);
  }
}

void HSV2RGB(float h,float s,float v) {

  int i;
  float m, n, f;

  s/=100;
  v/=100;

  if(s==0){
    rgb_colors[0]=rgb_colors[1]=rgb_colors[2]=round(v*255);
    return;
  }

  h/=60;
  i=floor(h);
  f=h-i;

  if(!(i&1)){
    f=1-f;
  }

  m=v*(1-s);
  n=v*(1-s*f);

  switch (i) {

    case 0: case 6:
      rgb_colors[0]=round(v*255);
      rgb_colors[1]=round(n*255);
      rgb_colors[2]=round(m*255);
    break;

    case 1:
      rgb_colors[0]=round(n*255);
      rgb_colors[1]=round(v*255);
      rgb_colors[2]=round(m*255);
    break;

    case 2:
      rgb_colors[0]=round(m*255);
      rgb_colors[1]=round(v*255);
      rgb_colors[2]=round(n*255);
    break;

    case 3:
      rgb_colors[0]=round(m*255);
      rgb_colors[1]=round(n*255);
      rgb_colors[2]=round(v*255);
    break;

    case 4:
      rgb_colors[0]=round(n*255);
      rgb_colors[1]=round(m*255);
      rgb_colors[2]=round(v*255);
    break;

    case 5:
      rgb_colors[0]=round(v*255);
      rgb_colors[1]=round(m*255);
      rgb_colors[2]=round(n*255);
    break;
  }
}
