#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "lib/display.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "lib/buttons.h"
#include "lib/leds_rgb.h"
#include "lib/buzzer.h"
#include <stdio.h>
#include <string.h>

#define MAX_USUARIOS 8 // Número máximo de usuários permitidos

SemaphoreHandle_t xSemaContagem;
SemaphoreHandle_t xSemaReset;
SemaphoreHandle_t xMutexDisplay;

uint8_t usuarios = 0;

// Função para atualizar o display com mutex
void atualizar_display(const char* msg1, const char* msg2) {
    xSemaphoreTake(xMutexDisplay, portMAX_DELAY);
    ssd1306_fill(&ssd, 0);
    ssd1306_draw_string(&ssd, msg1, 5, 10);
    ssd1306_draw_string(&ssd, msg2, 5, 45);
    ssd1306_send_data(&ssd);
    xSemaphoreGive(xMutexDisplay);
}

// Função para atualizar o LED RGB conforme o estado
void atualizar_led() 
{
    if (usuarios == 0) {
        leds(0, 0, 1); // Azul
    } else if (usuarios < MAX_USUARIOS-1) {
        leds(0, 1, 0); // Verde
    } else if (usuarios == MAX_USUARIOS-1) {
        leds(1, 1, 0); // Amarelo
    } else if (usuarios >= MAX_USUARIOS) {
        leds(1, 0, 0); // Vermelho
    }
}

// Tarefa de entrada
void vTaskEntrada(void *params) 
{
    char buffer[32];
    while (1) {
        if (gpio_get(BOTAO_A) == 0) {
            vTaskDelay(pdMS_TO_TICKS(30)); // debounce
            if (gpio_get(BOTAO_A) == 0) {
                if (usuarios < MAX_USUARIOS) {
                    usuarios++;
                    atualizar_led();
                    sprintf(buffer, "Usuarios: %d", usuarios);
                    atualizar_display("Entrada OK", buffer);
                    ssd1306_draw_string(&ssd, buffer, 5, 45);
                } else {
                    atualizar_led();
                    atualizar_display("Lotado!", "Aguarde...");
                    beep_curto();
                }
                while (gpio_get(BOTAO_A) == 0) vTaskDelay(pdMS_TO_TICKS(10));
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// Tarefa de saída
void vTaskSaida(void *params) 
{
    char buffer[32];
    while (1) {
        if (gpio_get(BOTAO_B) == 0) {
            vTaskDelay(pdMS_TO_TICKS(30)); // debounce
            if (gpio_get(BOTAO_B) == 0) {
                if (usuarios > 0) {
                    usuarios--;
                    atualizar_led();
                    sprintf(buffer, "Usuarios: %d", usuarios);
                    atualizar_display("Saida OK", buffer);
                } else {
                    atualizar_led();
                    atualizar_display("Nenhum usuario", "para sair");
                }
                while (gpio_get(BOTAO_B) == 0) vTaskDelay(pdMS_TO_TICKS(10));
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// Tarefa de reset
void vTaskReset(void *params) 
{
    char buffer[32];
    while (1) {
        if (xSemaphoreTake(xSemaReset, portMAX_DELAY) == pdTRUE) {
            usuarios = 0;
            atualizar_led();
            sprintf(buffer, "Usuarios: %d", usuarios);
            atualizar_display("Sist. Resetado", buffer);
            beep_duplo();
        }
    }
}

// Interrupção do botão de reset (joystick)
void gpio_reset_isr(uint gpio, uint32_t events) 
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (gpio == BOTAO_J) {
        xSemaphoreGiveFromISR(xSemaReset, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

int main() 
{
    sleep_ms(1000); // Atraso para estabilização
    stdio_init_all();
    display_init();
    iniciar_leds_rgb();
    iniciar_buzzer();
    iniciar_botoes();

    xSemaContagem = xSemaphoreCreateCounting(MAX_USUARIOS, 0); // não usado diretamente, mas pode ser útil para expansão
    xSemaReset = xSemaphoreCreateBinary();
    xMutexDisplay = xSemaphoreCreateMutex();

    atualizar_led();
    atualizar_display("Aguardando", "usuarios...");

    // Configura interrupção para o botão de reset (joystick)
    gpio_set_irq_enabled_with_callback(BOTAO_J, GPIO_IRQ_EDGE_FALL, true, &gpio_reset_isr);

    xTaskCreate(vTaskEntrada, "Entrada", configMINIMAL_STACK_SIZE+128, NULL, 2, NULL);
    xTaskCreate(vTaskSaida, "Saida", configMINIMAL_STACK_SIZE+128, NULL, 2, NULL);
    xTaskCreate(vTaskReset, "Reset", configMINIMAL_STACK_SIZE+128, NULL, 3, NULL);

    vTaskStartScheduler();
}