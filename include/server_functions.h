#ifndef _SERVER_FUNCTIONS_H

#define _SERVER_FUNCTIONS_H

#include <Arduino.h>

#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include "config.h"
#include "time_npt.h"
#include "log.h"

#define DEBUG
#include "debug_utils.h"

AsyncWebServer server_AP(80);

String res_enter_point()
{
  String response = "Al entrar en esta pagina el ESP32 se mantiene despierto."
                    "Resetear o actualizar para volver al funcionamient normal\n\n"
                    "Para actualizar, entrar a IP/update\n"
                    "Para reiniciar entrar a IP/reset\n\n\n"
                    "Ultimas medidas tomadas a %fecha%\n\n"
                    "Voltaje bateria: %vol_bat% V\n"
                    "Temperatura: %temp% C\n"
                    "Humedad: %hum% %\n"
                    "Presion atmosferica: %presion% hPa\n"
                    "Lluvia ultimos minutos: %lluvia% L/m^2\n"
                    "Dir. viento: %dir_viento%\n"
                    "Vel. media viento: %vel_media_viento% km/h\n"
                    "Vel. max. viento: %vel_max_viento% km/h\n"
                    "Vel. min. viento: %vel_min_viento% km/h\n";

  response.replace("%fecha%", get_date());
  response.replace("%vol_bat%", (String)voltaje_bat);
  response.replace("%temp%", (String)temp);
  response.replace("%hum%", (String)humedity);
  response.replace("%presion%", (String)presion);
  response.replace("%lluvia%", (String)liters_m2);
  response.replace("%dir_viento%", wind_direction);
  response.replace("%vel_media_viento%", (String)wind_avg);
  response.replace("%vel_max_viento%", (String)wind_max);
  response.replace("%vel_min_viento%", (String)wind_min);

  response.concat("\n\nLOG del ESP32\n\n");
  response.concat(read_log());

  return response;
}

// Inicia el servidor web
void init_server()
{
  server_AP.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
               { request->send(200, "text/plain", res_enter_point());
              keep_awake = true; });

  server_AP.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request)
               { request->send(200, "text/plain", "Reiniciando..."); 
              ESP.restart(); });
  // Inicia ElegantOTA
  AsyncElegantOTA.begin(&server_AP);

  // Inicia el servidor
  server_AP.begin();

  DEBUG_PRINT("Server HTTP started...");
  write_log("Server HTTP started...");
}

#endif //_SERVER_FUNCTIONS_H
