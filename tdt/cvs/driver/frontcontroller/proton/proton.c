/*
 * proton.c
 */

/*
 * HL101 frontpanel driver.
 *
 * Devices:
 *    - /dev/vfd (vfd ioctls and read/write function)
 *    - /dev/rc  (reading of key events)
 *
 */

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/termbits.h>
#include <linux/kthread.h>
#include <linux/version.h>
#include <linux/input.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/poll.h>
#include <linux/workqueue.h>
#include <linux/stm/pio.h>

#include "proton.h"
#include "encoding.h"

#include <linux/device.h> /* class_creatre */
#include <linux/cdev.h> /* cdev_init */

static short paramDebug = 0;
#define TAGDEBUG "[proton] "

#define dprintk(level, x...) do { \
if ((paramDebug) && (paramDebug > level)) printk(TAGDEBUG x); \
} while (0)

#define INVALID_KEY    -1
#define LOG_OFF         0
#define LOG_ON          1

#define NO_KEY_PRESS   -1
#define KEY_PRESS_DOWN  1
#define KEY_PRESS_UP    0

#define REC_NEW_KEY    34
#define REC_NO_KEY      0
#define REC_REPEAT_KEY  2

static int sec_0 = 0;
static int gmt_on = 0;
static int time_on = 0;

typedef struct
{
	struct file*      fp;
	int              read;
	struct semaphore sem;

} tFrontPanelOpen;

#define FRONTPANEL_MINOR_RC 1
#define LASTMINOR           2

static tFrontPanelOpen FrontPanelOpen [LASTMINOR];

typedef enum VFDMode_e{
	VFDWRITEMODE,
	VFDREADMODE
}VFDMode_T;

typedef enum SegNum_e{
	SEGNUM1 = 0,
	SEGNUM2
}SegNum_T;

typedef struct SegAddrVal_s{
	unsigned char Segaddr1;
	unsigned char Segaddr2;
	unsigned char CurrValue1;
	unsigned char CurrValue2;
}SegAddrVal_T;

typedef enum PIO_Mode_e
{
	PIO_Out,
	PIO_In
}PIO_Mode_T;

struct VFD_config
{
	struct stpio_pin*    clk;
	struct stpio_pin*    data;
	struct stpio_pin*    cs;
	int data_pin[2];
	int clk_pin[2];
	int cs_pin[2];
};

struct VFD_config cfg;

#define BUFFERSIZE 256

struct transmit_s
{
	unsigned char     buffer[BUFFERSIZE];
	int        len;
	int        needAck;
	int        ack_len;
	int        ack_len_header;
	unsigned char     ack_buffer[BUFFERSIZE];

	int requeueCount;
};

#define cMaxTransQueue    100

struct transmit_s transmit[cMaxTransQueue];

struct receive_s
{
	int len;
	unsigned char buffer[BUFFERSIZE];
};

#define cMaxReceiveQueue    100
static wait_queue_head_t   wq;

struct receive_s receive[cMaxReceiveQueue];

static int receiveCount = 0;

#define cMaxAckAttempts 150
#define cMaxQueueCount    5

struct semaphore        write_sem;
struct semaphore        rx_int_sem; 
struct semaphore        transmit_sem;
struct semaphore        receive_sem;
struct semaphore        key_mutex;
static struct semaphore    cs_sem;
static struct semaphore    display_sem;

struct saved_data_s
{
	int  length;
	char data[BUFFERSIZE];
};

static struct saved_data_s lastdata;

int writePosition = 0;
int readPosition = 0;
unsigned char receivedData[BUFFERSIZE];

#define VFD_CS_CLR() {down(&cs_sem);udelay(5);while(stpio_get_pin(cfg.cs)==0) udelay(10); stpio_set_pin(cfg.cs, 0);}
#define VFD_CS_SET() {stpio_set_pin(cfg.cs, 1);udelay(10);up(&cs_sem);}

unsigned char str[64];

static SegAddrVal_T VfdSegAddr[15];

struct semaphore vfd_sem;
struct rw_semaphore vfd_rws;

static int PROTONfp_Set_PIO_Mode(PIO_Mode_T Mode_PIO)
{
	if(Mode_PIO == PIO_Out)
		stpio_configure_pin(cfg.data, STPIO_OUT);
	else if(Mode_PIO == PIO_In)
		stpio_configure_pin(cfg.data, STPIO_IN);
	stpio_configure_pin(cfg.clk, STPIO_OUT);
	stpio_configure_pin(cfg.cs,  STPIO_OUT);
	return 0;
}

unsigned char PROTONfp_RD(void)
{
	int i;
	unsigned char val = 0, data = 0;

	down_read(&vfd_rws);

	PROTONfp_Set_PIO_Mode(PIO_In);
	for (i = 0; i < 8; i++)
	{
		val >>= 1;
		stpio_set_pin(cfg.clk, 0);
		udelay(1);
		data = stpio_get_pin(cfg.data);
		stpio_set_pin(cfg.clk, 1);
		if(data)
			val |= 0x80;
		stpio_set_pin(cfg.clk, 1);
		udelay(1);
	}
	udelay(1);
	PROTONfp_Set_PIO_Mode(PIO_Out);
	up_read(&vfd_rws);

	return val;
}

