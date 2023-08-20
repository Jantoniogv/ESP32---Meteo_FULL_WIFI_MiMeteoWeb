#include <Arduino.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ArduinoJson.h>

#include "server_functions.h"
#include "sensor_functions.h"
#include "wifi_functions.h"
#include "config.h"
#include "log.h"

#define DEBUG
#include "debug_utils.h"

void setup()
{
  //***** Inicializamos el pin del pluviometro y su interrupcion *****//
  pinMode(RAIN_SENSOR, INPUT);

  attachInterrupt(RAIN_SENSOR, sumLiters_m2, FALLING);
  delay(50);

  //***** Inicializamos el pin del anemometro y su interrupcion *****//
  pinMode(ANEMOMETER_SENSOR, INPUT);

  attachInterrupt(ANEMOMETER_SENSOR, wind_velocity_interrupt, RISING);

  //***** Establece la velocidad para el Monitor Serie *****//
  Serial.begin(115200);
  delay(100);

  //***** Configuramos el modo sleep del ESP32 *****//
  esp_sleep_enable_timer_wakeup(TIME_SLEEP * S_a_M_FACTOR * uS_a_S_FACTOR);

  //***** Configura el pin para despertar al esp32 cuando se reciba un valor bajo en el pin conectado al pluviometro *****//
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_25, 0);

  //***** Configura el pin para despertar al esp32 cuando se reciba un valor alto en el pin 27 conectado a un interruptor externo *****//
  // esp_sleep_enable_ext1_wakeup(PIN_WAKE_UP_SERVER, ESP_EXT1_WAKEUP_ANY_HIGH);

  delay(100);

  //***** Mide el voltaje de la bateria y si es inferior a 3.0 V, se duerme al ESP32 *****//
  if (read_voltaje() < voltaje_bat_min)
  {
    DEBUG_PRINT("Voltaje bajo, ESP32 a dormir");

    esp_deep_sleep_start();
  }

  //***** Suma la cantidad de un vaso del pluviometro, si este ha despertado al ESP32 *****//
  wakeup_reason = esp_sleep_get_wakeup_cause();

  if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0)
  {
    liters_m2 += u_pluviometro;
  }

  // Reinicia  el ESP32 cada 144 veces despierto, que coincide con aproximadamente cada 24 horas
  if (send_data_count >= 144)
  {
    ESP.restart();
  }

  //***** Se conecta al wifi *****//
  connectedWiFi();

  // Si llega ha este punto se ha iniciado correctamente y se detalla en el log
  write_log("Iniciado correctamente...");

  delay(100);
}

