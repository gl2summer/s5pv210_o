#include "types.h"
#include "uart.h"

#define APLL_LOCK		*((volatile u32 *)0xE0100000)
#define MPLL_LOCK		*((volatile u32 *)0xE0100008)
#define EPLL_LOCK		*((volatile u32 *)0xE0100010)
#define VPLL_LOCK		*((volatile u32 *)0xE0100020)

#define APLL_CON0		*((volatile u32 *)0xE0100100)
#define MPLL_CON		*((volatile u32 *)0xE0100108)
#define EPLL_CON0		*((volatile u32 *)0xE0100110)
#define EPLL_CON1		*((volatile u32 *)0xE0100114)
#define VPLL_CON		*((volatile u32 *)0xE0100120)

#define CLK_SRC0 		*((volatile u32 *)0xE0100200)
#define CLK_SRC1 		*((volatile u32 *)0xE0100204)
#define CLK_SRC2 		*((volatile u32 *)0xE0100208)
#define CLK_SRC3 		*((volatile u32 *)0xE010020C)
#define CLK_SRC4 		*((volatile u32 *)0xE0100210)
#define CLK_SRC5 		*((volatile u32 *)0xE0100214)
#define CLK_SRC6 		*((volatile u32 *)0xE0100218)

#define CLK_DIV0		*((volatile u32 *)0xE0100300)
#define CLK_DIV1		*((volatile u32 *)0xE0100304)
#define CLK_DIV2		*((volatile u32 *)0xE0100308)
#define CLK_DIV3		*((volatile u32 *)0xE010030C)
#define CLK_DIV4		*((volatile u32 *)0xE0100310)
#define CLK_DIV5		*((volatile u32 *)0xE0100314)
#define CLK_DIV6		*((volatile u32 *)0xE0100318)
#define CLK_DIV7		*((volatile u32 *)0xE010031C)



/* 计算x的y次方 */
u32 pow(u32 x, u8 y)
{
	if (y == 0)
		x = 1;
	else
	{
		y--;
		while (y--)
			x *= x;
	}
	return x;
}

/* 打印时钟信息 */
void print_clock(void)
{	
	u16 p, m, s, k;
	u32 SCLKAPLL, SCLKMPLL, SCLKEPLL, SCLKVPLL, t;
	u32 MOUT_MSYS, MOUT_DSYS, MOUT_PSYS;
	u8 APLL_RATIO, A2M_RATIO, HCLK_MSYS_RATIO, PCLK_MSYS_RATIO, HCLK_DSYS_RATIO, PCLK_DSYS_RATIO, HCLK_PSYS_RATIO, PCLK_PSYS_RATIO;
	APLL_RATIO = (CLK_DIV0 >> 0)& 0x7;
	A2M_RATIO  = (CLK_DIV0 >> 4) & 0x7;
	HCLK_MSYS_RATIO  = (CLK_DIV0 >> 8) & 0x7;
	PCLK_MSYS_RATIO  = (CLK_DIV0 >> 12) & 0x7;
	HCLK_DSYS_RATIO  = (CLK_DIV0 >> 16) & 0x7;
	PCLK_DSYS_RATIO  = (CLK_DIV0 >> 20) & 0x7;
	HCLK_PSYS_RATIO  = (CLK_DIV0 >> 24) & 0x7;
	PCLK_PSYS_RATIO  = (CLK_DIV0 >> 28) & 0x7;
	if (CLK_SRC0 & 0x1)
	{
		p = (APLL_CON0 >> 8)  & 0x3F;
		m = (APLL_CON0 >> 16) & 0x3FF;
		s = (APLL_CON0 >> 0)  & 0x7;
		SCLKAPLL = m * 24 / (p * pow(2, s - 1));	/* FOUT_APLL = MDIV X FIN / (PDIV ×2^(SDIV-1)) */
	}
	else
	{
		SCLKAPLL = 24;	// 外部晶振
	}
	if (CLK_SRC0 & (1 << 4))
	{
		p = (MPLL_CON >> 8)  & 0x3F;
		m = (MPLL_CON >> 16) & 0x3FF;
		s = (MPLL_CON >> 0)  & 0x7;
		SCLKMPLL = m * 24 / (p * pow(2, s));		/* FOUT_MPLL = MDIV X FIN / (PDIV ×2^SDIV) */
	}
	else
	{
		SCLKMPLL = 24;
	}
	if (CLK_SRC0 & (1 << 8))
	{
		p = (EPLL_CON0 >> 8)  & 0x3F;
		m = (EPLL_CON0 >> 16) & 0x1FF;
		s = (EPLL_CON0 >> 0)  & 0x7;
		k = EPLL_CON1;
		SCLKEPLL = (m + k / 65536) * 24 / (p * pow(2, s));	/* FOUT_EPLL = (MDIV + K / 65536) x FIN / (PDIV x 2^SDIV) */
	}
	else
	{
		SCLKEPLL = 24;
	}
	if (CLK_SRC0 & (1 << 12))
	{
		p = (VPLL_CON >> 8)  & 0x3F;
		m = (VPLL_CON >> 16) & 0x1FF;
		s = (VPLL_CON >> 0)  & 0x7;
		SCLKVPLL = m * 24 / (p * pow(2, s));		/* FOUT_VPLL = MDIV X FIN / (PDIV ×2^SDIV) */
	}
	else
	{
		SCLKVPLL = 24;
	}
	if (CLK_SRC0 & (1 << 16))
	{
		MOUT_MSYS = SCLKMPLL;
	}
	else
	{
		MOUT_MSYS = SCLKAPLL;
	}
	if (CLK_SRC0 & (1 << 20))
	{
		MOUT_DSYS = SCLKAPLL / (A2M_RATIO + 1);
	}
	else
	{
		MOUT_DSYS = SCLKMPLL;
	}
	if (CLK_SRC0 & (1 << 24))
	{
		MOUT_PSYS = SCLKAPLL / (A2M_RATIO + 1);
	}
	else
	{
		MOUT_PSYS = SCLKMPLL;
	}
	t = MOUT_MSYS / (APLL_RATIO + 1);
	uart_printf("ARMCLK 		= %d MHz\r\n", t);
	uart_printf("SCLKA2M 	= %d MHz\r\n", SCLKAPLL / (A2M_RATIO + 1));
	t /= (HCLK_MSYS_RATIO + 1);
	uart_printf("HCLK_MSYS 	= %d MHz\r\n", t);
	t /= (PCLK_MSYS_RATIO + 1);
	uart_printf("PCLK_MSYS 	= %d MHz\r\n", t);
	t = MOUT_DSYS / (HCLK_DSYS_RATIO + 1);
	uart_printf("HCLK_DSYS 	= %d MHz\r\n", t);
	t /= (PCLK_DSYS_RATIO + 1);
	uart_printf("PCLK_DSYS 	= %d MHz\r\n", t);
	t = MOUT_PSYS / (HCLK_PSYS_RATIO + 1);
	uart_printf("HCLK_PSYS	= %d MHz\r\n", t);
	uart_printf("PCLK_PSYS 	= %d MHz\r\n", t / (PCLK_PSYS_RATIO + 1));
	uart_printf("SCLKEPLL 	= %d MHz\r\n", SCLKEPLL);
	uart_printf("SCLKVPLL 	= %d MHz\r\n", SCLKVPLL);
	uart_printf("\r\n");
}
