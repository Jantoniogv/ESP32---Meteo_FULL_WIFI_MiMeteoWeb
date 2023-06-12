#ifndef _TIME_NPT_H_
#define _TIME_NPT_H_

#include <Arduino.h>

#include "config.h"

#define DEBUG
#include "debug_utils.h"

// Inicia la configuracion del servidor NTP
void time_npt_init()
{
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

//***** Funciones que obtienen la hora y los minutos actuales *****//
int getMinutes()
{
    struct tm timeinfo;

    if (!getLocalTime(&timeinfo))
    {
        DEBUG_PRINT("Failed to obtain time");
        return -1;
    }
    else
    {
        char char_current_minute[3];
        strftime(char_current_minute, 3, "%M", &timeinfo);
        DEBUG_PRINT((String)char_current_minute);

        return atoi(char_current_minute);
    }
}

String getdate()
{
    struct tm timeinfo;

    if (!getLocalTime(&timeinfo))
    {
        DEBUG_PRINT("Failed to obtain time");
        return "-1";
    }
    else
    {
        char char_current_date[20];
        strftime(char_current_date, 20, "%Y-%m-%dT%H:%M:%S", &timeinfo);
        DEBUG_PRINT((String)char_current_date);

        return (String)char_current_date;
    }
}

// int getHour() {
//   struct tm timeinfo;
//
//   if (!getLocalTime(&timeinfo)) {
//     DEBUG_PRINT("Failed to obtain time");
//     return -1;
//   }
//   else {
//     char current_hour[3];
//     strftime(current_hour, 3, "%H", &timeinfo);
//     DEBUG_PRINT(current_hour);
//     return atoi(current_hour);
//   }
// }

#endif //_TIME_NPT_H_