void loop()
{

  //***** Obtnemos el minuto de la hora, a fin de calcular si ha pasado el tiempo suficiente para realizar las medidas de nuevo *****//
  current_minute = get_minute();

  int diff_minutes_last_send_data = current_minute - last_minute;

  // Corregimos a formato sesagesimal en caso de que haya cambio de hora
  if (diff_minutes_last_send_data < 0)
  {
    diff_minutes_last_send_data += 60;
  }

  if ((diff_minutes_last_send_data >= MINUTES_BETWEEN_SEND_DATA) || send_data_count == 0)
  {
    send_data = true;
  }
  else
  {
    send_data = false;
  }

  //***** Si la conexion sigue activa, y han pasado los minutos establecidos entre datos enviados, mandamos los datos al servidor *****//
  if (WiFi.status() == WL_CONNECTED && current_minute != -1)
  {
    if (send_data == true)
    {

      //***** Establecemos el ultimo tiempo que se envio los datos al principio a fin de que el tiempo entre envio *****//
      //***** de datos sea de 10 minutos *****//
      last_minute = current_minute;

      HTTPClient client;

      //***** Inicializamos el sensor BME280 *****//
      if (bme.begin())
      {
        DEBUG_PRINT("BME280 encontrado!");
        write_log("BME280 encontrado!");

        //***** Leemos los datos del sensor BME280*****//
        temp = bme.readTemperature();       // Almacena en variable el valor de temperatura
        presion = bme.readPressure() / 100; // Almacena en variable el valor de presion divido por 100 para covertirlo a hectopascales
        humedity = bme.readHumidity();      // Almacena en variable el valor de humedad
      }
      else
      {
        // Si falla la comunicacion con el sensor mostrar texto y detener flujo del programa
        DEBUG_PRINT("BME280 no encontrado!");
        write_log("BME280 no encontrado!");

        // while (1);
      }

      //***** Durante este bucle se toman cinco medidas de la velocidad del viento, cada una de seis mil milisegundos *****//
      for (int i = 0; i < 5; i++)
      {
        rev_anemometer = 0;
        delay(6000);

        wind_velocity[i] = (float)rev_anemometer * 0.4; // Se almacena en el array cada medida en km/h, habiendose determinado que las revoluciones contadas en los seis
        // segundos multiplicadas por 0,4 nos da la velocidad en km/h, siendo una rev/s equivalente a 2,4 km/h

        DEBUG_PRINT("Velocidad del viento: " + String(wind_velocity[i]) + " km/s");
      }

      //***** Leemos la direccion del viento desde la veleta *****//
      wind_direction = read_wind_direction();

      delay(100);

      //***** Pasamos a String los valores de los sensores *****//
      s_Temp = String(temp);
      s_Presion = String(presion);
      s_Humedity = String(humedity);
      s_liters_m2 = String(liters_m2);
      s_wind_max = String(max_velocity());
      s_wind_min = String(min_velocity());
      s_wind_avg = String(avg_velocity());
      s_voltaje = String(voltaje_bat);

      s_Temp.replace('.', ',');
      s_Presion.replace('.', ',');
      s_Humedity.replace('.', ',');
      s_liters_m2.replace('.', ',');
      s_wind_max.replace('.', ',');
      s_wind_min.replace('.', ',');
      s_wind_avg.replace('.', ',');
      s_voltaje.replace('.', ',');

      // Contruimos la parte 'GET' de la peticion que enviamos al servidor
      // s_GET = "?temperature=" + s_Temp + "&humidity=" + s_Humedity + "&presion=" + s_Presion + "&rain=" + s_liters_m2 + "&wind_direction=" + wind_direction + "&avg_wind=" + s_wind_avg + "&max_wind=" + s_wind_max + "&min_wind=" + s_wind_min + "&voltaje=" + s_voltaje;

      //***** Conectamos a google script y enviamos los datos mediante GET, a una app que los almacena en google sheets *****//
      // DEBUG_PRINT("Starting connection to google scripts...");

      /* if (client.begin(serverURL))
      {
        // client.addHeader("Content-Type", "application/json");

        int httpCode = client.POST(s_GET);

        if (httpCode > 0)
        {
          DEBUG_PRINT("Statuscode: " + String(httpCode));

          client.end();
        }
        else
        {
          DEBUG_PRINT("Error on HTTP request to script.google.com");
        }
      }
      else
      {
        DEBUG_PRINT("Error conection to script.google.com");
      } */

      //***** Conectamos a mimeteoweb API y enviamos los datos mediante POST, es una API que luego los sirve mediante un servicio web *****//
      DEBUG_PRINT("Starting connection to mimetoweb...");
      write_log("Starting connection to mimetoweb...");

      if (client.begin(mimeteowebURL))
      {
        client.addHeader("Content-Type", "application/json");

        String jsonOutput;
        const size_t CAPACITY = JSON_OBJECT_SIZE(13);
        StaticJsonDocument<CAPACITY> doc;

        JsonObject object = doc.to<JsonObject>();
        object["location"] = "iznajar";
        object["date"] = get_date();
        object["temp"] = temp;
        object["hum"] = humedity;
        object["pressure"] = presion;
        object["water"] = liters_m2;
        object["avg_wind"] = avg_velocity();
        object["min_wind"] = min_velocity();
        object["max_wind"] = max_velocity();
        object["dir_wind"] = wind_direction;
        object["voltaje"] = voltaje_bat;

        serializeJson(doc, jsonOutput);

        // DEBUG_PRINT(jsonOutput);

        int httpCode = client.POST(jsonOutput);

        DEBUG_PRINT("Data send to mimeteoweb...");
        write_log("Data send to mimeteoweb...");

        DEBUG_PRINT("Status code http request: " + String(httpCode));
        write_log("Status code http request: " + String(httpCode));

        client.end();
      }
      else
      {
        DEBUG_PRINT("Error conection to mimeteoweb");
        write_log("Error conection to mimeteoweb");
      }

      delay(1000);

      //***** Imprimimos los valores de la estación en la consola *****//
      print_data_serial();

      //***** Resetea la lluvia caida *****//
      liters_m2 = 0;

      //***** Establece en false el envio de datos hasta que pase de nuevo 10 minutos *****//
      send_data = false;

      // Suma las veces que ha enviado datos
      send_data_count++;
    }

    //***** ESP32 a dormir si procede ******//

    //***** Obtnemos el minuto de la hora a fin de determinar la diferencia de tiempo entre el ultimo envio de datos *****//
    diff_minutes_last_send_data = get_minute() - last_minute;

    // Corregimos a formato sesagesimal en caso de que haya cambio de hora
    if (diff_minutes_last_send_data < 0)
    {
      diff_minutes_last_send_data += 60;
    }

    DEBUG_PRINT("Diferencia desde ultimo envio: " + String(diff_minutes_last_send_data));

    if ((!keep_awake) && (diff_minutes_last_send_data > TIME_SERVER))
    {
      DEBUG_PRINT("ESP32 to sleep!");

      //***** Configuramos el modo sleep del ESP32 *****//
      esp_sleep_enable_timer_wakeup((TIME_SLEEP - diff_minutes_last_send_data) * S_a_M_FACTOR * uS_a_S_FACTOR);

      esp_deep_sleep_start();
    }
  }
  else
  {
    WiFi.disconnect();

    DEBUG_PRINT("Desconectado de WiFi STA!!");
    write_log("Desconectado de WiFi STA!!");

    //***** Se intenta conectar de nuevo al WiFi *****//
    connectedWiFi();
  }
}