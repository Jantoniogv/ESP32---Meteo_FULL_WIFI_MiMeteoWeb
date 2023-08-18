#ifndef _CONFIG_H

#define _CONFIG_H

#include <Arduino.h>

#include <WiFi.h>

// Pluviometro pin
#define RAIN_SENSOR 25

// Definimos el pin 35 como pin de obtencion de datos de la veleta
#define WIND_DIRECTION_SENSOR 35

// Definimos el pin 34 como pin de obtencion de datos del anemometro
#define ANEMOMETER_SENSOR 34

// Definimos el pin 32 como medidor de voltaje
#define VOLTAJE_BATTERY 32

//***** Establecemos los factores de tiempo *****//
#define MINUTES_BETWEEN_SEND_DATA 10 /* Tiempo entre cada envio de datos */

#define uS_a_S_FACTOR 1000000 /* 1000000 microsegundos / segundo */

#define S_a_M_FACTOR 60 /* 60 segundos / minuto para tener segundos */

#define TIME_SLEEP 10 /* Tiempo que duerme en minutos */

#define TIME_SERVER 2 /* Tiempo que esta despierto esperando conexion el servidor web en segundos */

// #define mS_a_S_FACTOR 1000 /* x 1000 para tener segundos */

// #define PIN_WAKE_UP_SERVER 0x8000000 // Mascara de bit en hexadecimal obtenida con 2^GPIO_NUMBER, en este caso el pin 27, 2^27

#define N_SAMPLES_WIND_DIRECTION 20 // Numero de muestras que toma para medir la direccion del viento

#define N_COMP_WIND_DIRECTION 17 // Numero de componentes posibles de direccion del viento

#define N_SAMPLES_VOL 10 // Numero de muestras que toma para medir el voltaje

bool keep_awake = false; // Estado que almacena si debe mantener despierto al ESP32 o no

//***** Constantes y objetos necesarios para realizar la conexion WiFi *****//
wifi_mode_t initWifiType = WIFI_MODE_APSTA;

// const char *initSsidSTA = "Router_Casa_Juncares"; // SSID conexion WiFi STA
// const char *initPassSTA = "Red_Juncares";         // Contraseña conexion WiFi STA

const char *initSsidSTA = "Router_Casa_Juncares_2.4G"; // SSID conexion WiFi STA
const char *initPassSTA = "Red_Juncares";              // Contraseña conexion WiFi STA

const char *initSsidAP = "MyESP32AP_2022"; // SSID conexion WiFi AP
const char *initPassAP = "adminadmin";     // Contraseña conexion WiFi AP

IPAddress init_IP_ap = IPAddress(192, 168, 10, 1);
IPAddress subnet = IPAddress(255, 255, 255, 0);

//***** Constantes que definen los server y url a los que se conecta el ESP32 y envia los datos *****//
// const String serverURL = "https://script.google.com/macros/s/AKfycbyf-0jMuXnlqXoohD-7WVzVF8J8rs5QlrM02nCP6geWslt-PAVYDesZdD2zqWCUN54/exec"; // URL

const String mimeteowebURL = "https://mimeteoweb.up.railway.app/api/v1/add-meteo-dates";

//***** Constantes y objetos necesarios para realizar la conexion NTP y obtener la hora *****//
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 0;

//***** Variables para almacenar los datos de las mediciones del BME280 *****//
float temp = 0;     // variable para almacenar valor de temperatura
float presion = 0;  // variable para almacenar valor de presion atmosferica
float humedity = 0; // variable para almacenar valor de presion atmosferica

String s_Temp = "--";
String s_Presion = "--";
String s_Humedity = "--";

//***** Variable en las que se almacenara los valores medidos para la lluvia *****//
RTC_DATA_ATTR float liters_m2 = 0; // Esta variable estara almacenada en la memoria del reloj RTC a fin de que se guarde su valor durante los periodos de sueño

const float u_pluviometro = 0.45; // Unidad medida del pluviometro

String s_liters_m2 = "--"; // String donde se guardara la lluvia medida para enviar los datos al servidor

unsigned long last_t_liters = 0;    // Variable que almacena el ultimo tiempo en ms cuado se registro un pulso del pluviometro
unsigned long current_t_liters = 0; // Variable que almacena el tiempo actual en ms del pulso del pluviometro

int measurements_t_liters = 500; // Tiempo en ms, en los que se desprecia los pulsos registrado por el pluviometro, a fin de desprecia los rebotes

//***** Variables del anemometro *****//
volatile int rev_anemometer = 0; // Esta variable la encargada de almacenar las rpm  que se leen desde el anemometro

float wind_velocity[5]; // Este array almacena cinco medidas del anemometro a fin de realizar una media de entre ellos

String s_wind_max = "--";
String s_wind_min = "--";
String s_wind_avg = "--";

//***** Variables de la veleta *****//

// Arrays que identifican las distintas direcciones del viento, y definen el valor maximo y minimo que toma la lectura analogica del sensor
String wind_dir_comp[] = {"NORTE", "NORTE-NOROESTE", "NOROESTE", "ESTE-NOROESTE", "ESTE", "ESTE-SURESTE", "SURESTE", "SUR-SURESTE",
                          "SUR", "SUR-SUROESTE", "SUROESTE", "OESTE-SUROESTE", "OESTE", "OESTE-NOROESTE", "NOROESTE", "NORTE-NOROESTE", "UNDEFINED"};

int sensor_wind_min[] = {3030, 1460, 1690, 150, 210, 80, 550, 320, 980, 800, 2380, 2250, 3960, 3230, 3560, 2670};
int sensor_wind_max[] = {3110, 1540, 1770, 205, 240, 140, 630, 400, 1060, 880, 2440, 2330, 4040, 3310, 3640, 2750};

// Estas variables sirven para almacenar la direccion del viento, el valor leido del sensor y la media del valor despues de varias medidas
String wind_direction = "--";

// Constantes de medicion de voltaje
const int voltaje_analog = 3410;
const int voltaje_max_x_100 = 420;

// Almacena la lectura del voltaje
float voltaje_bat_min = 3.0;
float voltaje_bat = 0;
String s_voltaje = "--";

// int wind_direction_avg = 0;

//***** Esta variable almacena en formato String los datos a subir al servidor *****//
String s_GET = "--";

//***** Estas variales se encargan de almacenar la ultima vez que se envio datos y el minuto actual *****//
RTC_DATA_ATTR int last_minute = 0;
int current_minute = 0;

//***** Variable para contar las veces que el ESP32 ha enviado datos *****//
RTC_DATA_ATTR int send_data_count = 0;

//***** Esta variable se encarga de establecer cuando se envia datos 'true' y cuando no 'false' ******//
RTC_DATA_ATTR bool first_start = true;

//***** Esta variable se encarga de establecer cuando se envia datos 'true' y cuando no 'false' ******//
bool send_data = false;

//***** Creamos el objeto del sensor BME280 *****//
Adafruit_BME280 bme;

//***** Esta variable almacena la razon por la que el ESP32 ha despertado *****//
esp_sleep_wakeup_cause_t wakeup_reason;

//***** Estas variables se encargan de establecer el tiempo que el ESP32 lleva despierto *****//
// unsigned long last_t_sleep = 0;
// unsigned long current_t_sleep = 0;

#endif //_CONFIG_H