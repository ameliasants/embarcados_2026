.equ INTC_BASE, 0x48200000

/* Modos do ARM */
.equ CPSR_I,   0x80
.equ CPSR_F,   0x40
.equ CPSR_IRQ, 0x12
.equ CPSR_SVC, 0x13

/* Tabela de Vetores (IVT) */
_vector_table:
    ldr pc, _reset
    ldr pc, _undf
    ldr pc, _swi
    ldr pc, _pabt
    ldr pc, _dabt
    nop
    ldr pc, _irq_addr    /* Aponta direto para a variável abaixo! */
    ldr pc, _fiq

_reset: .word _start
_undf:  .word 0x4030CE24
_swi:   .word 0x4030CE28
_pabt:  .word 0x4030CE2C
_dabt:  .word 0x4030CE30
_irq_addr: .word .irq_handler  /* Bypass: Pula direto para nossa rotina Assembly */
_fiq:   .word 0x4030CE3C

/* Startup Code */
_start:
    /* Zera a flag V no registrador CP15 SCTRL */
    mrc p15, 0, r0, c1, c0, 0
    bic r0, r0, #(1<<13)
    mcr p15, 0, r0, c1, c0, 0

    /* Define o endereço da nossa Tabela de Vetores no CP15 VBAR */
    ldr r0, =_vector_table
    mcr p15, 0, r0, c12, c0, 0

    /* 1. CRIA O STACK DO MODO IRQ (Erro do PDF corrigido) */
    mrs r0, cpsr
    bic r0, r0, #0x1F
    orr r0, r0, #CPSR_IRQ
    msr cpsr_c, r0
    ldr sp, =0x40300000     /* Reserva espaço na RAM interna para a interrupção */

    /* 2. CRIA O STACK DO MODO SVC (MAIN) E HABILITA IRQ GLOBAL */
    mrs r0, cpsr
    bic r0, r0, #0x1F
    orr r0, r0, #CPSR_SVC
    orr r0, r0, #CPSR_F     /* Desabilita FIQ */
    and r0, r0, #~(CPSR_I)  /* Habilita IRQ (Limpa o bit I) */
    msr cpsr_c, r0
    ldr sp, =0x80200000     /* Reserva espaço na DDR RAM para o código em C */

    /* Salta para a função principal em C */
    bl _main

.loop: b .loop

/* Rotina de interrupção Assembly */
.irq_handler:
    stmfd sp!, {r0-r12, lr}   /* Agora o sp (Stack Pointer) existe! */
    mrs r11, spsr
    
    bl ISR_Handler            /* Invoca o tratador de interrupções no seu main.c */
    
    dsb
    msr spsr, r11
    ldmfd sp!, {r0-r12, lr}
    subs pc, lr, #4           /* Retorna o PC para onde o while(1) estava */