#ifndef UART_H
#define UART_H

void iniciar_uart0(void);
void uart_putc(char c);
void uart_puts(char *str);
char uart_getc(void);

#endif