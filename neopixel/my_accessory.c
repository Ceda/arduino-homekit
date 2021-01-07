/*
 * my_accessory.c
 * Define the accessory in C language using the Macro in characteristics.h
 *
 *  Created on: 2020-05-15
 *      Author: Mixiaoxiao (Wang Bin)
 */

#include <homekit/homekit.h>
#include <homekit/characteristics.h>

#define UUID_MODE "1C52000A-457C-4D3C-AABA-E6F207422A10"
#define UUID_SPEED "4CB2534E-9C95-4C4F-A746-36238BAACE3A"

void my_accessory_identify(homekit_value_t _value) {
	printf("accessory identify\n");
}

homekit_characteristic_t cha_on = HOMEKIT_CHARACTERISTIC_(ON, false);
homekit_characteristic_t cha_name = HOMEKIT_CHARACTERISTIC_(NAME, "ESP8266 Lamp");
homekit_characteristic_t cha_bright = HOMEKIT_CHARACTERISTIC_(BRIGHTNESS, 50);
homekit_characteristic_t cha_sat = HOMEKIT_CHARACTERISTIC_(SATURATION, (float) 0);
homekit_characteristic_t cha_hue = HOMEKIT_CHARACTERISTIC_(HUE, (float) 180);

homekit_characteristic_t cha_mode = HOMEKIT_CHARACTERISTIC_(
                                CUSTOM,
                                .type = UUID_MODE,
                                .description = "fxMode",
                                .format = homekit_format_int,
                                .permissions = homekit_permissions_paired_read
                                               | homekit_permissions_paired_write,
                                .min_value = (float[]) {0},
                                .max_value = (float[]) {7},
                                .min_step = (float[]) {1},
                                .value = HOMEKIT_INT_(0)
                                );
homekit_characteristic_t cha_speed = HOMEKIT_CHARACTERISTIC_(
                                CUSTOM,
                                .type = UUID_SPEED,
                                .description = "Speed",
                                .format = homekit_format_int,
                                .permissions = homekit_permissions_paired_read
                                               | homekit_permissions_paired_write,
                                .min_value = (float[]) {5000},
                                .max_value = (float[]) {100},
                                .min_step = (float[]) {100},
                                .value = HOMEKIT_INT_(1000)
                                );


homekit_accessory_t *accessories[] = {
    HOMEKIT_ACCESSORY(.id=1, .category=homekit_accessory_category_lightbulb, .services=(homekit_service_t*[]) {
        HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]) {
            HOMEKIT_CHARACTERISTIC(NAME, "ESP8266 Lamp"),
            HOMEKIT_CHARACTERISTIC(MANUFACTURER, "Arduino HomeKit"),
            HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "0123456"),
            HOMEKIT_CHARACTERISTIC(MODEL, "ESP8266/ESP32"),
            HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "1.0"),
            HOMEKIT_CHARACTERISTIC(IDENTIFY, my_accessory_identify),
            NULL
        }),
        HOMEKIT_SERVICE(LIGHTBULB, .primary=true, .characteristics=(homekit_characteristic_t*[]) {
            &cha_on,
            &cha_name,
            &cha_bright,
            &cha_sat,
            &cha_hue,
            &cha_speed,
            &cha_mode,
            NULL
        }),
        NULL
    }),
    NULL
};

homekit_server_config_t accessory_config = {
    .accessories = accessories,
    .password = "111-11-111"
};
