#include "soc_AM335x.h"
#include "hw_types.h"


/* ---------------- DEFINES ---------------- */

/* ---------------- UART ---------------- */
#define CM_WKUP_UART0_CLKCTRL_OFFSET            (0x00B4)
#define UART_THR_RHR_OFFSET (0x0000) // Transmite/Recebe 
#define UART_LSR_OFFSET     (0x0014) // Status da linha

/*--------------- Control Module UART --------------*/
#define CONF_UART0_RXD_OFFSET                   (0x970)
#define CONF_UART0_TXD_OFFSET                   (0x974)

/*--------------- Control Module UART --------------*/
#define UART_LSR_TX_EMPTY   (1 << 5)
#define UART_LSR_RX_READY   (1 << 0)
/*---------------  Register Access Mode Overview  --------------*/
#define UART_SYSC_OFFSET                        (0x0054) // System Configuration (Reset)
#define UART_SYSS_OFFSET                        (0x0058) // System Status (Confirma Reset)
#define UART_LCR_OFFSET                         (0x000C) // Line Control
#define UART_DLL_OFFSET                         (0x0000) // Divisor Latch Low
#define UART_DLH_OFFSET                         (0x0004) // Divisor Latch High
#define UART_MDR1_OFFSET                        (0x0020) // Mode Definition
/* ---------------- DELAY ---------------- */
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
/* ---------------- INICIALIZA UART0---------------- */

void iniciar_uart0()
{
    HWREG(SOC_CM_WKUP_REGS + CM_WKUP_UART0_CLKCTRL_OFFSET) = 0x02;

    /* MUX DOS PINOS*/
    HWREG(SOC_CONTROL_REGS + CONF_UART0_RXD_OFFSET) = (1 << 5) | (1 << 4) | 0x0;
    HWREG(SOC_CONTROL_REGS + CONF_UART0_TXD_OFFSET) = 0x0;

    /*RESET SOFTWARE*/
    HWREG(SOC_UART_0_REGS + UART_SYSC_OFFSET) |= 0x02;
    /*ATÉ O BIT DO RESET APAGAR*/
    while((HWREG(SOC_UART_0_REGS + UART_SYSC_OFFSET) & 0x01) == 0){

    }

    /*DESATIVA A UART TEMPORARIAMENTE(modo 7)*/
    HWREG(SOC_UART_0_REGS + UART_MDR1_OFFSET) = 0x07;

    /*MODO CONFIGURAÇÃO DE VELOCIDADE*/
    HWREG(SOC_UART_0_REGS + UART_LCR_OFFSET) = 0x03;

    /*DIVISOR 26*/
    HWREG(SOC_UART_0_REGS + UART_DLL_OFFSET) = 0x1A; // Low byte
    HWREG(SOC_UART_0_REGS + UART_DLH_OFFSET) = 0x00; // High byte

    /*CONFIGURA FORMATO DA LINHA */
    HWREG(SOC_UART_0_REGS + UART_LCR_OFFSET) = 0x03;

    /*LIGAR UART*/
    HWREG(SOC_UART_0_REGS + UART_MDR1_OFFSET) = 0x00;
}
void uart_putc(char c)
{
    while((HWREG(SOC_UART_0_REGS + UART_LSR_OFFSET) & UART_LSR_TX_EMPTY) == 0) {}
    
    HWREG(SOC_UART_0_REGS + UART_THR_RHR_OFFSET) = c;
}

void uart_puts(char *str)
{
    while(*str != '\0')
    {
        uart_putc(*str);
        str++;
    }
}

char uart_getc(void)
{
    while((HWREG(SOC_UART_0_REGS + UART_LSR_OFFSET) & UART_LSR_RX_READY) == 0) {}
    
    return (HWREG(SOC_UART_0_REGS + UART_THR_RHR_OFFSET));
}
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

    HWREG(SOC_CONTROL_REGS + CONF_GPMC_AD12) = 0x27;



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
    int ultimo3;
    int estadoLE;

    /* Inicializações */

    disable_watchdog();

    iniciar();

    iniciar_uart0();

    /* Estados iniciais */

    padrao = 0;

    ultimo1 = 0;

    ultimo2 = 0;

    ultimo3 = 0;

    estadoLE = 0;

    unsigned int tempo_atual = 2000000;

    uart_puts("\r\n=================================");
    uart_puts("\r\n  PISCA LED - METAL NU ");
    uart_puts("\r\n=================================");
    uart_puts("\r\nFrequencia Atual: LENTA (2s)\r\n");
    /* Loop principal */

    while(1)
    {


        /* BOTÃO 1 -> 1° Frequência        */

        if(pressionado(BOTAO1) && !ultimo1)
        {
	        uDelay(TEMPO_DEBOUNCE);

            if(pressionado(BOTAO1))
            {
                tempo_atual = 150000; /* Muda a variável de tempo */
                uart_puts("\r\n[Aviso] Frequencia alterada: RAPIDA (0.15s)");
                /* Próximo padrão */


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
                tempo_atual = 600000; 
                uart_puts("\r\n[Aviso] Frequencia alterada: MEDIA (0.6s)");
             

                while(pressionado(BOTAO2))
                {
                    uDelay(TEMPO_DEBOUNCE);
                }
            }
        }

        /* Atualiza estado anterior do botão */

        ultimo2 = pressionado(BOTAO2);

        /* --- VERIFICA BOTÃO 3 (Frequência Lenta) --- */
        if(pressionado(BOTAO3) && !ultimo3)
        {
            uDelay(TEMPO_DEBOUNCE);
            if(pressionado(BOTAO3))
            {
                tempo_atual = 2000000; 
                uart_puts("\r\n[Aviso] Frequencia alterada: LENTA (2s)");
                while(pressionado(BOTAO3)) { uDelay(TEMPO_DEBOUNCE); }
            }
        }
        ultimo3 = pressionado(BOTAO3);

        /* EXECUTA SEQUENCIA ATUAL                             */
        /* 1. Acende o LED*/
        liga(LED_EXTERNO);       
        
        /* 2. Mantém ele aceso  */
        uDelay(tempo_atual);     
        
        /* 3. Apaga o LED */
        desliga(LED_EXTERNO);    
        
        /* 4. Mantém ele apagadO*/
        uDelay(tempo_atual);
       


        
    }
}

