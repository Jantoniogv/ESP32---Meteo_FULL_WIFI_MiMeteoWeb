#include <Arduino.h>

// Pluviometro pin
#define RAIN_SENSOR 12

// Definimos el pin 35 como pin de obtencion de datos de la veleta
#define WIND_DIRECTION_SENSOR 35

// Definimos el pin 34 como pin de obtencion de datos del anemometro
#define ANEMOMETER_SENSOR 34

// Definimos el pin 32 como medidor de voltaje
#define VOLTAJE_BATTERY 13

//***** Establecemos los factores de tiempo *****//
#define MINUTES_BETWEEN_SEND_DATA 10 /* Tiempo entre cada envio de datos */

#define uS_a_S_FACTOR 1000000 /* x 1000000 para tener segundos */

#define TIME_SLEEP 600 /* Tiempo que duerme en segundos */

#define mS_a_S_FACTOR 1000 /* x 1000 para tener segundos */

#define TIME_RAIN 300 /* Tiempo minimo que esta despierto en segundos, en caso de despertar por lluvia */

//***** Incluimos la libreria WiFi que permiten una conexion segura *****//
//#include <WiFiClientSecure.h>
#include <WiFi.h>
#include <HTTPClient.h>

//***** Incluimos las librerias del senser BME280 *****//
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include <ArduinoJson.h>

//***** Constantes y objetos necesarios para realizar la conexion WiFi *****//
const char *ssid = "Router_Casa_Juncares"; // your network SSID (name of wifi network)
const char *password = "Red_Juncares";     // your network password

//***** Constantes que definen los server y url a los que se conecta el ESP32 y envia los datos *****//
const char *server = "script.google.com"; // Server

const String serverURL = "https://script.google.com/macros/s/AKfycbyf-0jMuXnlqXoohD-7WVzVF8J8rs5QlrM02nCP6geWslt-PAVYDesZdD2zqWCUN54/exec"; // URL

const char *mimeteowebServer = "mimeteoweb.up.railway.app";
const String mimeteowebURL = "https://mimeteoweb.up.railway.app/api/v1/add-meteo-dates";

//***** Constantes y objetos necesarios para realizar la conexion NTP y obtener la hora *****//
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 0;

//***** Variables para almacenar los datos de las mediciones del BME280 *****//
float temp = 0;     // variable para almacenar valor de temperatura
float presion = 0;  // variable para almacenar valor de presion atmosferica
float humedity = 0; // variable para almacenar valor de presion atmosferica

String s_Temp;
String s_Presion;
String s_Humedity;

//***** Variable en las que se almacenara los valores medidos para la lluvia *****//
RTC_DATA_ATTR float liters_m2 = 0; // Esta variable estara almacenada en la memoria del reloj RTC a fin de que se guarde su valor durante los periodos de sueño

String s_liters_m2; // String donde se guardara la lluvia medida para enviar los datos al servidor

unsigned long last_t_liters = 0;    // Variable que almacena el ultimo tiempo en ms cuado se registro un pulso del pluviometro
unsigned long current_t_liters = 0; // Variable que almacena el tiempo actual en ms del pulso del pluviometro

int measurements_t_liters = 500; // Tiempo en ms, en los que se desprecia los pulsos registrado por el pluviometro, a fin de desprecia los rebotes

//***** Variables del anemometro *****//
volatile int rev_anemometer = 0; // Esta variable la encargada de almacenar las rpm  que se leen desde el anemometro

float wind_velocity[5]; // Este array almacena cinco medidas del anemometro a fin de realizar una media de entre ellos

String s_wind_max;
String s_wind_min;
String s_wind_avg;

//***** Variables de la veleta *****//

// Arrays que identifican las distintas direcciones del viento, y definen el valor maximo y minimo que toma la lectura analogica del sensor
String wind_dir_comp[] = {"NORTE", "NORTE-NOROESTE", "NOROESTE", "ESTE-NOROESTE", "ESTE", "ESTE-SURESTE", "SURESTE", "SUR-SURESTE",
                          "SUR", "SUR-SUROESTE", "SUROESTE", "OESTE-SUROESTE", "OESTE", "OESTE-NOROESTE", "NOROESTE", "NORTE-NOROESTE", "UNDEFINED"};

int sensor_wind_min[] = {750, 2340, 2100, 3950, 3800, 4000, 3200, 3600, 2700, 3000, 1400, 1500, 100, 600, 300, 1100};
int sensor_wind_max[] = {900, 2380, 2200, 3980, 3940, 4095, 3400, 3700, 2950, 3100, 1499, 1600, 200, 700, 599, 1200};

// Estas variables sirven para almacenar la direccion del viento, el valor leido del sensor y la media del valor despues de varias medidas
String wind_direction;

// Almacena la lectura del voltaje
float voltaje_bat;
String s_voltaje;
// int wind_direction_avg = 0;

