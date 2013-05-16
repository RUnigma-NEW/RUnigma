/*
 *
 */ 

#include <string.h>

#define VFDDCRAMWRITE		0xc0425a00
#define VFDBRIGHTNESS		0xc0425a03
#define VFDDISPLAYWRITEONOFF	0xc0425a05
#define VFDDRIVERINIT		0xc0425a08
#define VFDICONDISPLAYONOFF		0xc0425a0a

struct vfd_ioctl_data {
	unsigned char start_address;
	unsigned char data[64];
	unsigned char length;
};

void opendevice();
void closedevice();
void initdisplay();
void writeonoff(int onoff);
void displayicon(int icon, int onoff);
void brightness(int value);
void writedisplay(const char* str);
