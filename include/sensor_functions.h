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

    int wind_direction_analog[N_SAMPLES_WIND_DIRECTION];   // Almacena las lecturas analogicas de la veleta
    int wind_direction_count[N_COMP_WIND_DIRECTION] = {0}; // Almacena las veces que ocurre cada direccion en cada toma de muestras

    int wind_dir_count_max = 0; // Almacena el maximo de veces que ha ocurrido un direccion

    int element_array_wdc; // Almacena el elemento del array componetes de direccion que ha ocurrido mas veces en la medida

    for (int i = 0; i < N_SAMPLES_WIND_DIRECTION; i++)
    {

        wind_direction_analog[i] = analogRead(WIND_DIRECTION_SENSOR);

        delay(50);
    }

    // Algoritmo que cuenta las veces que se ha dado cada una de las direcciones
    for (int i = 0; i < N_SAMPLES_WIND_DIRECTION; i++)
    {
        for (int j = 0; j < N_COMP_WIND_DIRECTION; j++)
        {
            // Comprueba a que componente de la direccion del viento pertence la medida a medida que recorre el array,
            // si la encuentra, suma uno en el array suma de cada componente, y si llega a la ultima posicion sin coincidir
            // ningun componente real, lo suma al UNDEFINED
            if (wind_direction_analog[i] <= sensor_wind_max[j] && wind_direction_analog[i] >= sensor_wind_min[j])
            {
                wind_direction_count[j]++;
                break;
            }
            else if (j == N_COMP_WIND_DIRECTION - 1)
            {

                wind_direction_count[N_COMP_WIND_DIRECTION - 1]++;
            }
        }
    }

    // Se determina la direccion que mas veces ha ocurrido y se devuelve
    for (int i = 0; i < N_COMP_WIND_DIRECTION; i++)
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
    int voltaje = 0;

    for (int i = 0; i < N_SAMPLES_VOL; i++)
    {

        voltaje += analogRead(VOLTAJE_BATTERY);
    }

    int voltaje_medio = voltaje / N_SAMPLES_VOL;

    DEBUG_PRINT(voltaje_medio);

    voltaje_medio = map(voltaje_medio, 0, voltaje_analog, 0, voltaje_max_x_100);

    voltaje_bat = (float)voltaje_medio / 100;

    DEBUG_PRINT(voltaje_bat);

    return voltaje_bat;
}

//***** Muestra los datos de los sensores en el monitor serie *****//
void print_data_serial()
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
    DEBUG_PRINT("Vel. max: " + String(max_velocity()) + " km/h");
    DEBUG_PRINT("Vel. min: " + String(min_velocity()) + " km/h");
    DEBUG_PRINT("Vel. media: " + String(avg_velocity()) + " km/h");

    // Muestra la direccion, velocidad maxima, minima y media del viento
    DEBUG_PRINT("Voltaje bateria: " + s_voltaje + "V");
}

#endif //_SENSOR_FUNCTIONS_H