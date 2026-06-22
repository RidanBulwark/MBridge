#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#include <stdint.h>

const char* float_to_str( float val, char *out_buf, size_t buf_size );

typedef struct {
    float    temperature;
    float    humidity;
    uint32_t timestamp_ms;
} SensorSample_t;

#endif