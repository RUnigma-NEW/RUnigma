#ifndef SHME2
#define SHME2

#include <sys/ipc.h>
#include <sys/shm.h>

#define SHMSZ 27
#define KEY 1234

static char* createshm()
{
	int shmid;
	key_t key = KEY;
	char *shm = NULL;

	if((shmid = shmget(key, SHMSZ, IPC_CREAT | 0666)) < 0) {
		return NULL;
	}

	shm = (char *) shmat(shmid, NULL, 0);
	if(shm == (char *) -1)
		return NULL;

	*shm = '\0';

	return shm;
}

static char* findshm()
{
	int shmid;
	key_t key = KEY;
	char *shm = NULL;

	if((shmid = shmget(key, SHMSZ, 0666)) < 0) {
		return NULL;
	}

	shm = (char *) shmat(shmid, NULL, 0);
	if(shm == (char *) -1)
		return NULL;

	return shm;
}

static int delshmentry(char *shm, char *key)
{
	if(shm == NULL) return -1;
	char *vbuf = NULL, *shmbuf = NULL;
	char *v, *s = shm;
	int matchkey = 0;
	int ret = 0;

	vbuf = (char *) malloc(256);
	if(vbuf == NULL)
		return -1;
	v = vbuf;
	memset(vbuf, '\0', 256);

	//FIXME: here we can have a segfault
	shmbuf = (char *) malloc(4096);
	if(shmbuf == NULL)
	{
		free(vbuf);
		return -1;
	}
	memset(shmbuf, '\0', 4096);

	while(*s != '\0')
	{
		*v++ = *s++;
		*v = '\0';
		if(strcmp(key, vbuf) == 0)
		{
			matchkey = 1;
			ret = 1;
		}
		if(matchkey == 0 && *s == ';')
		{
			*v++ = ';';
			*v = '\0';
			strcat(shmbuf, vbuf);
			v = vbuf;
			s++;
		}
		if(matchkey == 1 && *s == ';')
		{
			matchkey = 0;
			v = vbuf;
			s++;
		}
	}
	if(matchkey == 0)
	{
		*v++ = ';';
		*v = '\0';
		strcat(shmbuf, vbuf);
		v = vbuf;
	}
	shmbuf[strlen(shmbuf)-1] = '\0';
	*shm = '\0';
	strcpy(shm, shmbuf);

	free(vbuf);
	free(shmbuf);

	return ret;
}

static int setshmentry(char *shm, char *entry)
{
	if(shm == NULL) return -1;
	char *buf = NULL;
	char *c = NULL;
	int pos = 0;

	if(strlen(entry) > 255)
		return -1;

	buf = (char *) malloc(256);
	if(buf == NULL)
		return -1;
	memset(buf, '\0', 256);

	c = strchr(entry, '=');
	if(c != NULL)
		pos = (c - entry) + 1;
	else
	{
		free(buf);
		return -1;
	}

	strncpy(buf, entry, pos);
	buf[pos] = '\0';

	if(delshmentry(shm, buf) < 0)
	{
		free(buf);
		return -1;
	}

	if(*shm != '\0')
		strcat(shm, ";");
	strcat(shm, entry);

	free(buf);
	return 1;
}

static int getshmentry(char *shm, char *key, char *buf, int buflen)
{
	if(shm == NULL) return -1;
	char *v, *s = shm, *shmbuf;
	int matchkey = 0;
	int ret = 0;

	shmbuf = (char *) malloc(256);
	if(shmbuf == NULL)
		return -1;
	memset(shmbuf, '\0', 256);

	v = shmbuf;

	while(*s != '\0')
	{
		*v++ = *s++;
		*v = '\0';
		if(strcmp(key, shmbuf) == 0)
		{
			v = shmbuf;
			matchkey = 1;
			ret = 1;
		}
		if(matchkey == 0 && *s == ';')
		{
			v = shmbuf;
			s++;
		}
		if(matchkey == 1 && *s == ';')
		break;
	}
	if(matchkey == 0)
	v = shmbuf;
	*v = '\0';

	strncpy(buf, shmbuf, buflen-1);
	buf[buflen] = '\0';
	free(shmbuf);

	return ret;
}

static int checkshmentry(char *shm, char *key)
{
	if(shm == NULL) return -1;
	char *v = NULL, *s = shm, *shmbuf = NULL;
	int ret = 0;

	shmbuf = (char *) malloc(256);
	if(shmbuf == NULL)
		return -1;
	memset(shmbuf, '\0', 256);

	v = shmbuf;

	while(*s != '\0')
	{
		*v++ = *s++;
		*v = '\0';
		if(strcmp(key, shmbuf) == 0)
		{
			ret = 1;
			break;
		}
		if(*s == ';')
		{
			v = shmbuf;
			s++;
		}
	}
	free(shmbuf);
	return ret;
}

static int getshmentryall(char *shm, char *shmbuf, int buflen)
{
	if(shm == NULL) return -1;
	char *v = shmbuf, *s = shm;
	int count = 0;

	while(*s != '\0')
	{
		*v++ = *s++;
		*v = '\0';
		count++;
		if(*s == ';')
		{
			if(count >= buflen-1)
				break;
			s++;
			*v++ = '\n';
			count++;
                }
		if(count >= buflen-1)
			break;
	}
	*v = '\0';

	return 0;
}

static int incshmentry(char *shm, char *entry)
{
	if(shm == NULL) return -1;
	char *buf = NULL;
	char *pch;
	int value;
	
	if(strlen(entry) > 255)
		return -1;

	buf = (char *) malloc(256);
	if(buf == NULL)
		return -1;
	memset(buf, '\0', 256);
	
	getshmentry(shm, entry, buf, 256);
		
	if(strlen(buf) == 0) {
		sprintf(buf, "%s%d", entry, 1);
		if(setshmentry(shm, buf) != 1) {
			free(buf);
			return 0;
		}
		free(buf);
		return 1;
	}
		
	pch = strtok(buf, "=");
	if(pch != NULL) {
		value = atoi(pch);
		value++;
		sprintf(buf, "%s%d", entry, value);
		if(setshmentry(shm, buf) != 1) {
			free(buf);
			return 0;
		}
	}
		
	free(buf);
	return 1;
}
	
static int decshmentry(char *shm, char *entry)
{
	if(shm == NULL) return -1;
	char *buf = NULL;
	char *pch;
	int value;
	
	if(strlen(entry) > 255)
		return -1;

	buf = (char *) malloc(256);
	if(buf == NULL)
		return -1;
	memset(buf, '\0', 256);

	getshmentry(shm, entry, buf, 256);
		
	if(strlen(buf) == 0) {
		free(buf);
		return 1;
	}
			
	pch = strtok(buf, "=");
	if(pch != NULL) {
		value = atoi(pch);
		value--;
		if(value < 1) {
			delshmentry(shm, entry);
			free(buf);
			return 1;
		}
		sprintf(buf, "%s%d", entry, value);
		if(setshmentry(shm, buf) != 1) {
			free(buf);
			return 0;
		}
	}

	free(buf);
	return 1;
}

#endif
