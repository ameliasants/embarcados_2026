#include "soc_AM335x.h"
#include "hw_types.h"

/* ---------------- UART ---------------- */
#define CM_WKUP_UART0_CLKCTRL_OFFSET            (0x00B4)
#define UART_THR_RHR_OFFSET                     (0x0000) // Transmite/Recebe 
#define UART_LSR_OFFSET                         (0x0014) // Status da linha


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

/* ---------------- INICIALIZA UART0---------------- */

void iniciar_uart0()
{
    HWREG(SOC_CM_WKUP_REGS + CM_WKUP_UART0_CLKCTRL_OFFSET) = 0x02;

    /* MUX DOS PINOS*/
    HWREG(SOC_CONTROL_REGS + CONF_UART0_RXD_OFFSET) = (1 << 5) | (1 << 4) | 0x0;
    HWREG(SOC_CONTROL_REGS + CONF_UART0_TXD_OFFSET) = 0x0;

   /* RESET SOFTWARE */
    HWREG(SOC_UART_0_REGS + UART_SYSC_OFFSET) |= 0x02;

    /* ESPERA O RESET COMPLETAR */
    while((HWREG(SOC_UART_0_REGS + UART_SYSS_OFFSET) & 0x01) == 0){}

    /*DESATIVA A UART TEMPORARIAMENTE(modo 7)*/
    HWREG(SOC_UART_0_REGS + UART_MDR1_OFFSET) = 0x07;

    /*MODO CONFIGURAÇÃO DE VELOCIDADE*/
    HWREG(SOC_UART_0_REGS + UART_LCR_OFFSET) = 0x83;

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
