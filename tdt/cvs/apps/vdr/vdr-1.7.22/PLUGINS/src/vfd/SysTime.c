#include "SysTime.h"

#define bcdtoint(i) ((((i & 0xf0) >> 4) * 10) + (i & 0x0f))

/*** cSysTime ***************************************************************************************/
cSysTime::cSysTime()
{
	printf("SystTime Klasse erstellt\n");
}

cSysTime::~cSysTime() {
	printf("SystTime Klasse zerstoert\n");
}

void cSysTime::ChannelHasLock(const cDevice *Device, int HasLock) {
	if(HasLock)
		printf("HAT LOCK\n");
	else
		printf("HAT KEIN LOCK\n");
	// OsdChannel wird angezeit wenn VDR gestartet wird
	
	// wenn real_time < 2011
	// scan_time
	// wenn setzte scan_time as realtime
	
	
/* 	

	int timeout = 25;
	time_t real_time;
	time_t dvb_time;

	time(&real_time);
	scan_date(&dvb_time, timeout);

	fprintf(stdout, "System time: %s", ctime(&real_time));
	fprintf(stdout, "   TDT time: %s", ctime(&dvb_time)); */
}

time_t cSysTime::convert_date(unsigned char *dvb_buf)
{
	int i;
	int year, month, day, hour, min, sec;
	long int mjd;
	struct tm dvb_time;

	mjd = (dvb_buf[0] & 0xff) << 8;
	mjd += (dvb_buf[1] & 0xff);
	hour = bcdtoint(dvb_buf[2] & 0xff);
	min = bcdtoint(dvb_buf[3] & 0xff);
	sec = bcdtoint(dvb_buf[4] & 0xff);

	year = (int) ((mjd - 15078.2) / 365.25);
	month = (int) ((mjd - 14956.1 - (int) (year * 365.25)) / 30.6001);
	day = mjd - 14956 - (int) (year * 365.25) - (int) (month * 30.6001);
	if (month == 14 || month == 15)
		i = 1;
	else
		i = 0;
	year += i;
	month = month - 1 - i * 12;

	dvb_time.tm_sec = sec;
	dvb_time.tm_min = min;
	dvb_time.tm_hour = hour;
	dvb_time.tm_mday = day;
	dvb_time.tm_mon = month - 1;
	dvb_time.tm_year = year;
	dvb_time.tm_isdst = -1;
	dvb_time.tm_wday = 0;
	dvb_time.tm_yday = 0;
	return (timegm(&dvb_time));
}

int cSysTime::scan_date(time_t *dvb_time, unsigned int to)
{
	int fd_date;
	int n, seclen;
	time_t t;
	unsigned char buf[4096];
	struct dmx_sct_filter_params sctFilterParams;
	struct pollfd ufd;
	int found = 0;
	
	t = 0;
	if ((fd_date = open("/dev/dvb/adapter0/demux0", O_RDWR | O_NONBLOCK)) < 0) {
		perror("fd_date DEVICE: ");
		return -1;
	}

	memset(&sctFilterParams, 0, sizeof(sctFilterParams));
	sctFilterParams.pid = 0x14;
	sctFilterParams.timeout = 0;
	sctFilterParams.flags = DMX_IMMEDIATE_START;
	sctFilterParams.filter.filter[0] = 0x70;
	sctFilterParams.filter.mask[0] = 0xff;

	if (ioctl(fd_date, DMX_SET_FILTER, &sctFilterParams) < 0) {
		perror("DATE - DMX_SET_FILTER:");
		close(fd_date);
		return -1;
	}

	while (to > 0) {
		int res;

		memset(&ufd,0,sizeof(ufd));
		ufd.fd=fd_date;
		ufd.events=POLLIN;

		res = poll(&ufd,1,1000);
		if (0 == res) {
			fprintf(stdout, ".");
			fflush(stdout);
			to--;
			continue;
  		}
		if (1 == res) {
			found = 1;
			break;
		}
		printf("error polling for data");
		close(fd_date);
		return -1;
	}
	fprintf(stdout, "\n");
	if (0 == found) {
		printf("timeout - try tuning to a multiplex?\n");
		close(fd_date);
		return -1;
	}

	if ((n = read(fd_date, buf, 4096)) >= 3) {
		seclen = ((buf[1] & 0x0f) << 8) | (buf[2] & 0xff);
		if (n == seclen + 3) {
			t = convert_date(&(buf[3]));
		} else {
			printf("Under-read bytes for DATE - wanted %d, got %d\n", seclen, n);
			return 0;
		}
	} else {
		printf("Nothing to read from fd_date - try tuning to a multiplex?\n");
		return 0;
	}
	close(fd_date);
	*dvb_time = t;
	return 0;
}