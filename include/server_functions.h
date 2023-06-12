#ifndef _SERVER_FUNCTIONS_H

#define _SERVER_FUNCTIONS_H

#include <Arduino.h>

#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

#define DEBUG
#include "debug_utils.h"

AsyncWebServer server_OTA(80);

// Inicia el servidor web
void init_server()
{
  server_OTA.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send(200, "text/plain", "Hi! I am ESP32."); });

  // Inicia ElegantOTA
  AsyncElegantOTA.begin(&server_OTA);

  // Inicia el servidor
  server_OTA.begin();

  DEBUG_PRINT("Servidor HTTP iniciado...");
}

#endif //_SERVER_FUNCTIONS_H
