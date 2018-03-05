#ifndef _NAND_H__
#define _NAND_H__



#define MP0_1CON  		(*(volatile u32 *)0xE02002E0)
#define	MP0_3CON  		(*(volatile u32 *)0xE0200320)
#define	MP0_6CON 		(*(volatile u32 *)0xE0200380)

#define	NFCONF  		(*(volatile u32 *)0xB0E00000) 
#define	NFCONT  		(*(volatile u32 *)0xB0E00004) 	
#define	NFCMMD  		(*(volatile u32 *)0xB0E00008) 
#define	NFADDR  		(*(volatile u32 *)0xB0E0000C)
#define	NFDATA  		(*(volatile u8 *)0xB0E00010)
#define	NFSTAT  		(*(volatile u32 *)0xB0E00028)

#define	NFMECCD0  		(*(volatile u32 *)0xB0E00014)
#define	NFMECCD1  		(*(volatile u32 *)0xB0E00018)
#define	NFECCPRGECC0 	(*(volatile u32 *)0xB0E20090)
#define	NFECCPRGECC1 	(*(volatile u32 *)0xB0E20094)
#define	NFECCPRGECC2 	(*(volatile u32 *)0xB0E20098)
#define	NFECCPRGECC3 	(*(volatile u32 *)0xB0E2009C)
#define	NFECCCONF  		(*(volatile u32 *)0xB0E20000)
#define	NFECCCONT  		(*(volatile u32 *)0xB0E20020)
#define	NFECCSTAT  		(*(volatile u32 *)0xB0E20030)

#define NFECCSECSTAT	(*(volatile u32 *)0xB0E20040)
#define NFECCERL0		(*(volatile u32 *)0xB0E200C0)
#define NFECCERL1		(*(volatile u32 *)0xB0E200C4)
#define NFECCERL2		(*(volatile u32 *)0xB0E200C8)
#define NFECCERL3		(*(volatile u32 *)0xB0E200CC)
#define NFECCERP0		(*(volatile u32 *)0xB0E200F0)
#define NFECCERP1		(*(volatile u32 *)0xB0E200F4)
#define NFECCERP2		(*(volatile u32 *)0xB0E200F8)
#define NFECCERP3		(*(volatile u32 *)0xB0E200FC)

#define OOB_SIZE		64
#define PAGE_SIZE		2048
#define PAGE_PER_BLK	64
#define BLOCK_SIZE		(PAGE_SIZE * (PAGE_PER_BLK))

#define WRITE_ECC		0
#define READ_ECC		1

#define OOB_ECC_OFFSET		12
#define ECC_SIZE		512
#define ECC_BYTES		13


#define NF8_ReadPage_Adv(a,b,c) (((int(*)(u32, u32, u8*))(*((u32 *)0xD0037F90)))(a,b,c))

#define LOG(...)



/* NAND初始化 */
void nand_init(void);
/* 读NAND ID */
void nand_read_id(u8 id[]);
/* 擦除一个块 */
void nand_erase(u32 addr);
/* 随机读:从任意地址读任意字节的数据 */
void nand_read_random(u8 *buf, u32 addr, u32 size);
void nand_read_oob(u8 *buf, u32 addr, u32 size);
/* 写一页数据 */
void nand_write_page(u8 *buf, u32 addr, u8 *oob);
/* 写一页数据，同时将Ecc Code写入OOB区 */
void nand_write_page_8bit(u8 *buf, u32 addr);
int nand_read_page_8bit(u8 *buf, u32 addr);
/*
** 三星提供的固化在iROM里的函数
** 使用8位硬件ECC读取1页数据
** 成功返回0，否则返回1
*/
int NF8_ReadPage_8ECC(u32 addr, u8 *buf);
#endif //_NAND_H__
