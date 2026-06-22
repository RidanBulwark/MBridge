#ifndef APP_HAL_H
#define APP_HAL_H

#include <stdint.h>

// 0 = Success, negative = Error
int APP_I2C_ReadRegister(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);
int APP_MQTT_Publish(const char *topic, const char *payload);

#endif