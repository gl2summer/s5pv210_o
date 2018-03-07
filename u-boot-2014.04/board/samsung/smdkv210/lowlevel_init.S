/*
 * Copyright (C) 2009 Samsung Electronics
 * Kyungmin Park <kyungmin.park@samsung.com>
 * Minkyu Kang <mk7.kang@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <version.h>
#include <asm/arch/cpu.h>
#include <asm/arch/power.h>

/*
 * Register usages:
 *
 * r5 has zero always
 */

	.globl lowlevel_init
lowlevel_init:

	mov	r9, lr

	bl led_flash

	/* r5 has always zero */
	mov	r5, #0

	/* Disable Watchdog */
	ldr	r0, =S5PC110_WATCHDOG_BASE		@0xEA200000
	orr	r0, r0, #0x0
	str	r5, [r0]

	/* setting SRAM */
	ldr	r0, =S5PC110_SROMC_BASE
	ldr	r1, =0x9
	str	r1, [r0]

	/* S5PC100 has 3 groups of interrupt sources */
	ldr	r0, =S5PC110_VIC0_BASE			@0xE4000000
	ldr	r1, =S5PC110_VIC1_BASE			@0xE4000000
	ldr	r2, =S5PC110_VIC2_BASE			@0xE4000000

	/* Disable all interrupts (VIC0, VIC1 and VIC2) */
	mvn	r3, #0x0
	str	r3, [r0, #0x14]				@INTENCLEAR
	str	r3, [r1, #0x14]				@INTENCLEAR
	str	r3, [r2, #0x14]				@INTENCLEAR

	/* Set all interrupts as IRQ */
	str	r5, [r0, #0xc]				@INTSELECT
	str	r5, [r1, #0xc]				@INTSELECT
	str	r5, [r2, #0xc]				@INTSELECT

	/* Pending Interrupt Clear */
	str	r5, [r0, #0xf00]			@INTADDRESS
	str	r5, [r1, #0xf00]			@INTADDRESS
	str	r5, [r2, #0xf00]			@INTADDRESS
	

	/* for UART */
	ldr	r0, =S5PC110_GPIO_BASE
	ldr	r1, =0x00000022
	str	r1, [r0, #0x20]			@ GPA1_CON

	/* LED ON */
	ldr	r0, =0xE0200C40 		@GPH2CON
	ldr r1, [r0]
	ldr r2, =(0xFFFF<<16)
	bic r1, r1, r2
	ldr r2, =(0x1111<<16)
	orr r1, r1, r2
	str	r1, [r0]
	
	ldr	r0, =0xE0200C44			@GPH2DAT
	ldr r1, [r0]
	orr r1, r1, #0xFF
	str	r1, [r0]
	
	/* for TZPC */
	/* bl tzpc_asm_init */

	mov	lr, r9
	mov	pc, lr





/* 
 * for debug indication
 */
led_flash:
	ldr r5, =10
	/* LED */
	ldr	r0, =0xE0200C40 		@GPH2CON
	ldr r1, [r0]
	ldr r2, =(0xFFFF<<16)
	bic r1, r1, r2
	ldr r2, =(0x1111<<16)
	orr r1, r1, r2
	str	r1, [r0]

flash:
	ldr	r0, =0xE0200C44			@GPH2DAT
	ldr r1, [r0]
	bic r1, r1, #0xFF
	str	r1, [r0]

	ldr r3, =20000000
1:
	sub r3, r3, #1
	cmp r3, #0
	bne 1b

	ldr	r0, =0xE0200C44			@GPH2DAT
	ldr r1, [r0]
	orr r1, r1, #0xFF
	str	r1, [r0]
	
	ldr r3, =20000000
2:
	sub r3, r3, #1
	cmp r3, #0
	bne 2b

	sub r5, r5, #1
	cmp r5, #0
	bne flash
	
	mov	pc, lr


/*
 * tzpc_asm_init: Initialize TZPC
 */
tzpc_asm_init:
	ldr	r0, =0xE3800000
	mov	r1, #0x0
	str	r1, [r0]
	mov	r1, #0xff
	str	r1, [r0, #0x804]
	str	r1, [r0, #0x810]

	ldr	r0, =0xE2800000
	str	r1, [r0, #0x804]
	str	r1, [r0, #0x810]
	str	r1, [r0, #0x81C]

	ldr	r0, =0xE2900000
	str	r1, [r0, #0x804]
	str	r1, [r0, #0x810]

	mov	pc, lr

