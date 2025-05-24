#ifndef DISPLAY_H
#define DISPLAY_H

#include "lib/ssd1306.h"

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define ENDERECO 0x3C

ssd1306_t ssd;

void display_init()
{
    i2c_init(I2C_PORT, 400 * 1000); // Inicializa o I2C a 400kHz
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Configura o pino SDA
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Configura o pino SCL
    gpio_pull_up(I2C_SDA); // Habilita pull-up no SDA
    gpio_pull_up(I2C_SCL); // Habilita pull-up no SCL

    ssd.i2c_port = I2C_PORT;
    ssd.address = ENDERECO;
    ssd.width = 128;
    ssd.height = 64;
    ssd.pages = 8;
    ssd.external_vcc = false;
    ssd.bufsize = (ssd.width * ssd.pages);
    ssd.ram_buffer = malloc(ssd.bufsize);

    ssd1306_init(&ssd, 128, 64, false, ENDERECO, I2C_PORT); // Inicializa o driver SSD1306
    ssd1306_config(&ssd); // Configura o display
    ssd1306_fill(&ssd, 0); // Garante tela limpa
    ssd1306_send_data(&ssd); // Atualiza o display com tela limpa
}

#endif