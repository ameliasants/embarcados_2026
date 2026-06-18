#include "soc_AM335x.h"
#include "hw_types.h"
#include "timer.h"
#include "uart.h"

/* ---------------- DEFINES ---------------- */
#define TEMPO_PISCA_3S   1000000   // 3 segundo em us
#define TEMPO_PISCA_5S   1000000   // 5 segundos em us
#define TEMPO_DEBOUNCE   10000     // 10 ms
/* ---------------- TIMER ---------------- */
#define TEMPO_PISCA               500000   // 0,5s em us
#define TEMPO_DEBOUNCE            10000
/* ---------------- DELAY ---------------- */
#define TEMPO_SEQUENCIA 10  // Equivalente a 2 segundos
#define TEMPO_DEBOUNCE  10000   // Equivalente a 10 milissegundos

/* ---------------- CLOCK ---------------- */

#define SOC_PRCM_REGS             (0x44E00000)

#define CM_PER_GPIO1_CLKCTRL      (0x0AC)
#define CM_PER_TIMER7_CLKCTRL_OFFSET (0x007C) /* Energia do Timer 7 */


/* ---------------- CONTROL MODULE ---------------- */

#define SOC_CONTROL_REGS          (0x44E10000)

/* LEDs internos */

#define CONF_GPMC_A5              (0x0854)      // USR0 - GPIO1_21
#define CONF_GPMC_A6              (0x0858)      // USR1 - GPIO1_22
#define CONF_GPMC_A7              (0x085C)      // USR2 - GPIO1_23
#define CONF_GPMC_A8              (0x0860)      // USR3 - GPIO1_24

/* LED externo */

#define CONF_GPMC_A1              (0x0844)      // GPIO1_17

/* Botões */

#define CONF_GPMC_BE1N            (0x0878)      // GPIO1_28
#define CONF_GPMC_A0              (0x0840)      // GPIO1_16
#define CONF_GPMC_AD12            (0x0830)      // GPIO1_12

/* ---------------- GPIO ---------------- */

#define SOC_GPIO_1_REGS           (0x4804C000)

#define GPIO_OE                   (0x134)
#define GPIO_DATAIN               (0x138)
#define GPIO_CLEARDATAOUT         (0x190)
#define GPIO_SETDATAOUT           (0x194)


/* ---------------- LEDs ---------------- */

#define LED_USR0                  (1 << 21)
#define LED_USR1                  (1 << 22)
#define LED_USR2                  (1 << 23)
#define LED_USR3                  (1 << 24)

#define LED_EXTERNO               (1 << 17)
#define LED_EXTERNO2 		  (1 << 12)


#define TODOS_LEDS                ( LED_USR0 | \
                                    LED_USR1 | \
                                    LED_USR2 | \
                                    LED_USR3 )


/* ---------------- BOTÕES ---------------- */

#define BOTAO1                    (1 << 28)
#define BOTAO2                    (1 << 16)
#define BOTAO3                    (1 << 12)

/*
---------------- FUNÇÃO DE ATRASO ---------------- 

void atraso(unsigned int t)
{
    while(t--)
    {

    }
}

*/

/* ---------------- DESABILITA WATCHDOG ---------------- */

void disable_watchdog(void)
{
    while(HWREG(0x44E35034) != 0)
    {

    }

    HWREG(0x44E35048) = 0xAAAA;

    while(HWREG(0x44E35034) != 0)
    {

    }

    HWREG(0x44E35048) = 0x5555;

    while(HWREG(0x44E35034) != 0)
    {

    }
}


/* ---------------- INICIALIZA GPIO E MUX ---------------- */

void iniciar()
{
    unsigned int dir;


    /* 1. HABILITA CLOCK DO GPIO1                       */

    HWREG(SOC_CM_PER_REGS + CM_PER_GPIO1_CLKCTRL) |= 0x2;
    /*     HABILITA O CLOCK DO TIMER7*/
    HWREG(SOC_CM_PER_REGS + CM_PER_TIMER7_CLKCTRL_OFFSET) = 0x02; 



    /* 2. CONFIGURA MUX DOS BOTÕES                      */

    HWREG(SOC_CONTROL_REGS + CONF_GPMC_BE1N) = 0x2F;

    HWREG(SOC_CONTROL_REGS + CONF_GPMC_A0) = 0x2F;

    HWREG(SOC_CONTROL_REGS + CONF_GPMC_AD12) = 0x2F;



    /* 3. CONFIGURA MUX DOS LEDs                        */

    HWREG(SOC_CONTROL_REGS + CONF_GPMC_A5) = 0x0F;

    HWREG(SOC_CONTROL_REGS + CONF_GPMC_A6) = 0x0F;

    HWREG(SOC_CONTROL_REGS + CONF_GPMC_A7) = 0x0F;

    HWREG(SOC_CONTROL_REGS + CONF_GPMC_A8) = 0x0F;

    HWREG(SOC_CONTROL_REGS + CONF_GPMC_A1) = 0x0F;


    /* 4. CONFIGURA DIREÇÃO DOS GPIOs                   */

    dir = HWREG(SOC_GPIO_1_REGS + GPIO_OE);


    /* Botões como entrada */

    dir |= BOTAO1;

    dir |= BOTAO2;

    dir |= BOTAO3;


    /* LEDs internos como saída */

    dir &= ~LED_USR0;

    dir &= ~LED_USR1;

    dir &= ~LED_USR2;

    dir &= ~LED_USR3;


    /* LED externo como saída */

    dir &= ~LED_EXTERNO;
    
    dir &= ~LED_EXTERNO2;


    /* Escreve configuração */

    HWREG(SOC_GPIO_1_REGS + GPIO_OE) = dir;



    /* 5. INICIA LEDs DESLIGADOS                        */


    HWREG(SOC_GPIO_1_REGS + GPIO_CLEARDATAOUT) =
            TODOS_LEDS | LED_EXTERNO;
}

