#ifndef _TIMER_H__
#define _TIMER_H__





#define TCFG0		(*(volatile unsigned int *)0xE2500000)
#define TCFG1		(*(volatile unsigned int *)0xE2500004)
#define TCON		(*(volatile unsigned int *)0xE2500008)
#define TCNTB4		(*(volatile unsigned int *)0xE250003C)
#define TINT_CSTAT	(*(volatile unsigned int *)0xE2500044)
#define TCNTO4		(*(volatile unsigned int *)0xE2500040) 




unsigned long tim4_poll(void);
void tim4_init(void);


#endif //_TIMER_H__

