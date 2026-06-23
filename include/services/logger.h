#pragma once
#include <stdint.h>

const char* float_to_str( float val, char *out_buf, size_t buf_size );
void APP_LOG_Init(void);
void APP_LOG(const char *format, ...);