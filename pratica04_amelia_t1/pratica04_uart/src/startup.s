
_start:

    mrs r0, cpsr @Move o conteudo do CPSR para r0
    bic r0, r0, #0x1F @Limpa os 5 bits menos significativos
    orr r0, r0, #0x13 @Define 0x13, que corresponde ao Modo SVC
    orr r0, r0, #0xC0 
    msr cpsr, r0 @Escreve de volta no CPSR para aplicar
   
    ldr sp, = 0x4030CDFC

    bl _main
    

.loop: b .loop
