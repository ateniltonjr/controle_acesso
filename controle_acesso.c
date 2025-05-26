#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <stdio.h>
#include <string.h>
#include "lib/display.h"
#include "lib/buttons.h"
#include "lib/leds_rgb.h"
#include "lib/buzzer.h"

#define MAX_USUARIOS 8 // Número máximo de usuários permitidos
#define HISTORICO_TAM 5

uint8_t usuarios = 0; // Contador de usuários

SemaphoreHandle_t xSemaContagem; // Semáforo para contagem de usuários
SemaphoreHandle_t xSemaReset; // Semáforo para reset do sistema
SemaphoreHandle_t xMutexDisplay; // Mutex para acesso ao display

// Estrutura para histórico de eventos
typedef struct 
{
    char evento[20];
    uint8_t usuarios;
} HistoricoEvento;

HistoricoEvento historico[HISTORICO_TAM]; // Array para armazenar eventos históricos
uint8_t historico_idx = 0; // Índice do histórico

// Função para atualizar o display com mutex
void atualizar_display(const char* msg1, const char* msg2) 
{
    xSemaphoreTake(xMutexDisplay, portMAX_DELAY); // Aguarda o mutex
    ssd1306_fill(&ssd, 0);
    ssd1306_draw_string(&ssd, msg1, 5, 10);
    ssd1306_draw_string(&ssd, msg2, 5, 45);
    ssd1306_send_data(&ssd);
    xSemaphoreGive(xMutexDisplay); // Libera o mutex
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

// Adiciona um evento ao histórico
void adicionar_historico(const char* evento, uint8_t usuarios) {
    snprintf(historico[historico_idx].evento, sizeof(historico[historico_idx].evento), "%s", evento); // Copia o nome do evento
    historico[historico_idx].usuarios = usuarios; // Armazena o número de usuários no evento
    historico_idx = (historico_idx + 1) % HISTORICO_TAM; // Circular buffer
    printf("Evento: %s | Usuarios: %d\n", evento, usuarios); // Log para depuração
}

// Exibe o histórico de eventos no display
void exibir_historico() {
    xSemaphoreTake(xMutexDisplay, portMAX_DELAY);
    ssd1306_fill(&ssd, 0);
    int y = 0;
    printf("--- Historico de eventos ---\n");
    for (int i = 0; i < HISTORICO_TAM; i++) {
        uint8_t idx = (historico_idx + i) % HISTORICO_TAM;
        char linha[32];
        snprintf(linha, sizeof(linha), "%s: %d", historico[idx].evento, historico[idx].usuarios);
        ssd1306_draw_string(&ssd, linha, 0, y);
        printf("%s\n", linha);
        y += 12;
    }
    ssd1306_send_data(&ssd);
    xSemaphoreGive(xMutexDisplay);
}

// Tarefa de entrada
void vTaskEntrada(void *params) 
{
    char buffer[32]; // Buffer para mensagens
    while (1) {
        if (gpio_get(BOTAO_A) == 0) {
            vTaskDelay(pdMS_TO_TICKS(300)); // debounce
            if (gpio_get(BOTAO_A) == 0) {
                if (usuarios < MAX_USUARIOS) {
                    usuarios++;
                    atualizar_led();
                    sprintf(buffer, "Usuarios: %d", usuarios);
                    atualizar_display("Entrada OK", buffer);
                    adicionar_historico("Entrada", usuarios);
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
            vTaskDelay(pdMS_TO_TICKS(300)); // debounce
            if (gpio_get(BOTAO_B) == 0) {
                if (usuarios > 0) {
                    usuarios--;
                    atualizar_led();
                    sprintf(buffer, "Usuarios: %d", usuarios);
                    atualizar_display("Saida OK", buffer);
                    adicionar_historico("Saida", usuarios);
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
        if (xSemaphoreTake(xSemaReset, portMAX_DELAY) == pdTRUE) { // Aguarda o semáforo de reset
            usuarios = 0;
            atualizar_led();
            sprintf(buffer, "Usuarios: %d", usuarios);
            atualizar_display("Sist. Resetado", buffer);
            beep_duplo();
            adicionar_historico("Reset", usuarios);
        }
    }
}

// Interrupção do botão de reset (joystick)
void gpio_reset_isr(uint gpio, uint32_t events) 
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE; // Variável para verificar se uma tarefa de maior prioridade foi despertada
    if (gpio == BOTAO_J) {
        xSemaphoreGiveFromISR(xSemaReset, &xHigherPriorityTaskWoken); // Libera o semáforo de reset
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken); // Solicita troca de contexto se necessário
    }
}

// Botão C para exibir histórico
void vTaskHistorico(void *params) {
    while (1) {
        if (gpio_get(BOTAO_C) == 0) {
            vTaskDelay(pdMS_TO_TICKS(30));
            if (gpio_get(BOTAO_C) == 0) {
                exibir_historico();
                while (gpio_get(BOTAO_C) == 0) vTaskDelay(pdMS_TO_TICKS(10));
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

int main() 
{
    sleep_ms(1000); // Atraso para estabilização
    stdio_init_all(); // Inicializa a saída padrão
    display_init(); // Inicializa o display
    iniciar_leds_rgb(); // Inicializa os LEDs RGB
    iniciar_buzzer();// Inicializa o buzzer
    iniciar_botoes();// Inicializa os botões

    xSemaContagem = xSemaphoreCreateCounting(MAX_USUARIOS, 0); // não usado diretamente, mas pode ser útil para expansão
    xSemaReset = xSemaphoreCreateBinary(); // Semáforo para reset do sistema
    xMutexDisplay = xSemaphoreCreateMutex(); // Mutex para acesso ao display

    atualizar_led(); // Atualiza o estado dos LEDs
    atualizar_display("Aguardando", "usuarios...");

    // Configura interrupção para o botão de reset (joystick)
    gpio_set_irq_enabled_with_callback(BOTAO_J, GPIO_IRQ_EDGE_FALL, true, &gpio_reset_isr);

    // Criação das tarefas
    xTaskCreate(vTaskEntrada, "Entrada", configMINIMAL_STACK_SIZE+128, NULL, 2, NULL); // Tarefa de entrada
    xTaskCreate(vTaskSaida, "Saida", configMINIMAL_STACK_SIZE+128, NULL, 2, NULL); // Tarefa de saída
    xTaskCreate(vTaskReset, "Reset", configMINIMAL_STACK_SIZE+128, NULL, 3, NULL);// Tarefa de reset
    xTaskCreate(vTaskHistorico, "Historico", configMINIMAL_STACK_SIZE+128, NULL, 1, NULL); // Tarefa de histórico

    vTaskStartScheduler(); // Inicia o escalonador do FreeRTOS
}