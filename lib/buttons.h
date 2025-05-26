#ifndef BUTTONS_H
#define BUTTONS_H

#define BOTAO_A 5 
#define BOTAO_B 6
#define BOTAO_C 18
#define BOTAO_J 22

#include "pico/stdlib.h"

void iniciar_botoes()
{
    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);

    gpio_init(BOTAO_B);
    gpio_set_dir(BOTAO_B, GPIO_IN);
    gpio_pull_up(BOTAO_B);

    gpio_init(BOTAO_C);
    gpio_set_dir(BOTAO_C, GPIO_IN);
    gpio_pull_up(BOTAO_C);

    gpio_init(BOTAO_J);
    gpio_set_dir(BOTAO_J, GPIO_IN);
    gpio_pull_up(BOTAO_J);
}

#endif