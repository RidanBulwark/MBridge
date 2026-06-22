#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#include <stdint.h>

typedef struct {
    float    temperature;
    float    humidity;
    uint32_t timestamp_ms;
} SensorSample_t;

#endif