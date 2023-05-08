#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"

#define DHT_PIN 2

#define I2C_PORT i2c0
#define I2C_SDA_PIN 4
#define I2C_SCL_PIN 5

#define SGP30_ADDR 0x58
#define SGP30_INIT_REG "\x20\x03"
#define SGP30_MEASURE_REG "\x20\x08"

struct dht {
    int temperature;
    int temperature_decimal;
    int humidity;
    int humidity_decimal;
};

struct sgp30 {
    uint16_t tvoc_ppb;
    uint16_t co2_ppm;
};

void get_dht_data(struct dht *dht)
{
    gpio_put(DHT_PIN, 0);
    sleep_ms(18);
    gpio_put(DHT_PIN, 1);
    sleep_us(40);
    gpio_set_dir(DHT_PIN, GPIO_IN);
    while(gpio_get(DHT_PIN));
    sleep_us(80);
    if(!gpio_get(DHT_PIN));
    sleep_us(80);
    for(int i = 0; i < 4; i++)
    {
        int temp = 0;
        for(int j = 7; j >= 0; j--)
        {
            if(!gpio_get(DHT_PIN))
            {
                while(!gpio_get(DHT_PIN));
                sleep_us(30);
                temp |= gpio_get(DHT_PIN) << j;
                while(gpio_get(DHT_PIN));
            }
        }

        switch(i)
        {
            case 0:
                dht->humidity = temp;
                break;
            case 1:
                dht->humidity_decimal = temp;
                break;
            case 2:
                dht->temperature = temp;
                break;
            case 3:
                dht->temperature_decimal = temp;
                break;
        }
    }

    gpio_set_dir(DHT_PIN, GPIO_OUT);
    gpio_put(DHT_PIN, 1);
}

void get_sgp30_data(struct sgp30 *sgp30)
{
    uint8_t sgp30_data[6];

    i2c_write_blocking(I2C_PORT, SGP30_ADDR, SGP30_MEASURE_REG, 2, true);
    sleep_ms(25);
    i2c_read_blocking(I2C_PORT, SGP30_ADDR, sgp30_data, 6, false);

    sgp30->co2_ppm = (uint16_t)sgp30_data[0] << 8;
    sgp30->co2_ppm |= (uint16_t)sgp30_data[1];

    sgp30->tvoc_ppb = (uint16_t)sgp30_data[3] << 8;
    sgp30->tvoc_ppb |= (uint16_t)sgp30_data[4];
}

void main()
{
    stdio_init_all();

    gpio_init(DHT_PIN);
    gpio_set_dir(DHT_PIN, GPIO_OUT);

    i2c_init(I2C_PORT, 400000);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    sleep_ms(1000);

    i2c_write_blocking(I2C_PORT, SGP30_ADDR, SGP30_INIT_REG, 2, false);

    sleep_ms(1000);

    struct dht dht;
    struct sgp30 sgp30;

    while(1)
    {
        get_dht_data(&dht);
        get_sgp30_data(&sgp30);

        printf("Humidity (%%): %d.%d\n", dht.humidity, dht.humidity_decimal);
        printf("Temperature (°C): %d.%d\n", dht.temperature, dht.temperature_decimal);
        printf("TVOC (ppb): %lu\n", sgp30.tvoc_ppb);
        printf("CO2  (ppm): %lu\n\n", sgp30.co2_ppm);
        sleep_ms(1000);
    }
}