static int VFD_WR(unsigned char data)
{
	int i;
	down_write(&vfd_rws);
	for(i = 0; i < 8; i++)
	{
		stpio_set_pin(cfg.clk, 0);
		if(data & 0x01)
			stpio_set_pin(cfg.data, 1);
		else
			stpio_set_pin(cfg.data, 0);
		stpio_set_pin(cfg.clk, 1);
		data >>= 1;
	}
	up_write(&vfd_rws);
	return 0;
}

void VFD_Seg_Addr_Init(void)
{
	unsigned char i, addr = 0xC0;
	for(i = 0; i < 13; i++)
	{
		VfdSegAddr[i + 1].CurrValue1 = 0;
		VfdSegAddr[i + 1].CurrValue2 = 0;
		VfdSegAddr[i + 1].Segaddr1 = addr;
		VfdSegAddr[i + 1].Segaddr2 = addr + 1;
		addr += 3;
	}
}

static int VFD_Seg_Dig_Seg(unsigned char dignum, SegNum_T segnum, unsigned char val)
{
	int  res = 0;
	unsigned char  addr=0;
	if(segnum < 0 && segnum > 1)
		res = -EINVAL;

	VFD_CS_CLR();
	if(segnum == SEGNUM1)
	{
		addr = VfdSegAddr[dignum].Segaddr1;
		VfdSegAddr[dignum].CurrValue1 = val ;
	}
	else if(segnum == SEGNUM2)
	{
		addr = VfdSegAddr[dignum].Segaddr2;
		VfdSegAddr[dignum].CurrValue2 = val ;
	}
	res = VFD_WR(addr);
	udelay(1);
	res = VFD_WR(val);
	VFD_CS_SET();
	return res;
}

static int VFD_Set_Mode(VFDMode_T mode)
{
	int res = 0;
	unsigned char data = 0;

	if(mode == VFDWRITEMODE)
	{
		data = 0x44;
		VFD_CS_CLR();
		res = VFD_WR(data);
		VFD_CS_SET();
	}
	else if(mode == VFDREADMODE)
	{
		data = 0x46;
		res = VFD_WR(data);
		udelay(1);
	}
	return res;
}

static int VFD_Show_Content(void)
{
	int res = 0;
	VFD_CS_CLR();
	res = VFD_WR(0x8F);
	VFD_CS_SET();
	return res;
}

static int VFD_Show_Content_Off(void)
{
	int res = 0;
	res = VFD_WR(0x87);
	return res;
}

void VFD_Clear_All(void)
{
	int i;
	for(i = 0; i < 13; i++)
	{
		VFD_Seg_Dig_Seg(i + 1,SEGNUM1,0x00);
		VfdSegAddr[i + 1].CurrValue1 = 0x00;
		VFD_Seg_Dig_Seg(i + 1,SEGNUM2,0x00);
		VfdSegAddr[i + 1].CurrValue2 = 0;
	}
	sec_0 = 0;
}

void VFD_Draw_Num(unsigned char c, unsigned char position)
{
	int dignum;

	if(position < 1 || position > 4)
	{
		dprintk(2, "num position error! %d\n", position);
		return;
	}
	if(c > 9)
	{
		dprintk(2, "unknown num!\n");
		return;
	}
	dignum =10 - position / 3;
	if(position % 2 == 1)
	{
		if(NumLib[c][1] & 0x01)
			VFD_Seg_Dig_Seg(dignum, SEGNUM1, VfdSegAddr[dignum].CurrValue1 | 0x80);
		else
			VFD_Seg_Dig_Seg(dignum, SEGNUM1, VfdSegAddr[dignum].CurrValue1 & 0x7F);
		VfdSegAddr[dignum].CurrValue2 = VfdSegAddr[dignum].CurrValue2 & 0x40;
		VFD_Seg_Dig_Seg(dignum, SEGNUM2, (NumLib[c][1] >> 1) | VfdSegAddr[dignum].CurrValue2);
	}
	else if(position % 2 == 0)
	{
		if((NumLib[c][0] & 0x01))
			VFD_Seg_Dig_Seg(dignum, SEGNUM2, VfdSegAddr[dignum].CurrValue2 | 0x40);
		else
			VFD_Seg_Dig_Seg(dignum, SEGNUM2, VfdSegAddr[dignum].CurrValue2 & 0x3F);
		VfdSegAddr[dignum].CurrValue1 = VfdSegAddr[dignum].CurrValue1 & 0x80;
		VFD_Seg_Dig_Seg(dignum, SEGNUM1, (NumLib[c][0] >>1 ) | VfdSegAddr[dignum].CurrValue1 );
	}
}

static int VFD_Show_Time(int hh, int mm)
{
	int res = 0;
	if (down_interruptible(&vfd_sem))
	{
		dprintk(2, "%s bad parameter!\n", __func__);
		res =-EBUSY;
		return res;
	}
	if( (hh > 24) && (mm > 60))
		res = -EINVAL ;

	VFD_Draw_Num((hh/10), 1);
	VFD_Draw_Num((hh%10), 2);
	VFD_Draw_Num((mm/10), 3);
	VFD_Draw_Num((mm%10), 4);
	up(&vfd_sem);
	return 0;
}

