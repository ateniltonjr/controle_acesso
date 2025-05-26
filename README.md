![Capa do Projeto](https://github.com/user-attachments/assets/f2a5c9b8-6208-4723-8f46-1d74be421827)
# Painel de Controle Interativo com Acesso Concorrente

Este projeto implementa um sistema embarcado de controle de acesso para ambientes como laboratórios, bibliotecas ou refeitórios, utilizando a placa BitDogLab com RP2040 e FreeRTOS.

## Funcionalidades

- **Controle de acesso concorrente:** Limita o número de usuários simultâneos no ambiente.
- **Feedback visual:** LED RGB indica o estado de ocupação (azul, verde, amarelo, vermelho).
- **Feedback sonoro:** Buzzer emite beep curto para acesso negado e beep duplo para reset.
- **Display OLED:** Exibe mensagens de status, contagem de usuários e histórico de eventos.
- **Histórico de eventos:** Mostra os últimos acessos, saídas e resets no display e no monitor serial.
- **Acessibilidade:** Sinalização sonora e visual para facilitar o uso por todos.

## Como funciona

- **Entrada de usuário:**
  - Botão A incrementa o número de usuários, se houver vaga.
  - Se o limite for atingido, exibe aviso e emite beep curto.
- **Saída de usuário:**
  - Botão B decrementa o número de usuários, se houver alguém presente.
- **Reset do sistema:**
  - Botão do joystick zera a contagem de usuários e emite beep duplo.
- **Exibição do histórico:**
  - Botão C mostra os últimos eventos no display e no monitor serial.

## Sincronização e Concorrência

- **Semáforo de contagem:** (xSemaContagem) Pronto para controlar o número de usuários simultâneos.
- **Semáforo binário:** (xSemaReset) Usado para resetar o sistema via interrupção.
- **Mutex:** (xMutexDisplay) Protege o acesso ao display OLED entre tarefas concorrentes.

## Periféricos Utilizados

- **Botões físicos:** Entrada, saída, reset e histórico.
- **LED RGB:** Indicação visual de ocupação.
- **Buzzer:** Sinalização sonora.
- **Display OLED SSD1306:** Exibição de mensagens e histórico.

## Estrutura do Código

- `controle_acesso.c`: Código principal do sistema.
- `lib/`: Biblioteca com drivers para display, botões, buzzer e LEDs.

## Como compilar e rodar

1. Instale o SDK do Raspberry Pi Pico e o FreeRTOS.
2. Clone este repositório e abra a pasta no VS Code.
3. Compile o projeto usando o comando ou task "Compile Project".
4. Grave o firmware na placa BitDogLab usando o task "Flash" ou "Run Project".
5. Conecte o monitor serial para acompanhar os eventos.

## Demonstração

- [Link para vídeo de demonstração](https://youtu.be/_3FajyQkMhY)

## Créditos

Desenvolvido por Atenilton Santos de Souza Júnior
Polo: Bom Jesus da Lapa
Data: 20/04
20/05/2025

---

Sinta-se à vontade para adaptar este projeto para outras aplicações de controle de acesso, segurança ou acessibilidade!
