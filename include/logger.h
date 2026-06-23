#pragma once
#define DEBUG_LEVEL 1  // 0 = Silent, 1 = Production, 2 = Verbose Dev

#if DEBUG_LEVEL > 0
    void APP_LOG(const char *format, ...) __attribute__ ((format (printf, 1, 2)));
    void APP_LOG_Init(void);
#else
    // Compiles to literally zero CPU instructions when you build for customer release
    #define APP_LOG(format, ...)  do {} while(0)
    #define APP_LOG_Init(void)  do {} while(0)
#endif