static int VFD_Show_Ico(LogNum_T log_num, int log_stat)
{
	int res = 0 ;
	int dig_num = 0,seg_num = 0;
	SegNum_T seg_part = 0;
	u8	seg_offset = 0;
	u8	addr = 0,val = 0;

	if (down_interruptible(&vfd_sem))
	{
		res =-EBUSY;
		return res;
	}

	if(log_num >= LogNum_Max)
	{
		res = -EINVAL ;
		dprintk(2, "%s bad parameter!\n", __func__);
		return res;
	}
	dig_num = log_num/16;
	seg_num = log_num%16;
	seg_part = seg_num/9;

	VFD_CS_CLR();
	if(seg_part == SEGNUM1)
	{
		seg_offset = 0x01 << ((seg_num%9) - 1);
		addr = VfdSegAddr[dig_num].Segaddr1;
		if(log_stat == LOG_ON)
			VfdSegAddr[dig_num].CurrValue1 |= seg_offset;
		if(log_stat == LOG_OFF)
			VfdSegAddr[dig_num].CurrValue1 &= (0xFF-seg_offset);
		val = VfdSegAddr[dig_num].CurrValue1 ;
	}
	else if(seg_part == SEGNUM2)
	{
		seg_offset = 0x01 << ((seg_num%8) - 1);
		addr = VfdSegAddr[dig_num].Segaddr2;
		if(log_stat == LOG_ON)
			VfdSegAddr[dig_num].CurrValue2 |= seg_offset;
		if(log_stat == LOG_OFF)
			VfdSegAddr[dig_num].CurrValue2 &= (0xFF-seg_offset);
		val = VfdSegAddr[dig_num].CurrValue2 ;
	}
	res = VFD_WR(addr);
	udelay(10);
	res = VFD_WR(val);
	VFD_CS_SET();
	up(&vfd_sem);
	return res;
}

static struct task_struct *thread; 
static struct task_struct *time_thread;
static int thread_stop  = 1;

void clear_display(void)
{
	int j;
	for(j=0;j<8;j++)
	{
		VFD_Seg_Dig_Seg(j+1, SEGNUM1, 0x00);
		VFD_Seg_Dig_Seg(j+1, SEGNUM2, 0x00);
	}
}

void draw_thread(void *arg)
{
	struct vfd_ioctl_data *data;
	struct vfd_ioctl_data draw_data;
	int count = 0; 
	int pos = 0;
	int k = 0;
	int j = 0;
	unsigned char c0;
	unsigned char c1;
	unsigned char temp;
	unsigned char draw_buf[64][2];

	data = (struct vfd_ioctl_data *)arg;

	draw_data.length = data->length;
	memcpy(draw_data.data,data->data,data->length);

	thread_stop = 0;


	while(pos < draw_data.length)
	{
		if(kthread_should_stop())
		{
			thread_stop = 1;
			return;
		}
		c0 = c1 = temp = 0;
		if(draw_data.data[pos] == 32)
		{
			k++;
			if(k==3)
			{
				count -= 2;
				break;
			}
		}
		else
			k = 0;
		if (draw_data.data[pos] < 0x80)
		{
			temp = draw_data.data[pos];
			if(temp >= 65 && temp <= 95)
				temp = temp - 65;
			else if(temp >= 97 && temp <= 122)
				temp = temp - 97;
			else if(temp >= 40 && temp <= 57)
				temp = temp - 9;
			else if(temp == 32)
				temp = 49;
			if(temp < 50)
			{
				c0 = ASCII[temp][0];
				c1 = ASCII[temp][1];
			}
		}
		else 
			if (draw_data.data[pos] < 0xE0)
			{
				pos++;
				switch (draw_data.data[pos-1])
				{
				case 0xc2:
					c0 = UTF_C2[draw_data.data[pos] & 0x00][0];
					c1 = UTF_C2[draw_data.data[pos] & 0x00][1];
				break;
				case 0xc3:
					c0 = UTF_C3[draw_data.data[pos] & 0x00][0];
					c1 = UTF_C3[draw_data.data[pos] & 0x00][1];
				break;
				case 0xc4:
					c0 = UTF_C4[draw_data.data[pos] & 0x00][0];
					c1 = UTF_C4[draw_data.data[pos] & 0x00][1];
				break;
				case 0xc5:
					c0 = UTF_C5[draw_data.data[pos] & 0x00][0];
					c1 = UTF_C5[draw_data.data[pos] & 0x00][1];
				break;
				case 0xd0:
					c0 = UTF_D0[draw_data.data[pos] & 0x3f][0];
					c1 = UTF_D0[draw_data.data[pos] & 0x3f][1];
				break;
				case 0xd1:
					c0 = UTF_D1[draw_data.data[pos] & 0x3f][0];
					c1 = UTF_D1[draw_data.data[pos] & 0x3f][1];
				break;
				}
			}
			else {
				if (draw_data.data[pos] < 0xF0)
					pos+=2;
				else if (draw_data.data[pos] < 0xF8)
					pos+=3;
				else if (draw_data.data[pos] < 0xFC)
					pos+=4;
				else
					pos+=5;
				}
			draw_buf[count][0] = c0;
			draw_buf[count][1] = c1;
			count++;
 		pos++;
	}
	if(count > 8)
	{
		pos  = 0;
		while(pos < count)
		{
			if(kthread_should_stop())
			{
				thread_stop = 1;
				return;
			}
			k =8;
			if(count-pos < 8 )
				k = count-pos;
			clear_display();
			for(j=0;j<k;j++)
			{
				VFD_Seg_Dig_Seg(j+1, SEGNUM1, draw_buf[pos+j][0]);
				VFD_Seg_Dig_Seg(j+1, SEGNUM2, draw_buf[pos+j][1]);
			}
			msleep(200);
			pos++;
		}
	}
	if(count > 0)
	{
		k =8;
		if(count < 8 )
			k = count;
		if(kthread_should_stop())
		{
			thread_stop = 1;
			return;
		}
		clear_display();
		for(j=0;j<k;j++)
		{
			VFD_Seg_Dig_Seg(j+1, SEGNUM1, draw_buf[j][0]);
			VFD_Seg_Dig_Seg(j+1, SEGNUM2, draw_buf[j][1]);
		}
	}
	
	if(count == 0)
		clear_display();
	thread_stop = 1;
}

