#include <config.h>
#include <version.h>
#include <asm/arch/cpu.h>
#include <asm/arch/power.h>


	.globl lowlevel_init_spl
lowlevel_init_spl:
	mov	r9, lr
	
	bl clock_asm_init
	bl sdram_asm_init

	mov	lr, r9
	mov	pc, lr

