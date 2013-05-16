/*
 *
 */ 

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include "vfd-functions.h"

int file_vfd;

void opendevice()
{
	if ((file_vfd = open("/dev/vfd", O_RDWR)) < 0) { 
		printf("VFD: Error - can't open /dev/vfd\n");
	};
}

void closedevice()
{
	if (file_vfd != -1) {
		close(file_vfd);
	}
}

void initdisplay()
{
	struct vfd_ioctl_data data;
	
	data.start_address = 0x01;
	data.length = 0;
	ioctl(file_vfd, VFDDRIVERINIT, &data);
	return;
} 

void writeonoff(int onoff)
{
	struct vfd_ioctl_data data;
	
	if (onoff == 1)
		data.start_address = 0x01;
	else
		data.start_address = 0x00;
	data.length = 0;
	ioctl(file_vfd, VFDDISPLAYWRITEONOFF, &data);
	return;
} 

void displayicon(int icon, int onoff)
{
	struct vfd_ioctl_data data;
	
	data.start_address = 0x00;
	data.data[0] = icon;
	data.data[4] = onoff;
	data.length = 5;
	ioctl(file_vfd, VFDICONDISPLAYONOFF, &data);
	return;
} 

void brightness(int value)
{
	struct vfd_ioctl_data data;
	
	data.start_address = value;
	data.length = 0;
	ioctl(file_vfd, VFDBRIGHTNESS, &data);
	return;
} 

void writedisplay(const char* str) {
	int i;

	struct vfd_ioctl_data data;

	memset(data.data, ' ', 16);

	i = strlen(str);
	if (i > 16) i = 16;
	memcpy(data.data, str, i);

	data.start_address = 0;
	data.length = 16;

	ioctl(file_vfd, VFDDCRAMWRITE, &data);
	return;
}

