#include "soc_AM335x.h"
#include "hw_types.h"


/* ---------------- DEFINES ---------------- */

#define TEMPO_SEQUENCIA 2000000  // Equivalente a 2 segundos
#define TEMPO_DEBOUNCE  10000   // Equivalente a 10 milissegundos

/* ---------------- CLOCK ---------------- */

#define SOC_PRCM_REGS             (0x44E00000)

#define CM_PER_GPIO1_CLKCTRL      (0x0AC)
#define CM_PER_TIMER7_CLKCTRL_OFFSET (0x007C) /* Energia do Timer 7 */

/*----------------- TIMER -----------------------*/
#define DMTIMER_TCLR                 (0x0038) /* Controle */
#define DMTIMER_TCRR                 (0x003C) /* Contador */
#define DMTIMER_TWPS                 (0x0048) /* Status de Escrita Pendente (Sincronismo) */
#define DMTIMER_TSICR                (0x0054) /* Controle de Interface Síncrona */

#define DMTIMER_TCLR_ST              (1 << 0) /* Bit 0: Start */
#define DMTIMER_TSICR_POSTED         (1 << 2) /* Bit 2: Ativa modo de sincronismo (Posted) */
#define DMTIMER_WRITE_POST_TCLR      (1 << 0) /* Bit 0 do TWPS: TCLR está ocupado */
#define DMTIMER_WRITE_POST_TCRR      (1 << 1) /* Bit 1 do TWPS: TCRR está ocupado */

/* Matemática do Tempo (Clock de 24MHz) */
#define TIMER_1US_COUNT              (24)     /* 24 ciclos = 1 microssegundo (us) */

#define DMTimerWaitForWrite(reg, baseAdd) \
    if(HWREG(baseAdd + DMTIMER_TSICR) & DMTIMER_TSICR_POSTED) \
        while((reg & HWREG(baseAdd + DMTIMER_TWPS)))

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

#define TODOS_LEDS                ( LED_USR0 | \
                                    LED_USR1 | \
                                    LED_USR2 | \
                                    LED_USR3 )


/* ---------------- BOTÕES ---------------- */

#define BOTAO1                    (1 << 28)
#define BOTAO2                    (1 << 16)

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

    HWREG(SOC_CONTROL_REGS + CONF_GPMC_BE1N) = 0x27;

    HWREG(SOC_CONTROL_REGS + CONF_GPMC_A0) = 0x27;



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


    /* LEDs internos como saída */

    dir &= ~LED_USR0;

    dir &= ~LED_USR1;

    dir &= ~LED_USR2;

    dir &= ~LED_USR3;


    /* LED externo como saída */

    dir &= ~LED_EXTERNO;


    /* Escreve configuração */

    HWREG(SOC_GPIO_1_REGS + GPIO_OE) = dir;



    /* 5. INICIA LEDs DESLIGADOS                        */


    HWREG(SOC_GPIO_1_REGS + GPIO_CLEARDATAOUT) =
            TODOS_LEDS | LED_EXTERNO;
}

/* ---------------- FUNÇÕES TIMER ---------------- */

void DMTimerCounterSet(unsigned int baseAdd, unsigned int counter) {
    DMTimerWaitForWrite(DMTIMER_WRITE_POST_TCRR, baseAdd);
    HWREG(baseAdd + DMTIMER_TCRR) = counter;
}

unsigned int DMTimerCounterGet(unsigned int baseAdd){
    DMTimerWaitForWrite(DMTIMER_WRITE_POST_TCRR, baseAdd);
    return (HWREG(baseAdd + DMTIMER_TCRR));
}

void DMTimerEnable(unsigned int baseAdd){
    DMTimerWaitForWrite(DMTIMER_WRITE_POST_TCLR, baseAdd);
    HWREG(baseAdd + DMTIMER_TCLR) |= DMTIMER_TCLR_ST;
}

void DMTimerDisable(unsigned int baseAdd){
    DMTimerWaitForWrite(DMTIMER_WRITE_POST_TCLR, baseAdd);
    HWREG(baseAdd + DMTIMER_TCLR) &= ~DMTIMER_TCLR_ST;
}

/* A função de Atraso Universal (Microsegundos) */
void uDelay (unsigned int us){
    unsigned int ticks_needed = us * TIMER_1US_COUNT;
    DMTimerCounterSet(SOC_DMTIMER_7_REGS, 0);
    DMTimerEnable(SOC_DMTIMER_7_REGS);
    while(DMTimerCounterGet(SOC_DMTIMER_7_REGS) < ticks_needed);
    DMTimerDisable(SOC_DMTIMER_7_REGS);
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
    return HWREG(SOC_GPIO_1_REGS + GPIO_DATAIN) & botao;
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
    int padrao;
    int ultimo1;
    int ultimo2;
    int estadoLE;

    /* Inicializações */

    disable_watchdog();

    iniciar();


    /* Estados iniciais */

    padrao = 0;

    ultimo1 = 0;

    ultimo2 = 0;

    estadoLE = 0;


    /* Loop principal */

    while(1)
    {


        /* BOTÃO 1 -> TROCA PADRÃO DOS LEDs INTERNOS        */

        if(pressionado(BOTAO1) && !ultimo1)
        {
	   uDelay(TEMPO_DEBOUNCE);

            if(pressionado(BOTAO1))
            {
                /* Próximo padrão */

                padrao = (padrao + 1) % 3;

                /* Apaga LEDs */

                apagaTudo();

                /* Espera botão ser solto */

                while(pressionado(BOTAO1))
                {
		  uDelay(TEMPO_DEBOUNCE);
                }
            }
        }

        /* Atualiza estado anterior do botão */

        ultimo1 = pressionado(BOTAO1);



        /* BOTÃO 2 -> CONTROLA LED EXTERNO                  */

        if(pressionado(BOTAO2) && !ultimo2)
        {
            uDelay(TEMPO_DEBOUNCE);

            if(pressionado(BOTAO2))
            {
                /* Inverte estado */

                estadoLE ^= 1;

                /* Liga ou desliga LED */

                if(estadoLE)
                {
                    liga(LED_EXTERNO);
                }
                else
                {
                    desliga(LED_EXTERNO);
                }

                /* Espera botão ser solto */

                while(pressionado(BOTAO2))
                {
                    uDelay(TEMPO_DEBOUNCE);
                }
            }
        }

        /* Atualiza estado anterior do botão */

        ultimo2 = pressionado(BOTAO2);



        /* EXECUTA PADRÃO ATUAL                             */

        switch(padrao)
        {
            case 0:

                sequencia1();

                break;


            case 1:

                sequencia2();

                break;


            case 2:

                sequencia3();

                break;
        }


        /* Pequeno atraso quando nada acontece */

        if(!pressionado(BOTAO1) && !pressionado(BOTAO2))
        {
            uDelay(TEMPO_DEBOUNCE);
        }
    }
}


