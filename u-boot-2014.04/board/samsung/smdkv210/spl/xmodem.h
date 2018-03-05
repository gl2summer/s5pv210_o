#ifndef _XMODEM_H_
#define _XMODEM_H_


#include "stdbool.h"


#define XM_FRAME_SOH			0x01 //开始
#define XM_FRAME_EOT			0x04 //发送完成
#define XM_FRAME_ACK			0x06 //成功
#define XM_FRAME_NAK			0x15 //失败，要求重发
#define XM_FRAME_CAN			0x18 //无条件终止传输
#define XM_FRAME_NCG			0x43 //发送表示等待接收
#define XM_FRAME_UNKOWN			0xff //未知数据帧类型

#define XMODEM_DATA_SIZE		(128)
#define XMODEM_SOH_FRAME_SIZE	(XMODEM_DATA_SIZE + 5)

#define XM_NCG_RESEND_DLY	1000 //单位：毫秒
#define XM_SOH_RECV_TIMEOUT	1000 //单位：毫秒

typedef enum
{
	IDX_FRAME=0,
	IDX_PKT_NUM=1,
	IDX_PKT_NUM_A=2,
	IDX_DATAS=3,
	IDX_CHKSUM=131,
}SOH_IDX;




typedef enum
{
	XMEVT_ENTRY=0,
	XMEVT_SOH,
	XMEVT_EOT,
	XMEVT_ACK,
	XMEVT_NAK,
	XMEVT_CAN,
	XMEVT_NCG,
	XMEVT_UNKOWN,
}XMODEM_EVT;

typedef enum
{
	XMSTA_ENTRY=0,
	XMSTA_WAIT_TRANS, //等待传输的开始
	XMSTA_TRANSMIT, //传输中
	XMSTA_EXIT, //退出传输（完成、中断）
	XMSTA_STOP,
}XMODEM_STATE;

typedef void (*xModemCallback)(XMODEM_EVT evt, unsigned int data, unsigned char *buffer, unsigned int buffer_len);
typedef bool (*xModemPutData)(unsigned char *buffer, unsigned int buffer_len);
typedef bool (*xModemGetData)(unsigned char *buffer, unsigned int *buffer_len);


typedef struct
{
	xModemPutData putData;
	xModemGetData getData;
	
	xModemCallback	xmCallback;
	
	XMODEM_STATE	xmState;
	
	int timestamp_ncg_sent_prev;
	int ncg_send_retry_times;
	
	int timestamp_soh_recv_prev;
	int soh_nak_retry_times;
	
	unsigned char frame_offset;
	unsigned char frame[XMODEM_SOH_FRAME_SIZE];
}XMODEM;



void xmInit(xModemPutData put, xModemGetData get, xModemCallback cb);
void xmStart(void);
int xModemFSM(int timestamp);


#endif //_XMODEM_H_

