#define GPH2CON (*(volatile unsigned int *)0xE0200C40)
#define GPH2DAT (*(volatile unsigned int *)0xE0200C44)




void led_on(void)
{
	GPH2CON &= 0xFFFF<<16;
	GPH2CON |= 0x1111<<16;
	GPH2DAT |= 0xFF;
}
void led_off(void)
{
	GPH2CON &= 0xFFFF<<16;
	GPH2CON |= 0x1111<<16;
	GPH2DAT &= ~0xFF;
}
void led_inv(void)
{
	GPH2CON &= 0xFFFF<<16;
	GPH2CON |= 0x1111<<16;
	GPH2DAT ^= 0xFF;
}

static void delay(unsigned int t)
{
	volatile unsigned int i,j;
	for(i=0; i<t; i++)
	{
		for(j=0; j<50000; j++);
	}
}

void led_init(void)
{
	int count = 5;
	while(count--)
	{
		led_on();
		delay(10);
		led_off();
		delay(10);
	}
}
