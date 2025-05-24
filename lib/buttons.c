#include "buttons.h"
#include "pico/stdlib.h"

// Aguarda pressionamento de um botão (retorna 1 se pressionado, 0 caso contrário)
int buttons_wait_press(int botao) {
    static uint32_t debounce_time = 0;
    if (gpio_get(botao) == 0) { // Pressionado (pull-up)
        sleep_ms(10); // debounce
        if (gpio_get(botao) == 0 && (to_ms_since_boot(get_absolute_time()) - debounce_time > 200)) {
            debounce_time = to_ms_since_boot(get_absolute_time());
            while (gpio_get(botao) == 0) { tight_loop_contents(); } // espera soltar
            return 1;
        }
    }
    return 0;
}
