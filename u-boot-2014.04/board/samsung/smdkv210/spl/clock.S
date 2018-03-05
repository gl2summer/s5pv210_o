
#define APLL_LOCK		0xE0100000
#define MPLL_LOCK		0xE0100008
#define EPLL_LOCK		0xE0100010
#define VPLL_LOCK		0xE0100020

#define APLL_CON0		0xE0100100
#define MPLL_CON		0xE0100108
#define EPLL_CON0		0xE0100110
#define EPLL_CON1		0xE0100114
#define VPLL_CON		0xE0100120

#define CLK_SRC0 		0xE0100200
#define CLK_SRC1 		0xE0100204
#define CLK_SRC2 		0xE0100208
#define CLK_SRC3 		0xE010020C
#define CLK_SRC4 		0xE0100210
#define CLK_SRC5 		0xE0100214
#define CLK_SRC6 		0xE0100218

#define CLK_DIV0		0xE0100300
#define CLK_DIV1		0xE0100304
#define CLK_DIV2		0xE0100308
#define CLK_DIV3		0xE010030C
#define CLK_DIV4		0xE0100310
#define CLK_DIV5		0xE0100314
#define CLK_DIV6		0xE0100318
#define CLK_DIV7		0xE010031C



	.globl clock_asm_init
clock_asm_init:
	/*set PLL lock time*/
	ldr r0, =APLL_LOCK
	ldr r1, =0xffff
	str r1, [r0]
	ldr r0, =MPLL_LOCK
	ldr r1, =0xffff
	str r1, [r0]
	ldr r0, =EPLL_LOCK
	ldr r1, =0xffff
	str r1, [r0]
	ldr r0, =VPLL_LOCK
	ldr r1, =0xffff
	str r1, [r0]
	
	/* set PLL PMS value, enable PLL*/
	ldr r0, =APLL_CON0
	ldr r1, =(3<<8)|(125<<16)|(1<<0)|(1<<31)	/* FOUT_APLL = 1000MHz */
	str r1, [r0]
	ldr r0, =MPLL_CON
	ldr r1, =(12<<8)|(667<<16)|(1<<0)|(1<<31)	/* FOUT_MPLL = 667MHz */
	str r1, [r0]
	ldr r0, =EPLL_CON0
	ldr r1, =(3<<8)|(48<<16)|(2<<0)|(1<<31)	/* FOUT_EPLL = 96MHz */
	str r1, [r0]
	ldr r0, =VPLL_CON
	ldr r1, =(6<<8)|(108<<16)|(3<<0)|(1<<31)	/* FOUT_VPLL = 54MHz */
	str r1, [r0]
	
	/*wait until PLL lock finish*/
	
	1:
	ldr r0, =APLL_CON0
	ldr r1, [r0]
	ldr r2, =(1<<29)
	and r0, r1, r2
	cmp r0, #0
	beq 1b
	2:
	ldr r0, =MPLL_CON
	ldr r1, [r0]
	ldr r2, =(1<<29)
	and r0, r1, r2
	cmp r0, #0
	beq 2b
	3:
	ldr r0, =EPLL_CON0
	ldr r1, [r0]
	ldr r2, =(1<<29)
	and r0, r1, r2
	cmp r0, #0
	beq 3b
	4:
	ldr r0, =VPLL_CON
	ldr r1, [r0]
	ldr r2, =(1<<29)
	and r0, r1, r2
	cmp r0, #0
	beq 4b
	/*set system clock source, select PLL as clock output*/
	/* MOUT_MSYS = SCLKAPLL = FOUT_APLL = 1000MHz
	   MOUT_DSYS = SCLKMPLL = FOUT_MPLL = 667MHz
  	   MOUT_PSYS = SCLKMPLL = FOUT_MPLL = 667MHz
	   ONENAND = HCLK_PSYS */
	ldr r0, =CLK_SRC0
	ldr r1, =(1<<0)|(1<<4)|(1<<8)|(1<<12)
	str r1, [r0]
	
	/*set system clock divider ratio*/
	ldr r0, =CLK_DIV0
	// APLL_RATIO = 0, freq(ARMCLK) = MOUT_MSYS / (APLL_RATIO + 1) = 1000MHz */
	/* A2M_RATIO = 4, freq(A2M) = SCLKAPLL / (A2M_RATIO + 1) = 200MHz */
	/* HCLK_MSYS_RATIO = 4, freq(HCLK_MSYS) = ARMCLK / (HCLK_MSYS_RATIO + 1) = 200MHz */
	/* PCLK_MSYS_RATIO = 1, freq(PCLK_MSYS) = HCLK_MSYS / (PCLK_MSYS_RATIO + 1) = 100MHz */
	/* HCLK_DSYS_RATIO = 3, freq(HCLK_DSYS) = MOUT_DSYS / (HCLK_DSYS_RATIO + 1) = 166MHz */
	/* PCLK_DSYS_RATIO = 1, freq(PCLK_DSYS) = HCLK_DSYS / (PCLK_DSYS_RATIO + 1) = 83MHz */
	/* HCLK_PSYS_RATIO = 4, freq(HCLK_PSYS) = MOUT_PSYS / (HCLK_PSYS_RATIO + 1) = 133MHz */
	/* PCLK_PSYS_RATIO = 1, freq(PCLK_PSYS) = HCLK_PSYS / (PCLK_PSYS_RATIO + 1) = 66MHz */
	ldr r1, =(0<<0)|(4<<4)|(4<<8)|(1<<12)|(3<<16)|(1<<20)|(4<<24)|(1<<28)
	str r1, [r0]
	
	mov pc, lr
	

