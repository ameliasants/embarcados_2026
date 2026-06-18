#include "soc_AM335x.h"
#include "hw_types.h"
#include "timer.h"


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
#define TIMER_1US_COUNT              (24000)     /* 24 ciclos = 1 microssegundo (us) */

#define DMTimerWaitForWrite(reg, baseAdd) \
    if(HWREG(baseAdd + DMTIMER_TSICR) & DMTIMER_TSICR_POSTED) \
        while((reg & HWREG(baseAdd + DMTIMER_TWPS)))



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
  
    unsigned int inicio = DMTimerCounterGet(SOC_DMTIMER_7_REGS);
    unsigned int ticks_necessarios = us * TIMER_1US_COUNT;

    while((DMTimerCounterGet(SOC_DMTIMER_7_REGS) - inicio) < ticks_necessarios)
    {

    }
}