#define LEAPYEAR(year) (!((year) % 4) && (((year) % 100) || !((year) % 400)))
#define YEARSIZE(year) (LEAPYEAR(year) ? 366 : 365)
static const int _ytab[2][12] =
{
	{ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
	{ 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};

typedef struct
{
	unsigned short    year;
	unsigned short    month;
	unsigned short    day;
	unsigned short    dow;
	char    sdow[4];
	unsigned short    hour;
	unsigned short    min;
	unsigned short    sec;
	unsigned short    now;
} frontpanel_ioctl_time;

static frontpanel_ioctl_time * gmtime(register const time_t time)
{
	static frontpanel_ioctl_time fptime;
	register unsigned long dayclock, dayno;
	int year = 2012;

	dayclock = (unsigned long)time % 86400;
	dayno = (unsigned long)time / 86400;

	fptime.sec = dayclock % 60;
	fptime.min = (dayclock % 3600) / 60;
	fptime.hour = dayclock / 3600;
	fptime.dow = (dayno + 4) % 7;       /* day 0 was a thursday */
	while (dayno >= YEARSIZE(year)) {
		dayno -= YEARSIZE(year);
		year++;
	}
	fptime.year = year;
	fptime.month = 0;
	while (dayno >= _ytab[LEAPYEAR(year)][fptime.month]) {
		dayno -= _ytab[LEAPYEAR(year)][fptime.month];
		fptime.month++;
	}
	fptime.day = dayno + 1;
	fptime.month++;

	return &fptime;
}

void run_time_thread(void *arg)
{
	frontpanel_ioctl_time *pTime;
	struct timeval tv;
	int sec = 0;
	int hour = 0;
	int pTime_test = 0;
	unsigned char trig = 0;
	sec_0 = 0;

	while(time_on != 0)
	{
		do_gettimeofday(&tv);
		tv.tv_sec += 0;
		pTime = gmtime(tv.tv_sec);
		sec =(pTime->sec);
		pTime_test =(pTime->hour);
		if(time_on == 1)
		{
			if(sec == 0 || sec_0 == 0)
			{
				sec_0 = 1;
				hour = gmt_on + pTime_test;
				if (hour >= 24)
					hour -= 24;
				VFD_Show_Time(hour,pTime->min);
				msleep(1);
			}
			trig =!trig;
			VFD_Show_Ico(TIME_SECOND,trig);
		}
	msleep(500);
	}
}

int run_draw_thread(struct vfd_ioctl_data *draw_data)
{
	if(!thread_stop)
		kthread_stop(thread);

	//wait thread stop
	while(!thread_stop)
		{msleep(2);}

	thread_stop = 2;
	thread = kthread_run(draw_thread,draw_data,"draw thread");

	//wait thread run
	while(thread_stop == 2)
		{msleep(1);}

	return 0;
}

static int VFD_Show_Time_Off(void)
{
	int res = 0;
	if (down_interruptible(&vfd_sem))
	{
		res =-EBUSY;
		return res;
	}
	res = VFD_Seg_Dig_Seg(9, SEGNUM1, 0x00);
	res = VFD_Seg_Dig_Seg(9, SEGNUM2, 0x00);
	res = VFD_Seg_Dig_Seg(10,SEGNUM1, 0x00);
	res = VFD_Seg_Dig_Seg(10,SEGNUM2, 0x00);
	up(&vfd_sem);
	VFD_Show_Ico(TIME_SECOND,0);
	return res;
}

unsigned char PROTONfp_Scan_Keyboard(unsigned char read_num)
{
	int   res = 0;
	unsigned char key_val[read_num] ;
	unsigned char i = 0;
	VFD_CS_CLR();

	res = VFD_Set_Mode(VFDREADMODE);
	if(res != 0)
	{
		dprintk(2, "%s DEVICE BUSY!\n", __func__);
		return INVALID_KEY;
	}
	
	for (i = 0; i < read_num; i++)
	{
		key_val[i] = PROTONfp_RD();
	}
	VFD_CS_SET();
	udelay(10);
	res = VFD_Set_Mode(VFDWRITEMODE);
	if(res != 0)
	{
		dprintk(2, "%s DEVICE BUSY!\n", __func__);
		return INVALID_KEY;
	}
	return key_val[5];
}

static int PROTONfp_Get_Key_Value(void)
{

	int byte, key_val = INVALID_KEY;

	byte = PROTONfp_Scan_Keyboard(6);

	switch(byte)
	{
		case 0x02:
			key_val = KEY_LEFT;
			break;
		case 0x04:
			key_val = KEY_UP;
			break;
		case 0x08:
			key_val = KEY_OK;
			break;
		case 0x10:
			key_val = KEY_RIGHT;
			break;
		case 0x20:
			key_val = KEY_DOWN;
			break;
		case 0x40:
			key_val = KEY_POWER;
			break;
		case 0x80:
			key_val = KEY_MENU;
			break;
		default :
			key_val = INVALID_KEY;
			break;
	}
	return key_val;
}

int protonSetBrightness(int level)
{
	int res = 0;
	if(level <=0)
		level = 0;
	else if(level >= 7)
		level = 7;

	VFD_CS_CLR();
	VFD_WR(0x88+level);
	VFD_CS_SET();

	up(&display_sem);
	udelay(20);

	return res ;
}

int protonSetTime(char* time)
{
	char buffer[8];
	int res = 0;

	dprintk(5, "%s >\n", __func__);

	dprintk(5, "%s time: %02d:%02d\n", __func__, time[2], time[3]);
	memset(buffer, 0, 8);
	memcpy(buffer, time, 5);
	VFD_Show_Time(time[2], time[3]);
	dprintk(5, "%s <\n", __func__);

	return res;
}

int vfd_init_func(void)
{
	int res = 0 ;
	dprintk(5, "%s >\n", __func__);
	printk("Spider HL101 VFD module initializing\n");

	init_MUTEX(&vfd_sem);
	init_rwsem(&vfd_rws);

	cfg.data_pin[0] = 3;
	cfg.data_pin[1] = 2;
	cfg.clk_pin[0] = 3;
	cfg.clk_pin[1] = 4;
	cfg.cs_pin[0] = 3;
	cfg.cs_pin[1] = 5;

	cfg.cs  = stpio_request_pin (cfg.cs_pin[0], cfg.cs_pin[1], "VFD CS", STPIO_OUT);
	cfg.clk = stpio_request_pin (cfg.clk_pin[0], cfg.clk_pin[1], "VFD CLK", STPIO_OUT);
	cfg.data= stpio_request_pin (cfg.data_pin[0], cfg.data_pin[1], "VFD DATA", STPIO_OUT);

	if(!cfg.cs || !cfg.data || !cfg.clk)
	{
		printk("vfd_init_func:  PIO errror!\n");
		return res;
	}

	sema_init(&cs_sem,1);
	msleep(200);
	VFD_CS_CLR();
	res = VFD_WR(0x0C);
	VFD_CS_SET();
	res = VFD_Set_Mode(VFDWRITEMODE);
	VFD_Seg_Addr_Init();
	VFD_Clear_All();
	VFD_Show_Content();
	return res;
}

int VFD_CLR(void)
{
	int res = 0;
	sema_init(&cs_sem,1);
	msleep(200);
	VFD_CS_CLR();
	res = VFD_WR(0x0C);
	VFD_CS_SET();
	res = VFD_Set_Mode(VFDWRITEMODE);
	VFD_Seg_Addr_Init();
	VFD_Clear_All();
	VFD_Show_Content();
	return res;
}

int protonSetIcon(int which, int on)
{
	int res = 0;
	dprintk(5, "%s > %d, %d\n", __func__, which, on);
	if (which < 1 || which > 45)
	{
		printk("VFD/proton icon number out of range %d\n", which);
		return -EINVAL;
	}
	which-=1;
	which = ((which/15)+11)*16+(which%15)+1;
	//if(stpio_get_pin(cfg.cs)==0) {udelay(1); stpio_set_pin(cfg.cs,1);}
	if (stpio_get_pin(cfg.cs) !=0) {
	res = VFD_Show_Ico(which, on);
	VFD_CS_SET();}
	dprintk(10, "%s <\n", __func__);
	return res;
}

/* export for later use in e2_proc */
EXPORT_SYMBOL(protonSetIcon);

static ssize_t PROTONdev_write(struct file *filp, const char *buff, size_t len, loff_t *off)
{
	//char* kernel_buf;
	int minor, vLoop, res = 0;

	struct vfd_ioctl_data data;

	dprintk(5, "%s > (len %d, offs %d)\n", __func__, len, (int) *off);

	minor = -1;
	for (vLoop = 0; vLoop < LASTMINOR; vLoop++)
	{
		if (FrontPanelOpen[vLoop].fp == filp)
			minor = vLoop;
	}

	if (minor == -1)
	{
		printk("Error Bad Minor\n");
		return -ENODEV;
	}

	dprintk(1, "minor = %d\n", minor);

	if (minor == FRONTPANEL_MINOR_RC)
		return -EOPNOTSUPP;

	if(down_interruptible (&write_sem))
		return -ERESTARTSYS;

	data.length = len;
	if (data.length > VFD_DATA_LEN)
		data.length = VFD_DATA_LEN;

	if ((data.length > 0) && (buff[data.length - 1] == '\n'))
		data.length--;

	if(data.length <0)
	{ 
		res = -1;
		dprintk(2, "empty string\n");
	}
	else
	{
		if (copy_from_user(data.data,buff,data.length))
			res = -EFAULT;
		else
			res=run_draw_thread(&data);
	}

	up(&write_sem);

	dprintk(10, "%s < res %d data.length %d\n", __func__, res, len);

	if (res < 0)
		return res;
	else
		return len;
}

static ssize_t PROTONdev_read(struct file *filp, char __user *buff, size_t len, loff_t *off)
{
	int minor, vLoop;

	dprintk(5, "%s > (len %d, offs %d)\n", __func__, len, (int) *off);

	minor = -1;
	for (vLoop = 0; vLoop < LASTMINOR; vLoop++)
	{
		if (FrontPanelOpen[vLoop].fp == filp)
			minor = vLoop;
	}

	if (minor == -1)
	{
		printk("Error Bad Minor\n");
		return -EUSERS;
	}

	dprintk(1, "minor = %d\n", minor);

	if (minor == FRONTPANEL_MINOR_RC)
	{
		while (receiveCount == 0)
		{
			if (wait_event_interruptible(wq, receiveCount > 0))
				return -ERESTARTSYS;
		}

		/* 0. claim semaphore */
		down_interruptible(&receive_sem);

		/* 1. copy data to user */
		copy_to_user(buff, receive[0].buffer, receive[0].len);

		/* 2. copy all entries to start and decreas receiveCount */
		receiveCount--;
		memmove(&receive[0], &receive[1], 99 * sizeof(struct receive_s));

		/* 3. free semaphore */
		up(&receive_sem);

		return 8;
	}

	/* copy the current display string to the user */
	if (down_interruptible(&FrontPanelOpen[minor].sem))
	{
		printk("%s return erestartsys<\n", __func__);
		return -ERESTARTSYS;
	}

	if (FrontPanelOpen[minor].read == lastdata.length)
	{
		FrontPanelOpen[minor].read = 0;
		up (&FrontPanelOpen[minor].sem);
		printk("%s return 0<\n", __func__);
		return 0;
	}

	if (len > lastdata.length)
		len = lastdata.length;

	/* fixme: needs revision because of utf8! */
	if (len > 16)
		len = 16;

	FrontPanelOpen[minor].read = len;
	copy_to_user(buff, lastdata.data, len);

	up (&FrontPanelOpen[minor].sem);

	dprintk(10, "%s < (len %d)\n", __func__, len);
	return len;
}

int PROTONdev_open(struct inode *inode, struct file *filp)
{
	int minor;

	dprintk(5, "%s >\n", __func__);

	minor = MINOR(inode->i_rdev);

	dprintk(1, "open minor %d\n", minor);

	if (FrontPanelOpen[minor].fp != NULL)
	{
		printk("EUSER\n");
		return -EUSERS;
	}
	FrontPanelOpen[minor].fp = filp;
	FrontPanelOpen[minor].read = 0;

	dprintk(5, "%s <\n", __func__);
	return 0;
}

int PROTONdev_close(struct inode *inode, struct file *filp)
{
	int minor;

	dprintk(5, "%s >\n", __func__);

	minor = MINOR(inode->i_rdev);

	dprintk(1, "close minor %d\n", minor);

	if (FrontPanelOpen[minor].fp == NULL)
	{
		printk("EUSER\n");
		return -EUSERS;
	}
	FrontPanelOpen[minor].fp = NULL;
	FrontPanelOpen[minor].read = 0;

	dprintk(5, "%s <\n", __func__);
	return 0;
}

int protonSetONOFF(int onoff)
{
	if(onoff == 1 && time_on == 0)
	{
		time_on = 1;
		time_thread=kthread_run(run_time_thread,NULL,"time thread");
	}
	else if(onoff == 0)
		{
			time_on = 0;
			VFD_Show_Time_Off();
		}
	return 0;
}

int protonSetGMT(int gmt)
{
	gmt_on = gmt;
	sec_0 = 0;
	return 0;
}

int VFD_ON_All(void)
{
	int res = 0;
	unsigned char i;

	if (down_interruptible(&vfd_sem))
	{
		res =-EBUSY;
		return res;
	}
	for(i = 0; i < 13; i++)
	{
		VFD_Seg_Dig_Seg(i + 1,SEGNUM1, 0xff);
		VFD_Seg_Dig_Seg(i + 1,SEGNUM2, 0xff);
	}
	up(&vfd_sem);
	return res;
}

static struct vfd_ioctl_data vfd_data;

static int PROTONdev_ioctl(struct inode *Inode, struct file *File, unsigned int cmd, unsigned long arg)
{
	static int mode = 0;
	struct proton_ioctl_data * proton = (struct proton_ioctl_data *)arg;
	int res = -EINVAL;

	dprintk(5, "%s > 0x%.8x\n", __func__, cmd);

	if(down_interruptible (&write_sem))
		return -ERESTARTSYS;

	switch(cmd) {
	case VFDSETMODE:
		mode = proton->u.mode.compat;
		break;
	case VFDSETLED:
		res = VFD_ON_All();
		break;
	case VFDBRIGHTNESS:
		res = protonSetBrightness(proton->u.brightness.level);
		mode = 0;
		break;
	case VFDONOFF:
		res = protonSetONOFF(proton->u.onoff.level);
		break;
	case VFDGMT:
		res = protonSetGMT(proton->u.gmt.level);
		break;
	case VFDICONDISPLAYONOFF:
		if (mode == 0)
		{
			res = protonSetIcon(proton->u.icon.icon_nr, proton->u.icon.on);
		}
		mode = 0;
		break;
	case VFDSTANDBY:
		break;
	case VFDSETTIME:
		if (proton->u.time.time != 0 && time_on == 0)
			res = protonSetTime(proton->u.time.time);
		break;
	case VFDGETTIME:
		break;
	case VFDGETWAKEUPMODE:
		break;
	case VFDDISPLAYCHARS:
		if (mode == 0)
		{
			if (copy_from_user(&vfd_data, (void *) arg, sizeof(vfd_data)))
				return -EFAULT;
			if (vfd_data.length > sizeof(vfd_data.data))
				vfd_data.length = sizeof(vfd_data.data);
			while ((vfd_data.length > 0) && (vfd_data.data[vfd_data.length - 1 ] == '\n'))
				vfd_data.length--;
				res = run_draw_thread(&vfd_data);
		}
		mode = 0;
		break;
	case VFDDISPLAYWRITEONOFF:
		if(proton->u.mode.compat) // 1 = show, 0 = off
			VFD_Show_Content();
		else
			VFD_Show_Content_Off();
		break;
	case VFDDISPLAYCLR:
		if(!thread_stop)
			kthread_stop(thread);
		while(!thread_stop)
			{msleep(2);}
		VFD_CLR();
		break;
	default:
		printk("VFD/Proton: unknown IOCTL 0x%x\n", cmd);
		mode = 0;
		break;
	}

	up(&write_sem);

   dprintk(5, "%s <\n", __func__);
   return res;
}

static unsigned int PROTONdev_poll(struct file *filp, poll_table *wait)
{
	unsigned int mask = 0;
	poll_wait(filp, &wq, wait);
	if(receiveCount > 0)
		mask = POLLIN | POLLRDNORM;
	return mask;
}

static struct file_operations vfd_fops =
{
	.owner = THIS_MODULE,
	.ioctl = PROTONdev_ioctl,
	.write = PROTONdev_write,
	.read  = PROTONdev_read,
	.poll  = (void*) PROTONdev_poll,
	.open  = PROTONdev_open,
	.release  = PROTONdev_close
};

/*----- Button driver -------*/

static char *button_driver_name = "Spider HL101 frontpanel buttons";
static struct input_dev *button_dev;
static int button_value = -1;
static int bad_polling = 1;
static struct workqueue_struct *fpwq;

void button_bad_polling(void)
{
	int btn_pressed = 0;
	int report_key = 0;
	while(bad_polling == 1)
	{
		msleep(50);
		button_value = PROTONfp_Get_Key_Value();
		if (button_value != INVALID_KEY) {
			dprintk(5, "got button: %X\n", button_value);
			VFD_Show_Ico(DOT2,LOG_ON);
			if (1 == btn_pressed)
			{
				if (report_key == button_value)
					continue;
				input_report_key(button_dev, report_key, 0);
				input_sync(button_dev);
			}
			report_key = button_value;
			btn_pressed = 1;
			switch(button_value) {
				case KEY_LEFT: {
					input_report_key(button_dev, KEY_LEFT, 1);
					input_sync(button_dev);
					break;
				}
				case KEY_RIGHT: {
					input_report_key(button_dev, KEY_RIGHT, 1);
					input_sync(button_dev);
					break;
				}
				case KEY_UP: {
					input_report_key(button_dev, KEY_UP, 1);
					input_sync(button_dev);
					break;
				}
				case KEY_DOWN: {
					input_report_key(button_dev, KEY_DOWN, 1);
					input_sync(button_dev);
					break;
				}
				case KEY_OK: {
					input_report_key(button_dev, KEY_OK, 1);
					input_sync(button_dev);
					break;
				}
				case KEY_MENU: {
					input_report_key(button_dev, KEY_MENU, 1);
					input_sync(button_dev);
					break;
				}
				case KEY_POWER: {
					input_report_key(button_dev, KEY_POWER, 1);
					input_sync(button_dev);
					break;
				}
				default:
					dprintk(5, "[BTN] unknown button_value?\n");
			}
		}
		else {
			if(btn_pressed) {
				btn_pressed = 0;
				msleep(80);
				VFD_Show_Ico(DOT2,LOG_OFF);
				input_report_key(button_dev, report_key, 0);
				input_sync(button_dev);
			}
		}
	}
	bad_polling = 2;
}
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,17)
static DECLARE_WORK(button_obj, button_bad_polling);
#else
static DECLARE_WORK(button_obj, button_bad_polling, NULL);
#endif
static int button_input_open(struct input_dev *dev)
{
	fpwq = create_workqueue("button");
	if(queue_work(fpwq, &button_obj)) {
		dprintk(5, "[BTN] queue_work successful ...\n");
		return 0;
	}
	dprintk(5, "[BTN] queue_work not successful, exiting ...\n");
	return 1;
}

static void button_input_close(struct input_dev *dev)
{
	bad_polling = 0;
	while (bad_polling != 2)
		msleep(1);
	bad_polling = 1;

	if (fpwq)
	{
		destroy_workqueue(fpwq);
		dprintk(5, "[BTN] workqueue destroyed\n");
	}
}

int button_dev_init(void)
{
	int error;

	dprintk(5, "[BTN] allocating and registering button device\n");

	button_dev = input_allocate_device();
	if (!button_dev)
		return -ENOMEM;

	button_dev->name = button_driver_name;
	button_dev->open = button_input_open;
	button_dev->close= button_input_close;


	set_bit(EV_KEY        , button_dev->evbit );
	set_bit(KEY_UP        , button_dev->keybit);
	set_bit(KEY_DOWN      , button_dev->keybit);
	set_bit(KEY_LEFT      , button_dev->keybit);
	set_bit(KEY_RIGHT     , button_dev->keybit);
	set_bit(KEY_POWER     , button_dev->keybit);
	set_bit(KEY_MENU      , button_dev->keybit);
	set_bit(KEY_OK        , button_dev->keybit);

	error = input_register_device(button_dev);
	if (error)
		input_free_device(button_dev);

	return error;
}

void button_dev_exit(void)
{
	dprintk(5, "[BTN] unregistering button device\n");
	input_unregister_device(button_dev);
}

static struct cdev   vfd_cdev;
static struct class *vfd_class = 0;

static int __init proton_init_module(void)
{
	int i;
	int result;
	dprintk(5, "%s >\n", __func__);
	sema_init(&display_sem, 1);

	if(vfd_init_func()) {
		printk("unable to init module\n");
		return -1;
	}

	if(button_dev_init() != 0)
		return -1;

//	if (register_chrdev(VFD_MAJOR,"VFD",&vfd_fops))
//		printk("unable to get major %d for VFD\n",VFD_MAJOR);
	result = register_chrdev_region(MKDEV(VFD_MAJOR, 0), 2, "proton");
	if (result < 0) {
		printk( KERN_ALERT "VFD cannot register device (%d)\n", result);
		return result;
	}

	cdev_init(&vfd_cdev, &vfd_fops);
	vfd_cdev.owner = THIS_MODULE;
	vfd_cdev.ops   = &vfd_fops;
	if (cdev_add(&vfd_cdev, MKDEV(VFD_MAJOR, 0), 2) < 0) { 
		printk("VFD couldn't register '%s' driver\n", "proton"); 
		return -1; 
	}

	vfd_class = class_create(THIS_MODULE, "proton");
	device_create(vfd_class, NULL, MKDEV(VFD_MAJOR, 0), NULL, "vfd", 0);
	device_create(vfd_class, NULL, MKDEV(VFD_MAJOR, 1), NULL, "rc", 1);

	sema_init(&write_sem, 1);
	sema_init(&key_mutex, 1);

	for (i = 0; i < LASTMINOR; i++)
		sema_init(&FrontPanelOpen[i].sem, 1);

	//time_thread=kthread_run(run_time_thread,NULL,"time thread",NULL,true);

	dprintk(5, "%s < %d\n", __func__, result);

	return result;
}

static void __exit proton_cleanup_module(void)
{
	if(cfg.data != NULL)
		stpio_free_pin (cfg.data);
	if(cfg.clk != NULL)
		stpio_free_pin (cfg.clk);
	if(cfg.cs != NULL)
		stpio_free_pin (cfg.cs);

	while(!thread_stop)
		msleep(2);

	dprintk(5, "[BTN] unloading ...\n");
	button_dev_exit();

	if(!thread_stop && time_thread)
		kthread_stop(time_thread);

	cdev_del(&vfd_cdev);
	unregister_chrdev_region(MKDEV(VFD_MAJOR, 0), 2);
	device_destroy(vfd_class, MKDEV(VFD_MAJOR, 0));
	device_destroy(vfd_class, MKDEV(VFD_MAJOR, 1));
	class_destroy(vfd_class);
	printk("HL101 FrontPanel module unloading\n");
}

module_init(proton_init_module);
module_exit(proton_cleanup_module);

module_param(paramDebug, short, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(paramDebug, "Debug Output 0=disabled >0=enabled(debuglevel)");

MODULE_DESCRIPTION("VFD module for Spider HL101");
MODULE_AUTHOR("Spider-Team");
MODULE_LICENSE("GPL");
