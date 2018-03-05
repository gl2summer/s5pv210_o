#include <stddef.h>
#include "xmodem.h"


XMODEM xmodem;

void xmInit(xModemPutData put, xModemGetData get, xModemCallback cb)
{
	xmodem.putData = put;
	xmodem.getData = get;
	
	xmodem.xmCallback = cb;
	
	xmodem.xmState = XMSTA_STOP;
}

void xmStart(void)
{
	xmodem.xmState = XMSTA_ENTRY;
}


static unsigned short calcCrc(unsigned char *start, unsigned char length)
{
	unsigned short crc = 0x00;
	unsigned char  *data=start;
	unsigned char i;

	for (i = 0; i < length; i++)
	{
		crc  = (crc >> 8) | (crc << 8);
		crc ^= data[i];
		crc ^= (crc & 0xff) >> 4;
		crc ^= crc << 12;
		crc ^= (crc & 0xff) << 5;
	}
	return crc;
}

static bool checkSohFrame(unsigned char *buff, unsigned char len)
{
	if(buff == NULL)return false;
	if(len < XMODEM_SOH_FRAME_SIZE)return false;	
	if(buff[IDX_FRAME] != XM_FRAME_SOH)
	{
		//printf("not soh\r\n");
		return false;
	}
	if((unsigned char)(buff[IDX_PKT_NUM] ^ buff[IDX_PKT_NUM_A]) != 0xff)
	{
		//printf("sn error\r\n");
		return false;
	}
	unsigned short crc_cal = calcCrc(&buff[IDX_DATAS], XMODEM_DATA_SIZE);
	unsigned short pkt_cal = ((unsigned short)buff[IDX_CHKSUM] << 8) | buff[IDX_CHKSUM+1];
	if(crc_cal != pkt_cal)
	{
		//printf("crc error\r\n");
		return false;
	}
	
	return true;
}

static bool dealSohFrame(void)
{
	if(xmodem.getData == NULL)return false;
	unsigned int data_len = sizeof(xmodem.frame)-xmodem.frame_offset;
	if(xmodem.getData(xmodem.frame+xmodem.frame_offset, &data_len))//查询是否有数据
	{
		xmodem.frame_offset += data_len;
		if(checkSohFrame(xmodem.frame, xmodem.frame_offset)) //检查接收到的数据，成功则开始开始传输数据
		{
			if(xmodem.xmCallback != NULL)
			{
				xmodem.xmCallback(XMEVT_SOH, xmodem.frame[IDX_PKT_NUM], &xmodem.frame[IDX_DATAS], XMODEM_DATA_SIZE);
			}
			xmodem.frame_offset = 0;
			return true;
		}
	}
	return false;
}

static void dealSohFrameTimeout(void)
{
	xmodem.frame_offset = 0;
}

static unsigned char dealExceptionFrame(void)
{
	if(xmodem.getData == NULL)return XM_FRAME_UNKOWN;
	
	unsigned int data_len = sizeof(xmodem.frame)-xmodem.frame_offset;
	if(xmodem.getData(xmodem.frame+xmodem.frame_offset, &data_len))//查询是否有数据
	{
		xmodem.frame_offset += data_len;
		if(xmodem.frame_offset == 1)
		{
			if(xmodem.frame[IDX_FRAME]==XM_FRAME_EOT)
			{
				if(xmodem.xmCallback != NULL)
				{
					xmodem.xmCallback(XMEVT_EOT, 0, NULL, 0);
				}
				xmodem.frame_offset = 0;
				return XM_FRAME_EOT;
			}
			else if(xmodem.frame[IDX_FRAME]==XM_FRAME_CAN)
			{
				if(xmodem.xmCallback != NULL)
				{
					xmodem.xmCallback(XMEVT_CAN, 0, NULL, 0);
				}
				xmodem.frame_offset = 0;
				return XM_FRAME_CAN;
			}
			return XM_FRAME_UNKOWN;
		}
	}
	return XM_FRAME_UNKOWN;
}

static void dealExit(void)
{
}


int xModemFSM(int timestamp)
{
	switch(xmodem.xmState)
	{
		case XMSTA_ENTRY:
			xmodem.ncg_send_retry_times = 0;
			xmodem.timestamp_ncg_sent_prev = 0;
			
			xmodem.soh_nak_retry_times = 0;
			xmodem.timestamp_soh_recv_prev = 0;
			
			xmodem.frame_offset = 0;
			
			if(xmodem.xmCallback != NULL)
			{
				xmodem.xmCallback(XMEVT_ENTRY, 0, NULL, 0);
			}
			
			xmodem.xmState = XMSTA_WAIT_TRANS;
		break;
		
		case XMSTA_WAIT_TRANS:
			if(dealSohFrame())//查询是否有数据
			{
				if(xmodem.putData != NULL)
				{
					unsigned char ack = XM_FRAME_ACK;
					xmodem.putData(&ack, 1);
				}
				xmodem.soh_nak_retry_times = 0;
				xmodem.timestamp_soh_recv_prev = timestamp;
				xmodem.xmState = XMSTA_TRANSMIT;
			}
			else
			{
				//未收到开始传输的数据，超时发送握手信号
				if(timestamp - xmodem.timestamp_ncg_sent_prev >= XM_NCG_RESEND_DLY)
				{
					if(xmodem.ncg_send_retry_times++ > 100)
					{
						xmodem.xmState = XMSTA_EXIT;
						break;
					}
					
					dealSohFrameTimeout();
					xmodem.timestamp_ncg_sent_prev = timestamp;
					
					if(xmodem.putData != NULL)
					{
						unsigned char ncg = XM_FRAME_NCG;
						xmodem.putData(&ncg, 1);
					}
				}
			}
		break;
		
		case XMSTA_TRANSMIT:
			if(dealExceptionFrame()==XM_FRAME_EOT) //完成
			{
				xmodem.xmState = XMSTA_EXIT;
			}
			else if(dealExceptionFrame()==XM_FRAME_CAN) //取消
			{
				xmodem.xmState = XMSTA_EXIT;
			}
			else if(dealSohFrame()) //查询是否有数据
			{
				if(xmodem.putData != NULL)
				{
					unsigned char ack = XM_FRAME_ACK;
					xmodem.putData(&ack, 1);
				}
				xmodem.soh_nak_retry_times = 0;
				xmodem.timestamp_soh_recv_prev = timestamp;
			}
			else
			{
				//未收到开始传输的数据，超时发送握手信号
				if(timestamp - xmodem.timestamp_soh_recv_prev >= XM_SOH_RECV_TIMEOUT)
				{
					dealSohFrameTimeout();
					
					if(xmodem.soh_nak_retry_times++ > 50)
					{
						xmodem.xmState = XMSTA_EXIT;
						break;
					}
					
					xmodem.timestamp_soh_recv_prev = timestamp;
					
					if(xmodem.putData != NULL)
					{
						unsigned char nak = XM_FRAME_NAK;
						xmodem.putData(&nak, 1);
					}
				}
			}
		break;
		
		case XMSTA_EXIT:
		dealExit();
		xmodem.xmState = XMSTA_STOP;
		break;
		
		case XMSTA_STOP:
		break;
		
		default:
		break;
	}
	
	return xmodem.xmState;
}






