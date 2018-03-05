#include <stddef.h>
#include <stdarg.h>
#include "uart.h"


/* UART2初始化 */
void uart_init(void)
{
	/** 配置GPA1_0为UART_2_RXD
	** 配置GPA1_1为UART_2_TXD */
	GPA1CON &= ~0xFF;
	GPA1CON |= 0x22;

	/* 8-bits/One stop bit/No parity/Normal mode operation */
	ULCON2 = 0x3 | (0 << 2) | (0 << 3) | (0 << 6);

	/* Interrupt request or polling mode/Normal transmit/Normal operation/PCLK/ */
	UCON2 = 1 | (1 << 2) | (0 << 10);

	/* 静止FIFO */
	UFCON2 = 0;

	/*
	** 波特率计算：115200bps
	** PCLK = 66MHz
	** DIV_VAL = (66000000/(115200 x 16))-1 = 35.8 - 1 = 34.8
	** UBRDIV0 = 34(DIV_VAL的整数部分)
	** (num of 1's in UDIVSLOTn)/16 = 0.8
	** (num of 1's in UDIVSLOTn) = 12
	** UDIVSLOT0 = 0xDDDD (查表)
	*/
	UBRDIV2 = 34;
	UDIVSLOT2 = 0xDDDD;
	
	/* 屏蔽 MODEM、TXD、ERROR、RXD中断 */
	UINTM2 = 0xF;

	/* 清除中断及中断挂起标志 */
	UINTP2 |= 0xF;
	UINTSP2 |= 0xF;
}

static char uart_get(void)
{
	while (!(UTRSTAT2 & 0x1));		/* 等待接收缓冲区有数据可读 */
	return URXH2;					/* 接收一字节数据 */		
}

static void uart_put(char c)
{
	while (!(UTRSTAT2 & (0x1<<2)));	/* 等待发送缓冲区为空 */
	UTXH2 = (unsigned char)c;		/* 发送一字节数据 */		
}


char get_char(void)
{
	return uart_get();
}

bool try_get_char(char *c)
{
	if(!(UTRSTAT2 & 0x1))
		return false;
		
	if(c != NULL)
	{
		*c = URXH2;
	}
	return true;
}

void put_char(char c)
{
	uart_put(c);		
}

void put_chars(char *s, unsigned int len)
{
	while(len--)
	{
		uart_put(*s++);
	}
}

/* 打印字符串 */
void put_string(char *str)
{
	if(!str)
		return ;

	char *p = str;
	while (*p)
		put_char(*p++);
}


/* 打印uint整数到终端 */
static void put_int(u32 v)
{
	int i;
	u8 a[10];
	u8 cnt = 0;
	if (v == 0)
	{
		put_char('0');
		return;
	}
	while (v)
	{
		a[cnt++] = v % 10;
		v /= 10; 
	}
	for (i = cnt - 1; i >= 0; i--)
		put_char(a[i] + 0x30);/* 整数0-9的ASCII分别为0x30-0x39 */
}

/* 将uchar型数按16d进制打印到终端 */
static void put_hex(u8 v, u8 small)
{
	/* 注意：必须用volatile修饰，否则会出错 */
	vu8 h, l;		/* 高4位和第4位(这里按二进制算) */
	char *hex1 = "0123456789abcdef";		/* 这里放在数据段中 */
	char *hex2 = "0123456789ABCDEF";
	h = v >> 4;
	l = v & 0x0F;
	if (small)	/* 小写 */
	{
		put_char(hex1[h]);	/* 高4位 */
		put_char(hex1[l]);	/* 低4位 */
	}
	else		/* 大写 */
	{
		put_char(hex2[h]);	/* 高4位 */
		put_char(hex2[l]);	/* 低4位 */
	}
}

/* 将int型整数按16进制打印到终端 */
static void put_int_hex(u32 v, u8 small)
{
	if (v >> 24)
	{
		put_hex(v >> 24, small);
		put_hex((v >> 16) & 0xFF, small);
		put_hex((v >> 8) & 0xFF, small);
		put_hex(v & 0xFF, small);
	}
	else if ((v >> 16) & 0xFF)
	{
		put_hex((v >> 16) & 0xFF, small);
		put_hex((v >> 8) & 0xFF, small);
		put_hex(v & 0xFF, small);
	}
	else if ((v >> 8) & 0xFF)
	{
		put_hex((v >> 8) & 0xFF, small);
		put_hex(v & 0xFF, small);
	}
	else
		put_hex(v & 0xFF, small);
}


/* 格式化输出到终端 */
void uart_printf(const char *fmt, ...)
{
	va_list ap;
	char c;
	char *s;
	u32 d;
	u8 small;

	va_start(ap, fmt);
	while (*fmt)
	{
		small = 0;
		c = *fmt++;
		if (c == '%')
		{
			switch (*fmt++)
			{
				case 'c':              /* char */
					c = (char) va_arg(ap, int);
					put_char(c);
				break;
				case 's':              /* string */
					s = va_arg(ap, char *);
					put_string(s);
				break;
				case 'd':              /* int */
				case 'u':
					d = va_arg(ap, int);
					put_int(d);
				break;
				case 'x':
					small = 1;	// small
				case 'X':
					d = va_arg(ap, int);
					put_int_hex(d, small);
				break;
			}
		}
		else
		{
			put_char(c);
		}
	}
	va_end(ap);
}


