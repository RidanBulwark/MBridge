#include "app_hal.h"
#include <stdio.h>
#include <stdlib.h>

// Stateful mock: simulates an SHT31 sensor warming up over time
int APP_I2C_ReadRegister(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len) {
    static uint16_t fake_temp_raw = 24000; // Roughly 19°C
    
    if (dev_addr != 0x44 || len < 4) return -1; // NACK

    fake_temp_raw += (rand() % 50) - 20; // Wander slightly

    // Pack fake temp
    data[0] = (fake_temp_raw >> 8) & 0xFF;
    data[1] = fake_temp_raw & 0xFF;
    // Pack fake humidity (static 50%)
    data[2] = 0x80; 
    data[3] = 0x00;

    return 0; // ACK
}

int APP_MQTT_Publish(const char *topic, const char *payload) {
    // In a pure POSIX simulator, we can literally call out to the host OS!
    // In raw ARM QEMU, we print to the semi-hosting UART Console.
    
    printf("[SIM-MQTT Broker Trap] Topic: '%s' | Payload: %s\n", topic, payload);
    return 0;
}