//***** Esta variable almacena en formato String los datos a subir al servidor *****//
String s_GET;

//***** Estas variales se encargan de almacenar la ultima vez que se envio datos y el minuto actual *****//
RTC_DATA_ATTR int last_minute = 0;
int current_minute = 0;

//***** Variable para contar las veces que el ESP32 ha despertado *****//
RTC_DATA_ATTR int boot_sleep_count = 0;

//***** Esta variable se encarga de establecer cuando se envia datos 'true' y cuando no 'false' ******//
bool send_data = false;

//***** Creamos el objeto del sensor BME280 *****//
Adafruit_BME280 bme;

//***** Esta variable almacena la razon por la que el ESP32 ha despertado *****//
esp_sleep_wakeup_cause_t wakeup_reason;

//***** Estas variables se encargan de establecer el tiempo que el ESP32 lleva despierto *****//
unsigned long last_t_sleep = 0;
unsigned long current_t_sleep = 0;

//***** Declaramos las funciónes del programa *****//
void connectedWiFi();   // Conectar al wifi
void printDataSerial(); // Imprimir datos en el monitor serie

int getMinutes(); // Obtiene los minutos actuales

String getdate(); // Obtiene la fecha actual

// int getHour(); //Obtiene la hora actual

void sumLiters_m2(); // Esta función se ejecutara cuando se active el pluviometro

void wind_velocity_interrupt(); // Funcion que se activa en cada interrupcion producida por el anemometro

// Estas funciones calculan el valor maximo, minimo y la media de las diez medidas tomadas del anemometro
float max_velocity();
float min_velocity();
float avg_velocity();

String read_wind_direction(); // Funcion que lee la direccion del viento

//***** Funcion para medir el voltaje de la bateria *****//
float read_voltaje();
float altitud;

void setup()
{

  //***** Inicializamos el pin del pluviometro y su interrupcion *****//
  pinMode(RAIN_SENSOR, INPUT_PULLDOWN);

  attachInterrupt(RAIN_SENSOR, sumLiters_m2, RISING);
  delay(50);

  //***** Inicializamos el pin del anemometro y su interrupcion *****//
  pinMode(ANEMOMETER_SENSOR, INPUT);

  attachInterrupt(ANEMOMETER_SENSOR, wind_velocity_interrupt, RISING);

  //***** Establece la velocidad para el Monitor Serie *****//
  Serial.begin(115200);
  delay(100);

  //***** Configuramos el modo sleep del ESP32 *****//
  esp_sleep_enable_timer_wakeup(TIME_SLEEP * uS_a_S_FACTOR);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_12, 1);

  delay(100);

  //***** Mide el voltaje de la bateria y si es inferior a 2,7 V, se duerme al ESP32 *****//
  if (read_voltaje() < 2.7)
  {

    Serial.println("Voltaje bajo, ESP32 a dormir");

    esp_deep_sleep_start();
  }
  //***** Se conecta al wifi *****//
  connectedWiFi();

  //***** Configuramos la hora en funcion del servidor NTP *****//
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  delay(100);

  //***** Suma la cantidad de un vaso del pluviometro, si este ha despertado al ESP32 *****//
  wakeup_reason = esp_sleep_get_wakeup_cause();

  if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0)
  {
    liters_m2 += 0.3537;
  }

  boot_sleep_count++; // Suma las veces que ha despertado

  if (boot_sleep_count >= 144)
  {
    ESP.restart();
  }

  delay(100);
}

