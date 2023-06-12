#ifndef _SENSOR_FUNCTIONS_H

#define _SENSOR_FUNCTIONS_H

//***** Incluimos la librerias necesarias *****//
#include <Arduino.h>

#include "config.h"

#define DEBUG
#include "debug_utils.h"

//***** Interrupcion que suma la precipitacion acumulada cada vez que se activa el pluviometro *****//
void IRAM_ATTR sumLiters_m2()
{

    current_t_liters = millis();

    if ((current_t_liters - last_t_liters) > measurements_t_liters)
    {
        liters_m2 += u_pluviometro;

        last_t_liters = current_t_liters;
    }
}

// Interrupcion que aumenta en uno las revoluciones del anemometro
void IRAM_ATTR wind_velocity_interrupt()
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

    // DEBUG_PRINT("Lectura sensor de direccion del viento: " + String(wind_direction_avg));

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

    DEBUG_PRINT(voltaje);

    voltaje = map(voltaje, 0, 3440, 0, 420);

    voltaje_bat = (float)voltaje / 100;

    DEBUG_PRINT(voltaje_bat);

    return voltaje_bat;
}

//***** Muestra los datos de los sensores en el monitor serie *****//
void printDataSerial()
{
    DEBUG_PRINT("-------------------------------");

    DEBUG_PRINT("Temperatura: "); // muestra texto
    DEBUG_PRINT(temp);            // muestra valor de la variable
    DEBUG_PRINT(" C ");           // muestra letra C indicando grados centigrados

    DEBUG_PRINT("Humedad: "); // muestra texto
    DEBUG_PRINT(humedity);    // muestra valor de la variable
    DEBUG_PRINT(" % ");       // muestra texto % indicando porcentje

    DEBUG_PRINT("Presion: "); // muestra texto
    DEBUG_PRINT(presion);     // muestra valor de la variable
    DEBUG_PRINT(" hPa\n");    // muestra texto hPa indicando hectopascales

    DEBUG_PRINT("Litros/m2: " + String(liters_m2)); // Muestra la lluvia caida

    // Muestra la direccion, velocidad maxima, minima y media del viento
    DEBUG_PRINT("Direccion viento: " + wind_direction + "\n");
    DEBUG_PRINT("Vel. max: " + String(max_velocity()) + " km/s");
    DEBUG_PRINT("Vel. min: " + String(min_velocity()) + " km/s");
    DEBUG_PRINT("Vel. media: " + String(avg_velocity()) + " km/s");

    // Muestra la direccion, velocidad maxima, minima y media del viento
    DEBUG_PRINT("Voltaje bateria: " + s_voltaje + "V");
}

#endif //_SENSOR_FUNCTIONS_H