#ifndef LEDS_RGB_H
#define LEDS_RGB_H

#include <stdlib.h>
#include "pico/stdlib.h"
#include <stdbool.h>

#define red 13
#define green 11
#define blue 12

void iniciar_leds_rgb()
{
    gpio_init(red);
    gpio_set_dir(red, GPIO_OUT);

    gpio_init(green);
    gpio_set_dir(green, GPIO_OUT);

    gpio_init(blue);
    gpio_set_dir(blue, GPIO_OUT);
}

void leds(bool r, bool g, bool b)
{
    gpio_put(red, r);
    gpio_put(green, g);
    gpio_put(blue, b);
}

#endif