void loop()
{

  //***** Obtnemos el minuto de la hora, a fin de calcular si ha pasado el tiempo suficiente para realizar las medidas de nuevo *****//
  current_minute = getMinutes();

  int diff_minutes = current_minute - last_minute;

  // Corregimos a formato sesagesimal en caso de que haya cambio de hora
  if (diff_minutes < 0)
  {
    diff_minutes += 60;
  }

  // Serial.println("diff_minutes: " + String(abs_diff_minutes));

  if ((diff_minutes >= MINUTES_BETWEEN_SEND_DATA))
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

    if (send_data == true || boot_sleep_count == 1)
    {
      HTTPClient client;

      //***** Inicializamos el sensor BME280 *****//
      if (bme.begin())
      { // si falla la comunicacion con el sensor mostrar

        Serial.println("BME280 encontrado!\n"); // texto y detener flujo del programa

        //***** Leemos los datos del sensor BME280*****//
        temp = bme.readTemperature();       // Almacena en variable el valor de temperatura
        presion = bme.readPressure() / 100; // Almacena en variable el valor de presion divido por 100 para covertirlo a hectopascales
        humedity = bme.readHumidity();      // Almacena en variable el valor de humedad
      }
      else
      {
        Serial.println("BME280 no encontrado!\n"); // texto y de tener flujo del programa

        // while (1);          // mediante bucle infinito
      }

      //***** Durante este bucle se toman cinco medidas de la velocidad del viento, cada una de seis mil milisegundos *****//
      for (int i = 0; i < 5; i++)
      {
        rev_anemometer = 0;
        delay(6000);

        wind_velocity[i] = (float)rev_anemometer * 0.4; // Se almacena en el array cada medida en km/h, habiendose determinado que las revoluciones contadas en los seis
        // segundos multiplicadas por 0,4 nos da la velocidad en km/h, siendo una rev/s equivalente a 2,4 km/h

        Serial.println("Velocidad del viento: " + String(wind_velocity[i]) + " km/s");
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
      s_GET = "?temperature=" + s_Temp + "&humidity=" + s_Humedity + "&presion=" + s_Presion + "&rain=" + s_liters_m2 + "&wind_direction=" + wind_direction + "&avg_wind=" + s_wind_avg + "&max_wind=" + s_wind_max + "&min_wind=" + s_wind_min + "&voltaje=" + s_voltaje;

      //***** Conectamos a google script y enviamos los datos mediante GET, a una app que los almacena en google sheets *****//
      Serial.println("\nStarting connection to google scripts ...");

      if (client.begin(serverURL))
      {
        // client.addHeader("Content-Type", "application/json");

        int httpCode = client.POST(s_GET);

        if (httpCode > 0)
        {
          Serial.println("\nStatuscode: " + String(httpCode));

          client.end();
        }
        else
        {
          Serial.println("Error on HTTP request to script.google.com");
        }
      }
      else
      {
        Serial.println("Error conection to script.google.com");
      }

      //***** Conectamos a mimeteoweb API y enviamos los datos mediante POST, es una API que luego los sirve mediante un servicio web *****//
      Serial.println("\nStarting connection to mimetoweb ...");

      if (client.begin(mimeteowebURL))
      {
        client.addHeader("Content-Type", "application/json");

        String jsonOutput;
        const size_t CAPACITY = JSON_OBJECT_SIZE(13);
        StaticJsonDocument<CAPACITY> doc;

        JsonObject object = doc.to<JsonObject>();
        object["location"] = "iznajar";
        object["date"] = getdate();
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

        // Serial.println(jsonOutput);

        int httpCode = client.POST(jsonOutput);

        if (httpCode > 0)
        {
          Serial.println("\nStatuscode: " + String(httpCode));

          client.end();
        }
        else
        {
          Serial.println("Error on HTTP request to mimeteoweb");
        }
      }
      else
      {
        Serial.println("Error conection to mimeteoweb");
      }

      delay(1000);

      //***** Imprimimos los valores de la estación en la consola *****//
      printDataSerial();

      //***** Resetea la lluvia caida *****//
      liters_m2 = 0;

      //***** Establecemos el ultimo tiempo que se envio los datos *****//
      last_minute = current_minute;

      //***** ESP32 a dormir si procede ******//

      current_t_sleep = millis();

      Serial.println("Last minute: " + String(last_minute));

      if ((wakeup_reason != ESP_SLEEP_WAKEUP_EXT0) || (current_t_sleep - last_t_sleep > TIME_RAIN * mS_a_S_FACTOR))
      {
        Serial.println("ESP32 to sleep!");

        esp_deep_sleep_start();
      }
    }
  }
  else
  {
    delay(1000);

    Serial.println("ESP32 to sleep!");

    esp_deep_sleep_start();
  }
}

//***** Conecta al ESP32 si a internet mediante wifi *****//
void connectedWiFi()
{
  int wifi_count = 0;

  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // attempt to connect to Wifi network:
  while (WiFi.status() != WL_CONNECTED && wifi_count < 20)
  {
    Serial.print(".");
    // wait 1 second for re-trying
    delay(1000);

    wifi_count++;
  }

  if (WiFi.status() == WL_CONNECTED)
  {

    Serial.print("Connected to ");
    Serial.println(ssid);

    // client.setCACert(root_ca);
    // client.setCACert(root_ca2);
  }
  else
  {

    Serial.println("\nNo connected!!\n");

    delay(1000);

    Serial.println("ESP32 to sleep!");

    esp_deep_sleep_start();
  }
}

