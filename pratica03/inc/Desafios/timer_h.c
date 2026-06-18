#include "soc_AM335x.h"
#include "hw_types.h"



/* Control Module (Pin Muxing do Botão P8_11) */
#define CONF_GPMC_AD13_OFFSET        (0x0834)  

/* PRCM (Energia e Clock) */
#define CM_PER_GPIO1_CLKCTRL_OFFSET  (0x00AC)
#define CM_PER_TIMER7_CLKCTRL_OFFSET (0x007C) /* Energia do Timer 7 */

/* GPIO1 */
#define GPIO1_OE_OFFSET              (0x0134)
#define GPIO1_DATAIN_OFFSET          (0x0138)
#define GPIO1_CLEARDATAOUT_OFFSET    (0x0190)
#define GPIO1_SETDATAOUT_OFFSET      (0x0194)

/* Watchdog */
#define WDT_WSPR_OFFSET              (0x0048) 
#define WDT_WWPS_OFFSET              (0x0034) 

#define WDT1_WSPR HWREG(SOC_WDT_1_REGS + WDT_WSPR_OFFSET)
#define WDT1_WWPS HWREG(SOC_WDT_1_REGS + WDT_WWPS_OFFSET)

/* Pinos */
#define USR0 21
#define USR1 22
#define BUTTON 13


/* Timer */
#define DMTIMER_TCLR                 (0x0038) /* Controle */
#define DMTIMER_TCRR                 (0x003C) /* Contador */
#define DMTIMER_TWPS                 (0x0048) /* Status de Escrita Pendente (Sincronismo) */
#define DMTIMER_TSICR                (0x0054) /* Controle de Interface Síncrona */

/*Máscaras  */
#define DMTIMER_TCLR_ST              (1 << 0) /* Bit 0: Start */
#define DMTIMER_TSICR_POSTED         (1 << 2) /* Bit 2: Ativa modo de sincronismo (Posted) */
#define DMTIMER_WRITE_POST_TCLR      (1 << 0) /* Bit 0 do TWPS: TCLR está ocupado */
#define DMTIMER_WRITE_POST_TCRR      (1 << 1) /* Bit 1 do TWPS: TCRR está ocupado */

/* Matemática do Tempo (Clock de 24MHz) */
#define TIMER_1US_COUNT              (24)     /* 24 ciclos = 1 microssegundo (us) */



void disable_watchdog(){
    while(WDT1_WWPS != 0){}
    WDT1_WSPR = 0xAAAA;
    while(WDT1_WWPS != 0){}
    WDT1_WSPR = 0x5555;
    while(WDT1_WWPS != 0){}
}

#define DMTimerWaitForWrite(reg, baseAdd) \
    if(HWREG(baseAdd + DMTIMER_TSICR) & DMTIMER_TSICR_POSTED) \
        while((reg & HWREG(baseAdd + DMTIMER_TWPS)))

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

int main (){

    /* 1. Desliga o cão de guarda */
    disable_watchdog();
    
    /* 2. Liga a energia dos Módulos (GPIO e TIMER 7) */
    HWREG(SOC_CM_PER_REGS + CM_PER_GPIO1_CLKCTRL_OFFSET) = 0x02;
    HWREG(SOC_CM_PER_REGS + CM_PER_TIMER7_CLKCTRL_OFFSET) = 0x02; 

    /* 3. Configura o pino físico com Pull-Up interno */
    HWREG(SOC_CONTROL_REGS + CONF_GPMC_AD13_OFFSET) = 0x37; 

    /* 4. Configuração das Portas (OE) */
    HWREG(SOC_GPIO_1_REGS + GPIO1_OE_OFFSET) &= ~((1 << USR0) | (1 << USR1));
    HWREG(SOC_GPIO_1_REGS + GPIO1_OE_OFFSET) |= (1 << BUTTON);

    int count = 0;
    int est_bot = 1;
    int est_ant = 1;

    while(1){
        
        /* Lê o Botão */
        if(HWREG(SOC_GPIO_1_REGS + GPIO1_DATAIN_OFFSET) & (1 << BUTTON)){
            est_bot = 1; /* Solto */
        }
        else{
            est_bot = 0; /* Pressionado */
        }
        
        /* Máquina de Estados: Transição na Borda de Descida */
        if(est_bot == 0 && est_ant == 1){
            count++;

            if(count > 3){
                count = 0;
            }
            
            switch(count){
                case 0:
                    HWREG(SOC_GPIO_1_REGS + GPIO1_CLEARDATAOUT_OFFSET) = (1 << USR0) | (1 << USR1);
                    break;
                case 1:
                    HWREG(SOC_GPIO_1_REGS + GPIO1_SETDATAOUT_OFFSET) = (1 << USR0);
                    HWREG(SOC_GPIO_1_REGS + GPIO1_CLEARDATAOUT_OFFSET) = (1 << USR1);
                    break;
                case 2:
                    HWREG(SOC_GPIO_1_REGS + GPIO1_CLEARDATAOUT_OFFSET) = (1 << USR0);
                    HWREG(SOC_GPIO_1_REGS + GPIO1_SETDATAOUT_OFFSET) = (1 << USR1);
                    break;
                case 3:
                    HWREG(SOC_GPIO_1_REGS + GPIO1_SETDATAOUT_OFFSET) = (1 << USR0) | (1 << USR1);
                    break;
            }
        }

        est_ant = est_bot;

        uDelay(1000); 
    }

    return 0;
}