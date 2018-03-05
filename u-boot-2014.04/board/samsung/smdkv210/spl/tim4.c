#include "tim4.h"

static volatile unsigned long g_counter = 0;


unsigned long tim4_poll(void)
{
	if(TINT_CSTAT & (1<<9)) //Timer 4 Interrupt Status
	{
		TINT_CSTAT |= (1<<9); //Clear Timer 4 Interrupt Status
		g_counter += 10;
	}
	return g_counter;
}


void tim4_init(void)
{
    g_counter = 0;
    
    //设置预分频
    TCFG0 &= ~(0xFF<<8);
    TCFG0 |= (65<<8);                    //1级分频以后为66/65+1 = 1MHz
    //设置2级分频
    TCFG1 &= ~(0xF<<16);
    TCFG1 |= (0x2<<16);                  //2级分频以后频率为1/4 = 250KHz
    
    //设置TCNTB寄存器
    TCNTB4 = 2500;

    //设置TCON寄存器
	TCON |= (0x1<<21); //Update TCNTB4, Manual Update First time!!!!!!
	TCON &= ~(0x7<<20);
	TCON |= (0x1<<22); //Auto-Reload
    TCON |= (1<<20); //Start Timer4
    
    TINT_CSTAT &= ~(1<<4); // 中断禁能
    TINT_CSTAT |= (1<<9); // 清中断状态
    
    TCON |= (1<<20); //Start Timer4
}


void delay(unsigned int t)
{
	volatile unsigned int i,j;
	for(i=0; i<t; i++)
	{
		for(j=0; j<10000; j++);
	}
}