//***** Muestra los datos de los sensores en el monitor serie *****//
void printDataSerial()
{

  Serial.println("-------------------------------");

  Serial.print("Temperatura: "); // muestra texto
  Serial.print(temp, 1);         // muestra valor de la variable
  Serial.print(" C ");           // muestra letra C indicando grados centigrados

  Serial.print("Humedad: "); // muestra texto
  Serial.print(humedity, 0); // muestra valor de la variable
  Serial.print(" % ");       // muestra texto % indicando porcentje

  Serial.print("Presion: "); // muestra texto
  Serial.print(presion, 0);  // muestra valor de la variable
  Serial.println(" hPa\n");  // muestra texto hPa indicando hectopascales

  Serial.println("Litros/m2: " + String(liters_m2)); // Muestra la lluvia caida

  // Muestra la direccion, velocidad maxima, minima y media del viento
  Serial.println("Direccion viento: " + wind_direction + "\n");
  Serial.println("Vel. max: " + String(max_velocity()) + " km/s");
  Serial.println("Vel. min: " + String(min_velocity()) + " km/s");
  Serial.println("Vel. media: " + String(avg_velocity()) + " km/s");

  // Muestra la direccion, velocidad maxima, minima y media del viento
  Serial.println("Voltaje bateria: " + s_voltaje + "V");
}

//***** Interrupcion que suma la precipitacion acumulada cada vez que se activa el pluviometro *****//
void sumLiters_m2()
{

  current_t_liters = millis();

  if ((current_t_liters - last_t_liters) > measurements_t_liters)
  {
    liters_m2 += 0.45;

    last_t_liters = current_t_liters;
  }
}

// Interrupcion que aumenta en uno las revoluciones del anemometro
void wind_velocity_interrupt()
{

  rev_anemometer++;
}

// Funciones para calcular la velocidad maxima, minima y media del viento
float max_velocity()
{

  float _max = 0;

  for (int i = 0; i < 5; i++)
  {

    if (wind_velocity[i] > _max)
    {

      _max = wind_velocity[i];
    }
  }

  return _max;
}

float min_velocity()
{

  float _min = wind_velocity[0];

  for (int i = 1; i < 5; i++)
  {

    if (wind_velocity[i] < _min)
    {

      _min = wind_velocity[i];
    }
  }

  return _min;
}

float avg_velocity()
{

  float _sum = 0;

  for (int i = 1; i < 5; i++)
  {

    _sum += wind_velocity[i];
  }

  return _sum / 5;
}

// Esta funcion nos determinara la direccion aproximada del viento
String read_wind_direction()
{

  int wind_direction_analog[20];
  int wind_direction_count[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  int wind_dir_count_max = 0;

  int element_array_wdc;
  // wind_direction_avg = 0;

  for (int i = 0; i < 20; i++)
  {

    wind_direction_analog[i] = analogRead(WIND_DIRECTION_SENSOR);

    delay(50);
  }

  // wind_direction_avg = (int)(wind_direction_analog / 10);

  // Serial.println("Lectura sensor de direccion del viento: " + String(wind_direction_avg));

  // Algoritmo que cuenta las veces que se ha dado cada una de las direcciones
  for (int i = 0; i < 20; i++)
  {
    for (int j = 0; j < 17; j++)
    {

      if (wind_direction_analog[i] <= sensor_wind_max[j] && wind_direction_analog[i] >= sensor_wind_min[j])
      {

        wind_direction_count[j]++;

        // wind_dir = wind_dir_comp[i];

        break;
      }
      else
      {

        wind_direction_count[j] = 16;

        // wind_dir = wind_dir_comp[16];
      }
    }
  }

  // Se determina la direccion que mas veces ha ocurrido y se devuelve
  for (int i = 0; i < 17; i++)
  {

    if (wind_direction_count[i] > wind_dir_count_max)
    {

      wind_dir_count_max = wind_direction_count[i];

      element_array_wdc = i;
    }
  }

  return wind_dir_comp[element_array_wdc];
}

//***** Lee el voltaje de la bateria *****//
float read_voltaje()
{

  int voltaje = analogRead(VOLTAJE_BATTERY);

  Serial.println(voltaje);

  voltaje = map(voltaje, 0, 3440, 0, 420);

  voltaje_bat = (float)voltaje / 100;

  Serial.println(voltaje_bat);

  return voltaje_bat;
}

//***** Funciones que obtienen la hora y los minutos actuales *****//
int getMinutes()
{
  struct tm timeinfo;

  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    return -1;
  }
  else
  {
    char char_current_minute[3];
    strftime(char_current_minute, 3, "%M", &timeinfo);
    Serial.println((String)char_current_minute);

    return atoi(char_current_minute);
  }
}

String getdate()
{
  struct tm timeinfo;

  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    return "-1";
  }
  else
  {
    char char_current_date[20];
    strftime(char_current_date, 20, "%Y-%m-%dT%H:%M:%S", &timeinfo);
    Serial.println((String)char_current_date);

    return (String)char_current_date;
  }
}

// int getHour() {
//   struct tm timeinfo;
//
//   if (!getLocalTime(&timeinfo)) {
//     Serial.println("Failed to obtain time");
//     return -1;
//   }
//   else {
//     char current_hour[3];
//     strftime(current_hour, 3, "%H", &timeinfo);
//     Serial.println(current_hour);
//     return atoi(current_hour);
//   }
// }