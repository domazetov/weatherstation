#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mosquitto.h>
#include <sensors.h>

void receive_dht(char * topic, char * payload)
{
    printf("DHT: ");
    if(strstr(topic, "humi") != NULL)
    {
        printf("Humidity: %.2f\n", str2float(payload));
    }
    else if(strstr(topic, "temp") != NULL)
    {
        printf("Temperature: %.2f\n", str2float(payload));
    }
}

void receive_bmp(char * topic, char * payload)
{
    printf("BMP: ");
    if(strstr(topic, "temp") != NULL)
    {
        printf("Temperature: %.2f\n", str2float(payload));
    }
    else if(strstr(topic, "pres") != NULL)
    {
        printf("Pressure: %.2f\n", str2float(payload));
        printf("#########################\n");
    }
}

float str2float(char * payload)
{
    union floatAsBytes{
        float value;
        unsigned char bytes[4];
    }data;

    const unsigned char *pos = payload;
    for(int i = 0; i < 4; i++)
    {
        sscanf(pos+6-2*i, "%2hhx", &data.bytes[i]);
        //printf("%02X ", data.bytes[i]);
    }

    return data.value;
}