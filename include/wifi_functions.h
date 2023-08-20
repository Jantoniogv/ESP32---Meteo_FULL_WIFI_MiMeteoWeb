#ifndef _WIFI_FUNCTIONS_H

#define _WIFI_FUNCTIONS_H

//***** Incluimos la librerias necesarias *****//
#include <Arduino.h>

#include <WiFi.h>
#include <HTTPClient.h>
#include "server_functions.h"
#include "config.h"
#include "time_npt.h"
#include "log.h"

#define DEBUG
#include "debug_utils.h"

//***** Conecta al ESP32 si a internet mediante wifi *****//
void connectedWiFi()
{
    // Configuramos el modo del wifi
    WiFi.mode(initWifiType);

    // Conexion AP WiFi
    WiFi.softAP(initSsidAP, initPassAP);

    // Asignamos la IP del punto de acceso
    WiFi.softAPConfig(init_IP_ap, init_IP_ap, subnet);

    DEBUG_PRINT("SsidAP: " + String(initSsidAP) + "PassAP: " + String(initPassAP));
    write_log("SsidAP: " + String(initSsidAP) + "PassAP: " + String(initPassAP));

    DEBUG_PRINT("IP as soft AP: " + WiFi.softAPIP().toString());
    write_log("IP as soft AP: " + WiFi.softAPIP().toString());

    // Se inicia el servidor web
    init_server();

    // Conexion STA WiFi
    int wifi_count = 0;

    DEBUG_PRINT("Conecting WiFi STA a SSID: " + String(initSsidSTA));

    WiFi.begin(initSsidSTA, initPassSTA);

    // attempt to connect to Wifi network:
    while (WiFi.status() != WL_CONNECTED && wifi_count < 20)
    {
        DEBUG_PRINT(".");

        // Espera un segundo para volver a intentar
        delay(1000);

        wifi_count++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        DEBUG_PRINT("Conected to " + String(initSsidSTA));
        write_log("Conected to " + String(initSsidSTA));

        DEBUG_PRINT("IP local " + WiFi.localIP().toString());
        write_log("IP local " + WiFi.localIP().toString());

        // Inicia la configuracion del servidor NTP
        time_npt_init();

        // client.setCACert(root_ca);
        // client.setCACert(root_ca2);
    }
    else
    {

        DEBUG_PRINT("Fallo conexion WiFi STA, espera dos minutos conexiones en modo AP y se va a modo sleep...");
        write_log("Fallo conexion WiFi STA, espera dos minutos conexiones en modo AP y se va a modo sleep...");

        delay(TIME_SERVER * S_a_M_FACTOR * 1000);

        DEBUG_PRINT("ESP32 to sleep!");

        esp_deep_sleep_start();
    }
}

#endif //_WIFI_FUNCTIONS_H