/* ---------------- LIGA LED ---------------- */

void liga(unsigned int led)
{
    HWREG(SOC_GPIO_1_REGS + GPIO_SETDATAOUT) = led;
}


/* ---------------- DESLIGA LED ---------------- */

void desliga(unsigned int led)
{
    HWREG(SOC_GPIO_1_REGS + GPIO_CLEARDATAOUT) = led;
}


/* ---------------- LEITURA DOS BOTÕES ---------------- */

int pressionado(unsigned int botao)
{
    return (HWREG(SOC_GPIO_1_REGS + GPIO_DATAIN) & botao);
}

/* ---------------- APAGA TODOS LEDs INTERNOS ---------------- */

void apagaTudo()
{
    desliga(TODOS_LEDS);
}


/* ---------------- PADRÃO 1 ---------------- */
/* LEDs acendem um por um                    */

void sequencia1()
{
    unsigned int leds[] =
    {
        LED_USR0,
        LED_USR1,
        LED_USR2,
        LED_USR3
    };

    int i;

    for(i = 0; i < 4; i++)
    {
        /* Permite troca de padrão */

        if(pressionado(BOTAO1))
        {
            break;
        }

        /* Liga LED */

        liga(leds[i]);

        /* Espera */

        uDelay(TEMPO_SEQUENCIA);
        /* Desliga LED */

        desliga(leds[i]);
    }
}


/* ---------------- PADRÃO 2 ---------------- */
/* Todos os LEDs piscam                      */

void sequencia2()
{
    if(pressionado(BOTAO1))
    {
        return;
    }

    /* Liga todos */

    liga(TODOS_LEDS);

    uDelay(TEMPO_SEQUENCIA);

    /* Desliga todos */

    desliga(TODOS_LEDS);

    uDelay(TEMPO_SEQUENCIA);
}


/* ---------------- PADRÃO 3 ---------------- */
/* LEDs alternados                           */

void sequencia3()
{
    if(pressionado(BOTAO1))
    {
        return;
    }


    /* Liga LEDs 0 e 1 */

    liga(LED_USR0);

    liga(LED_USR1);


    /* Desliga LEDs 2 e 3 */

    desliga(LED_USR2);

    desliga(LED_USR3);


    uDelay(TEMPO_SEQUENCIA);


    /* Verifica troca de padrão */

    if(pressionado(BOTAO1))
    {
        apagaTudo();

        return;
    }


    /* Desliga LEDs 0 e 1 */

    desliga(LED_USR0);

    desliga(LED_USR1);


    /* Liga LEDs 2 e 3 */

    liga(LED_USR2);

    liga(LED_USR3);


    uDelay(TEMPO_SEQUENCIA);


    /* Apaga todos */

    apagaTudo();
}


/* ---------------- FUNÇÃO PRINCIPAL ---------------- */
int _main(void)
{
    int led_estado = 0;
    int led2_estado = 0;
    unsigned int ultimo_toggle = 0;
    unsigned int periodo_atual = TEMPO_PISCA_3S;
    unsigned int agora;
    char opcao;

    disable_watchdog();
    iniciar();
    iniciar_uart0();

    uart_puts("\r\nSistema iniciado\r\n");
    uart_puts("Menu:\r\n");
    uart_puts("1 - Frequencia 3s no LED AZUL \r\n");
    uart_puts("2 - Frequencia 5s no LED VERDE \r\n");
    uart_puts("Digite 1 ou 2 a qualquer momento.\r\n");

    desliga(LED_EXTERNO);
    desliga(LED_EXTERNO2);

    DMTimerCounterSet(SOC_DMTIMER_7_REGS, 0);
    DMTimerEnable(SOC_DMTIMER_7_REGS);

    ultimo_toggle = DMTimerCounterGet(SOC_DMTIMER_7_REGS);
/*
    while(1)
    {
        agora = DMTimerCounterGet(SOC_DMTIMER_7_REGS);

        if(uart_rx_ready())
        {
            opcao = uart_getc();

            if(opcao == '1')
            {
                periodo_atual = TEMPO_PISCA_3S;
                led_estado = 0;
                led2_estado = 0;
                desliga(LED_EXTERNO);
                desliga(LED_EXTERNO2);
                ultimo_toggle = agora;
                uart_puts("\r\nSelecionado 3s no LED AZUL \r\n");
            }
            else if(opcao == '2')
            {
                periodo_atual = TEMPO_PISCA_5S;
                led_estado = 0;
                led2_estado = 0;
                desliga(LED_EXTERNO);
                desliga(LED_EXTERNO2);
                ultimo_toggle = agora;
                uart_puts("\r\nSelecionado 5s no LED VERDE\r\n");
            }
        }

        if(periodo_atual == TEMPO_PISCA_3S)
        {
            if((agora - ultimo_toggle) >= (periodo_atual * 24))
            {
                ultimo_toggle = agora;
                led_estado ^= 1;

                if(led_estado)
                    liga(LED_EXTERNO);
                else
                    desliga(LED_EXTERNO);
            }
        }
        else if(periodo_atual == TEMPO_PISCA_5S)
        {
            if((agora - ultimo_toggle) >= (periodo_atual * 24))
            {
                ultimo_toggle = agora;
                led2_estado ^= 1;

                if(led2_estado)
                    liga(LED_EXTERNO2);
                else
                    desliga(LED_EXTERNO2);
            }
        }
    }
    */
    while(1){
         liga(LED_EXTERNO);
         uDelay(10);
         desliga(LED_EXTERNO);
         uDelay(10);
    }
}
