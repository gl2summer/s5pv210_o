#ifndef _UART_H__
#define _UART_H__


#include <stdbool.h>
#include "types.h"


#define GPA1CON		*((volatile unsigned int *)0xE0200020)
#define ULCON2 		*((volatile unsigned int *)0xE2900800)
#define UCON2 		*((volatile unsigned int *)0xE2900804)
#define UFCON2 		*((volatile unsigned int *)0xE2900808)
#define UTRSTAT2 	*((volatile unsigned int *)0xE2900810)
#define UTXH2  		*((volatile unsigned int *)0xE2900820)
#define URXH2 		*((volatile unsigned int *)0xE2900824)
#define UBRDIV2 	*((volatile unsigned int *)0xE2900828)
#define UDIVSLOT2	*((volatile unsigned int *)0xE290082C)
#define UINTP2		*((volatile unsigned int *)0xE2900830)
#define UINTSP2		*((volatile unsigned int *)0xE2900834)
#define UINTM2		*((volatile unsigned int *)0xE2900838)



/* UART2初始化 */
void uart_init(void);
char get_char(void);
bool try_get_char(char *c);
void put_char(char c);
void put_chars(char *s, unsigned int len);
/* 打印字符串 */
void put_string(char *str);
/* 格式化输出到终端 */
void uart_printf(const char *fmt, ...);


#endif // _UART_H__

