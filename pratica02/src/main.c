#include "soc_AM335x.h"
#include "hw_types.h"

/* ============================================================
 * DEFINES
 * ============================================================
 */

#define TEMPO 1000000

/* ---------------- CLOCK ---------------- */

#define SOC_PRCM_REGS              (0x44E00000)
#define SOC_CM_PER_REGS            (SOC_PRCM_REGS + 0)

#define CKM_PER_GPIO1_CLKCTRL      (0x0AC)

/* ---------------- CONTROL MODULE ---------------- */

#define SOC_CONTROL_REGS           (0x44E10000)

#define CM_conf_gpmc_a5            (0x0854)
#define CM_conf_gpmc_a6            (0x0858)
#define CM_conf_gpmc_a7            (0x085C)
#define CM_conf_gpmc_a8            (0x0860)
#define CM_conf_gpmc_ben1          (0x0878)

/* ---------------- GPIO ---------------- */

#define SOC_GPIO_1_REGS            (0x4804C000)

#define GPIO_OE                    (0x134)
#define GPIO_CLEARDATAOUT          (0x190)
#define GPIO_SETDATAOUT            (0x194)

/* ---------------- LEDs ---------------- */

#define LED_USR0                   (1 << 21)
#define LED_USR1                   (1 << 22)
#define LED_USR2                   (1 << 23)
#define LED_USR3                   (1 << 24)

#define LED_EXTERNO                (1 << 28) // operação de bit para o pino 28

#define TODOS_LEDS                 ( LED_USR0  | \
                                     LED_USR1  | \
                                     LED_USR2  | \
                                     LED_USR3  | \
                                     LED_EXTERNO )

/* ============================================================
 * PROTÓTIPOS
 * ============================================================
 */

void delay(void);

void inicializaGPIO(void);

void apagaTodosLEDs(void);

void sequenciaCrescente(void);
void sequenciaDecrescente(void);
void piscaTodos(void);

/* ============================================================
 * MAIN
 * ============================================================
 */

int _main(void)
{
  /* Inicializa os pinos GPIO utilizados */
    inicializaGPIO();

    while(1)
    {
        sequenciaCrescente();

        sequenciaDecrescente();

        piscaTodos();
    }

    return 0;
}

/* ============================================================
 * DELAY
 * ============================================================
 */

void delay(void)
{
    volatile unsigned int i;
    
       /* Pequeno atraso por software */
    for(i = 0; i < TEMPO; i++);
}

/* ============================================================
 * INICIALIZA GPIO
 * ============================================================
 */

void inicializaGPIO(void)
{
    unsigned int valor_temp;
    unsigned int endereco_temp;

    
    /* Habilita clock do módulo GPIO1 */
    HWREG(SOC_CM_PER_REGS + CKM_PER_GPIO1_CLKCTRL) |=
            (1 << 18) | (0x2 << 0);

     /* Configura os pinos para função GPIO */
    HWREG(SOC_CONTROL_REGS + CM_conf_gpmc_a5)   |= 7;
    HWREG(SOC_CONTROL_REGS + CM_conf_gpmc_a6)   |= 7;
    HWREG(SOC_CONTROL_REGS + CM_conf_gpmc_a7)   |= 7;
    HWREG(SOC_CONTROL_REGS + CM_conf_gpmc_a8)   |= 7;

    HWREG(SOC_CONTROL_REGS + CM_conf_gpmc_ben1) |= 7;

    /* --------------------------------------------------------
     * CONFIGURA GPIO COMO SAÍDA
     * 0 = OUTPUT
     * --------------------------------------------------------
     */

    endereco_temp = SOC_GPIO_1_REGS + GPIO_OE;

    valor_temp = HWREG(endereco_temp);

    valor_temp &= ~LED_USR0;
    valor_temp &= ~LED_USR1;
    valor_temp &= ~LED_USR2;
    valor_temp &= ~LED_USR3;

    valor_temp &= ~LED_EXTERNO;

    HWREG(endereco_temp) = valor_temp;
}

/* ============================================================
 * APAGA TODOS OS LEDS
 * ============================================================
 */

void apagaTodosLEDs(void)
{
    HWREG(SOC_GPIO_1_REGS + GPIO_CLEARDATAOUT) = TODOS_LEDS;
}

/* ============================================================
 * SEQUÊNCIA CRESCENTE
 * ============================================================
 */

void sequenciaCrescente(void)
{
    apagaTodosLEDs();

    HWREG(SOC_GPIO_1_REGS + GPIO_SETDATAOUT) = LED_USR0;
    delay();

    apagaTodosLEDs();

    HWREG(SOC_GPIO_1_REGS + GPIO_SETDATAOUT) = LED_USR1;
    delay();

    apagaTodosLEDs();

    HWREG(SOC_GPIO_1_REGS + GPIO_SETDATAOUT) = LED_USR2;
    delay();

    apagaTodosLEDs();

    HWREG(SOC_GPIO_1_REGS + GPIO_SETDATAOUT) = LED_USR3;
    delay();

    apagaTodosLEDs();

    HWREG(SOC_GPIO_1_REGS + GPIO_SETDATAOUT) = LED_EXTERNO;
    delay();
}

/* ============================================================
 * SEQUÊNCIA DECRESCENTE
 * ============================================================
 */

void sequenciaDecrescente(void)
{
    apagaTodosLEDs();

    HWREG(SOC_GPIO_1_REGS + GPIO_SETDATAOUT) = LED_EXTERNO;
    delay();

    apagaTodosLEDs();

    HWREG(SOC_GPIO_1_REGS + GPIO_SETDATAOUT) = LED_USR3;
    delay();

    apagaTodosLEDs();

    HWREG(SOC_GPIO_1_REGS + GPIO_SETDATAOUT) = LED_USR2;
    delay();

    apagaTodosLEDs();

    HWREG(SOC_GPIO_1_REGS + GPIO_SETDATAOUT) = LED_USR1;
    delay();

    apagaTodosLEDs();

    HWREG(SOC_GPIO_1_REGS + GPIO_SETDATAOUT) = LED_USR0;
    delay();
}

/* ============================================================
 * PISCA TODOS OS LEDS
 * ============================================================
 */

void piscaTodos(void)
{
    HWREG(SOC_GPIO_1_REGS + GPIO_SETDATAOUT) = TODOS_LEDS;

    delay();

    HWREG(SOC_GPIO_1_REGS + GPIO_CLEARDATAOUT) = TODOS_LEDS;

    delay();
} q
