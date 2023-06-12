#ifndef DEBUG_UTILS_H
#define DEBUG_UTILS_H

#ifdef DEBUG_PLUS
#define DEBUG_PRINT(str)               \
    Serial.print(millis());            \
    Serial.print(": ");                \
    Serial.print(__PRETTY_FUNCTION__); \
    Serial.print(' ');                 \
    Serial.print(__FILE__);            \
    Serial.print(':');                 \
    Serial.print(__LINE__);            \
    Serial.print(' ');                 \
    Serial.println(str);
#endif

#ifdef DEBUG
#define DEBUG_PRINT(str)    \
    Serial.print(millis()); \
    Serial.print(": ");     \
    Serial.println(str);

#else
#define DEBUG_PRINT(str)
#endif

/*
#define DEBUG_PRINT(str)               \
    Serial.print(millis());            \
    Serial.print(": ");                \
    Serial.print(__PRETTY_FUNCTION__); \
    Serial.print(' ');                 \
    Serial.print(__FILE__);            \
    Serial.print(':');                 \
    Serial.print(__LINE__);            \
    Serial.print(' ');                 \
    Serial.println(str);
#endif

#ifdef DEBUG
#define DEBUG_PRINT(str) \
    Serial.println(str);
#endif

#ifndef DEBUG_PRINT
#define DEBUG_PRINT(str)
#endif */

#endif // DEBUG_UTILS_H