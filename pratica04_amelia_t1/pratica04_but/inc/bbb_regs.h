#ifndef _BBB_REGS_H_
#define _BBB_REGS_H_

/* UART0 */
#define UART0_BASE (*((volatile unsigned int*)0x44E09000))
#define UART0_RHR  (*((volatile unsigned int*)0x44E09000))
#define UART0_THR  (*((volatile unsigned int*)0x44E09000))
#define UART0_LSR  (*((volatile unsigned int*)0x44E09014))

/* Watch Dog Timer */
#define  WDT_BASE (*((volatile unsigned int*)0x44E35000))
#define  WDT_WSPR (*((volatile unsigned int*)0x44E35048))
#define  WDT_WWPS (*((volatile unsigned int*)0x44E35034))

/* Clock */
#define CM_PER_GPIO1_CLKCTRL (*((volatile unsigned int*) 0x44e000AC))
#define CM_RTC_RTC_CLKCTRL   (*((volatile unsigned int*) 0x44E00800))
#define CM_RTC_CLKSTCTRL     (*((volatile unsigned int*) 0x44E00804))

/* GPIO */
#define GPIO1_OE            (*((volatile unsigned int*) 0x4804C134))
#define GPIO1_SETDATAOUT    (*((volatile unsigned int*) 0x4804C194))
#define GPIO1_CLEARDATAOUT  (*((volatile unsigned int*) 0x4804C190))

/* RTC */
#define RTC_BASE           (*((volatile unsigned int*) 0x44E3E000))
#define SECONDS_REG        (*((volatile unsigned int*) 0x44E3E000))
#define MINUTES_REG        (*((volatile unsigned int*) 0x44E3E004))
#define HOURS_REG          (*((volatile unsigned int*) 0x44E3E008))
#define RTC_CTRL_REG       (*((volatile unsigned int*) 0x44E3E040))
#define RTC_INTERRUPTS_REG (*((volatile unsigned int*) 0x44E3E048))
#define RTC_OSC_REG        (*((volatile unsigned int*) 0x44E3E054))
#define KICK0R             (*((volatile unsigned int*) 0x44E3E06C))
#define KICK1R             (*((volatile unsigned int*) 0x44E3E070))

#endif

