#ifndef TIMER_H
#define TIMER_H

void DMTimerCounterSet(unsigned int baseAdd, unsigned int counter);
unsigned int DMTimerCounterGet(unsigned int baseAdd);
void DMTimerEnable(unsigned int baseAdd);
void DMTimerDisable(unsigned int baseAdd);
void uDelay(unsigned int us);

#endif

