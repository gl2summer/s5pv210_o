#include <stdbool.h>
#include "uart.h"
#include "clock_c.h"
#include "led.h"
#include "tim4.h"
#include "nand.h"
#include "xmodem.h"
#include "myboot_cfg.h"

unsigned char nand_buff[PAGE_SIZE];
unsigned int buff_offset;
unsigned int nand_address;

unsigned char uart_buf[128];
unsigned char buf_len;




void program_tonand(void)
{
	if(!(nand_address % BLOCK_SIZE))
	{
		nand_erase(nand_address);
	}
	nand_write_page_8bit(nand_buff, nand_address);
	nand_address += PAGE_SIZE;
}

void try_program(unsigned char *buffer, unsigned int buffer_len, bool bfinish)
{
	if(bfinish)
	{
		program_tonand();
		uart_printf("finished!\n");
		return ;
	}
	
	int i;
	for(i=0; i<buffer_len; i++)
	{
		nand_buff[buff_offset] = buffer[i];
		buff_offset++;
		if(buff_offset >= PAGE_SIZE)
		{
			program_tonand();
			buff_offset = 0;
		}
	}
}


void uart_buff(char c)
{
	if(buf_len < sizeof(uart_buf))
	{
		uart_buf[buf_len] = c;
		buf_len++;
	}
}

void xmCallback(XMODEM_EVT evt, unsigned int data, unsigned char *buffer, unsigned int buffer_len)
{
	if(evt==XMEVT_ENTRY)
	{
		buff_offset = 0;
	}
	else if(evt==XMEVT_SOH)
	{
		try_program(buffer, buffer_len, false);
	}
	else if(evt==XMEVT_EOT)
	{
		try_program(buffer, buffer_len, true);
	}
}

bool xmPutData(unsigned char* buffer, unsigned int buffer_len)
{
	put_chars((char*)buffer, buffer_len);
	return true;
}

bool xmGetData(unsigned char *buffer, unsigned int *buffer_len)
{
	unsigned char i;
	for(i=0; i<buf_len; i++)
	{
		if(buffer)
			buffer[i] = uart_buf[i];
	}
	if(buffer_len)
		*buffer_len = buf_len;
	buf_len = 0;
	
	return true;
}


//base_addr: flash存储的起始地址
void doXmodemDownloadInit(u32 base_addr)
{
	xmInit(xmPutData, xmGetData, xmCallback);
	
	uart_printf("please send your file: \n");
	
	xmStart();
	nand_address = base_addr;
}

/*return true when exit*/
bool doXmodemDownloading(unsigned long timestamp)
{
	return (xModemFSM(timestamp)==XMSTA_STOP);
}



typedef enum{MENU_ENTRY=0, MENU_AUTO_BOOT, MENU_CMD, MENU_DOWNLOAD, MENU_BOOT}MENU_STATE;
#define AUTOBOOT_TIMEOUTS 3


void menu(unsigned long timestamp, bool recved, char c)
{
	static MENU_STATE menu_state = MENU_ENTRY;
	static unsigned long timestamp_auto_boot = 0;
	static int auto_boot_timeout = AUTOBOOT_TIMEOUTS;
	
	switch(menu_state)
	{
		case MENU_AUTO_BOOT:
			if(recved)
			{
				uart_printf("\r\n\r\n---------------- boot menu, please input your command: ----------------\r\n");
				uart_printf(">> D: download myboot(spl+uboot) into nandflash(uart2, xModem, 0)\r\n");
				uart_printf(">> B: start uboot\r\n");
				uart_printf("\r\n");
				menu_state = MENU_CMD;
				break;
			}
		
			if(auto_boot_timeout <= 0)
			{
				menu_state = MENU_BOOT;
				break;
			}
			
			if(timestamp_auto_boot==0)
			{
				timestamp_auto_boot = timestamp;
				uart_printf("\rauto boot in %d second...", auto_boot_timeout);
			}
			if(timestamp - timestamp_auto_boot >= 1000)
			{
				auto_boot_timeout--;
				timestamp_auto_boot = timestamp;
				uart_printf("\rauto boot in %d second...", auto_boot_timeout);
			}
			break;
			
		case MENU_CMD:
			switch(c)
			{
				case 'd':
				case 'D':
					doXmodemDownloadInit(BL1_NF_ADDR);
					menu_state = MENU_DOWNLOAD;
				break;
				
				case 'b':
				case 'B':
					menu_state = MENU_BOOT;
				break;
				
				default:
				break;
			}
			break;
		
		case MENU_DOWNLOAD:
			if(recved)
				uart_buff(c);
			if(doXmodemDownloading(timestamp))
				menu_state = MENU_CMD;
			break;
		
		case MENU_BOOT:
			uart_printf("\r\nbooting...\r\n");
			NF8_ReadPage_8ECC(BL2_NF_ADDR, (unsigned char *)BL2_RAM_ADDR);
			((BL2_APP)((u32 *)BL2_RAM_ADDR))();
			while(1);
			break;
		
		case MENU_ENTRY:
		default:
			uart_printf("Going to auto boot, press any to cancel!\r\n");
			menu_state = MENU_AUTO_BOOT;
			timestamp_auto_boot = 0;
			auto_boot_timeout = AUTOBOOT_TIMEOUTS;
			break;
	}
}




void board_init_f_spl(void)
{
	uart_init();
	nand_init();
	tim4_init();
	
	print_clock();
	
	//delay(100);

	while(1)
	{
		char c;
		bool recved = try_get_char(&c);
		unsigned long timestamp = tim4_poll();
		menu(timestamp, recved, c);
	}
}
