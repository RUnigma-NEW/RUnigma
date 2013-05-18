// #include <string.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <poll.h>

#include "playback.h"
#include "common.h"

#define DEBUG

static const char FILENAME[] = "playback.c";

//little helper functions
static int playback_getline(char** pbuffer, size_t* pbufsize, int fd)
{
	size_t i = 0;
	int rc;
	while (1) {
		if (i >= *pbufsize) {
			char *newbuf = (char*)realloc(*pbuffer, (*pbufsize)+1024);
			if (newbuf == NULL)
				return -1;
			*pbuffer = newbuf;
			*pbufsize = (*pbufsize)+1024;
		}
		rc = read(fd, (*pbuffer)+i, 1);
		if (rc <= 0 || (*pbuffer)[i] == '\n')
		{
			(*pbuffer)[i] = '\0';
			return rc <= 0 ? -1 : i;
		}
		if ((*pbuffer)[i] != '\r') i++;
	}
}

int openHttpConnection(Context_t  *context, char** content,off_t off)
{
	char* url=context->playback->uri;
	char host[256];
	int port = 80;
	char uri[512];
	int x;
	int slash = 0;
	for(x=7;x<strlen(url)-1;x++){
		if(url[x]=='/'){
			slash=x;
			break;
		}
	}
	if (slash > 0) {
		strncpy(host,url+7,slash-7);
		host[slash-7]='\0';
		strcpy(uri,url+slash);
	} else {
		strcpy(host,url+7);
		strcpy(uri,"/");
	}
	int dp=0;
	for(x=0;x<strlen(host)-1;x++){
		if(host[x]==':'){
			dp=x;
			break;
		}
	}
	if (dp > 0) {
		port = atoi(host+dp+1);
		//host = host.substr(0, dp);
		host[dp]='\0';
	}
#ifdef DEBUG
	printf("URL: %s\n",url);
	printf("Host: %s\n",host);
	printf("URI: %s\n",uri);
	printf("Port: %d\n",port);
#endif

	struct hostent* h = gethostbyname(host);
	if (h == NULL || h->h_addr_list == NULL){
		printf("hostlookup failed\n");
		return 0;
	}
	int fd = socket(PF_INET, SOCK_STREAM, 0);
	if (fd == -1){
		printf("creating socket failed");
		return 0;
	}
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = *((in_addr_t*)h->h_addr_list[0]);
	addr.sin_port = htons(port);

#ifdef DEBUG
	printf("connecting to %s\n", url);
#endif

	if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		printf("connect failed for: %s\n", url);
		return 0;
	}
	char request[1024];
	strcpy(request,"GET ");
	strcat(request,uri);
	strcat(request," HTTP/1.1\r\n");
	strcat(request,"Host: ");
	strcat(request,host);
	if(port!=80){
		char p[16];
		sprintf(p,"%d",port); 
		strcat(request,":");
		strcat(request,p);
	}
	strcat(request,"\r\n");
	
	char b[32];

/* konfetti: I think this must be %lu because in case of big files the value becomes 
 * negative in %ld case which might not work!!!
 * I dont know how to test so I did not change...(see openUPNPConnection which works fine)
 */
	sprintf(b,"%ld",(long int) off); 
	strcat(request,"Range: bytes=");
	strcat(request,b);
	strcat(request,"-\r\n");
	
	//strcat(request,"User-Agent: VLC mediaplayer - version 1.0.2\r\n");
	//strcat(request,"Range: bytes=0-\r\n");
	//strcat(request,"Icy-MetaData: 1\r\n");
	strcat(request,"Accept: */*\r\n");
	strcat(request,"Connection: close\r\n");
	strcat(request,"\r\n");
#ifdef DEBUG
	printf("sending:\n%s",request);
#endif
	write(fd, request, strlen(request));

	int rc,ext_found=0;
	size_t buflen = 1000;
	char* linebuf = (char*)malloc(1000);

	rc = playback_getline(&linebuf, &buflen, fd);
#ifdef DEBUG
	printf("RECV(%d): %s\n", rc, linebuf);
#endif
	if (rc <= 0)
	{
		close(fd);
		free(linebuf);
		return 0;
	}

	char proto[100];
	int statuscode = 0;
	char statusmsg[100];
	rc = sscanf(linebuf, "%99s %d %99s", proto, &statuscode, statusmsg);
	if (statuscode == 303){
#ifdef DEBUG
		printf("redirecting.\n");
		printf("got: %s\n",linebuf);
#endif
		while ((rc = playback_getline(&linebuf, &buflen, fd)) > 0)
		{
			if(!strncmp("Location: ", linebuf, 10)){
				close(fd);
				free(context->playback->uri);
				context->playback->uri=strdup(linebuf+10);
				return openHttpConnection(context, content,off);
			}
#ifdef DEBUG
			printf("RECV(%d): %s\n", rc, linebuf);
#endif
		}
		free(linebuf);
		close(fd);
		return 0;
	}
	if (rc != 3 || (statuscode != 200 && statuscode != 206)) {
		printf("wrong response: \"200 OK\" expected.\n");
#ifdef DEBUG
		printf("got: %s\n",linebuf);
#endif
		while ((rc = playback_getline(&linebuf, &buflen, fd)) > 0)
		{
#ifdef DEBUG
			printf("RECV(%d): %s\n", rc, linebuf);
#endif
		}
		free(linebuf);
		close(fd);
		return 0;
	}
#ifdef DEBUG
	printf("proto=%s, code=%d, msg=%s\n", proto, statuscode, statusmsg);
#endif
	while ((rc = playback_getline(&linebuf, &buflen, fd)) > 0)
	{
#ifdef DEBUG
		printf("RECV(%d): %s\n", rc, linebuf);
#endif
		if(!strncmp("Content-Length: ", linebuf, 16)){
				long int pos;
				sscanf(linebuf+16,"%ld",&pos);
				context->playback->size=pos+off;
		}
		if(rc == 24) { //ContentType
			int ret = 0;
#ifdef DEBUG
			printf("%s: Content-Type: ", __func__);
#endif
			if(!(ret = strncmp("audio/mpeg", linebuf+14, 10))) {
#ifdef DEBUG
				printf("mp3");
				printf("strdup in %s::%s:%d\n", FILENAME, __FUNCTION__,__LINE__);
#endif
				*content = strdup("mp3");
				ext_found=1;
			} else  {
				printf(" Unknown (%s) %d", linebuf, ret);
			}
			printf("\n");
		} else if(rc == 23) {
			int ret = 0;
#ifdef DEBUG
			printf("%s: Content-Type: ", __func__);
#endif
			if(!(ret = strncmp("audio/mpeg", linebuf+13, 10))) {
#ifdef DEBUG
				printf("mp3");
				printf("strdup in %s::%s:%d\n", FILENAME, __FUNCTION__,__LINE__);
#endif
				*content = strdup("mp3");
				ext_found=1;
			}if(!(ret = strncmp("video/mp4", linebuf+14, 9))) {
#ifdef DEBUG
				printf("mp4");
				printf("strdup in %s::%s:%d\n", FILENAME, __FUNCTION__,__LINE__);
#endif
				*content = strdup("mp4");
				ext_found=1;
			} else  {
				printf(" Unknown (%s) %d", linebuf, ret);
			}
			printf("\n");
		} else {
			
		}
	}
	free(linebuf);
	if(!ext_found){
		//default mp3
#ifdef DEBUG
		printf("strdup in %s::%s:%d\n", FILENAME, __FUNCTION__,__LINE__);
#endif
		*content = strdup("mp3");
	}
	return fd;
}

/* konfetti: hack hack: copied from neutrino netfile.cpp 
 * I dont like most of the stuff in this file. we should 
 * implement a mechanism like netfile.cpp to open _ALL_
 * media file formats in the correct manner. just for the
 * first shot and because of missing time hacking the world ...
 */
int ConnectToUPNPServer(char *hostname, int port)
{
	struct hostent *host;
	struct sockaddr_in sock;
	int fd, addr;
	struct pollfd pfd;
        char err_txt[2048];

	printf("looking up hostname: %s\n", hostname);

	host = gethostbyname(hostname);

	if(host == NULL)
	{
		herror(err_txt);
		return -1;
	}

	addr = htonl(*(int *)host->h_addr);

	printf("connecting to %s [%d.%d.%d.%d], port %d\n", host->h_name,
			(addr & 0xff000000) >> 24,
			(addr & 0x00ff0000) >> 16,
			(addr & 0x0000ff00) >>  8,
			(addr & 0x000000ff), port);

	fd = socket(AF_INET, SOCK_STREAM, 0);

	if(fd == -1)
	{
		strcpy(err_txt, strerror(errno));
		return -1;
	}

	memset(&sock, 0, sizeof(sock));
	memmove((char*)&sock.sin_addr, host->h_addr, host->h_length);

	sock.sin_family = AF_INET;
	sock.sin_port = htons(port);

	int flgs = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flgs | O_NONBLOCK);

	if( connect(fd, (struct sockaddr *)&sock, sizeof(sock)) == -1 )
	{
		if(errno != EINPROGRESS) {
			close(fd);
			strcpy(err_txt, strerror(errno));
			printf("error connecting to %s: %s\n", hostname, err_txt);
			return -1;
		}
	}
	printf("connected to %s\n", hostname);
	pfd.fd = fd;
	pfd.events = POLLOUT | POLLERR | POLLHUP;
	pfd.revents = 0;

	int ret = poll(&pfd, 1, 5000);
	if(ret != 1) {
		strcpy(err_txt, strerror(errno));
		printf("error connecting to %s: %s\n", hostname, err_txt);
		close(fd);
		return -1;
	}
	if ((pfd.revents & POLLOUT) == POLLOUT) {
		fcntl(fd, F_SETFL, flgs &~ O_NONBLOCK);
		return fd;
	}

	return fd;
}

/* adapted from netfile.cpp */
void parseURL(char* url, char* host, char* file, int* port) 
{
	char *ptr = strstr(url, "://");

	if (ptr) /* otherwise this is not an url */
	{
	        if (strstr(url, "http") == NULL)
		{
		   printf("%s: not an url ? (%s)\n", __func__, url);
                   return;
		}

		strcpy(host, ptr + 3);
             
		/* extract the file path from the url */
		ptr = strchr(ptr + 3, '/');
		if(ptr) 
		{
	            strcpy(file, ptr);
		}
		else  
		{
		    strcpy(file, "/");
                }
		
		/* extract the host part from the url */
		ptr = strchr(host, '/');
		if(ptr) 
		    *ptr = 0;

#ifdef login_supported
		if ((ptr = strchr(host, '@')))
		{
			*ptr = 0;
			base64_encode(buffer, url.host);
			strcpy(url.logindata, buffer);

			strcpy(buffer, ptr + 1);
			strcpy(url.host, buffer);
		}
		else
			url.logindata[0] = 0;
#endif
		ptr = strrchr(host, ':');

		if(ptr)
		{
			*port = atoi(ptr + 1);
			*ptr = 0;
		}
	} else
	{
            printf("%s: not an url ? (%s)\n", __func__, url);
	}
}

int openUPNPConnection(Context_t *context, off_t off)
{
	char*  url = context->playback->uri;
	char   host[256], uri[512], request[1024], proto[100], statusmsg[100];
	int    port = 80, statuscode = 0, fd, rc;
	size_t buflen = 1000;
	char*  linebuf = (char*)malloc(1000);
	
	printf("%s (0x%lx %lu) linebuf = %p>\n", __func__, (long unsigned int) off, (long unsigned int) off, linebuf);
	
        parseURL(url, host, uri, &port);

#ifdef DEBUG
	printf("URL: %s\n",url);
	printf("Host: %s\n",host);
	printf("URI: %s\n",uri);
	printf("Port: %d\n",port);
#endif

        fd = ConnectToUPNPServer(host, port);

        /* send the get message (only http/1.1 supported) */ 
	strcpy(request,"GET ");
	strcat(request,uri);
	strcat(request," HTTP/1.1\r\n");
	send(fd, request, strlen(request), 0);
	
	/* now send hostname */
	sprintf(request, "Host: %s:%d\r\n", host, port);
	send(fd, request, strlen(request), 0);

        /* send the offset */
	sprintf(request, "Range: bytes=%lu-\r\n", (unsigned long int) off);
printf("send: %s\n", request);
	send(fd, request, strlen(request), 0);

        /* now send a "faked" user agent */
	sprintf(request, "User-Agent: WinampMPEG/5.52\r\n");
	send(fd, request, strlen(request), 0);

/* fixme: authorization if needed (see netfile.cpp) */

	sprintf(request, "Accept: */*\r\n");
	send(fd, request, strlen(request), 0);

	sprintf(request , "Connection: close\r\n");
	send(fd, request, strlen(request), 0);

	/* end request */
	sprintf(request , "\r\n");
	send(fd, request, strlen(request), 0);

	rc = playback_getline(&linebuf, &buflen, fd);
#ifdef DEBUG
	printf("RECV(%d): %s\n", rc, linebuf);
#endif
	if (rc <= 0)
	{
		close(fd);
		free(linebuf);
		return -1;
	}

	rc = sscanf(linebuf, "%99s %d %99s", proto, &statuscode, statusmsg);

#ifdef DEBUG
   printf("proto %s\n", proto);
   printf("statuscode %d\n", statuscode);
   printf("statuscode %s\n", statusmsg);
#endif
        
	/* we currently only accept http 1.1 */
        if (strstr(proto, "HTTP/1.1") == NULL)
	{
	    printf("%s: not an HTTP/1.1 response (%s)\n", __func__, proto);
	    free(linebuf);
	    return -1;
	}
	
	switch (statuscode)
	{
	   case 200: /* OK */
	   case 206: /* Partial */
#ifdef DEBUG
             printf("%s: statuscode ok or partial %d\n", __func__, statuscode);
#endif
	     while ((rc = playback_getline(&linebuf, &buflen, fd)) > 0)
	     {
#ifdef DEBUG
		     printf("RECV(%d): %s\n", rc, linebuf);
#endif
		     if( !strncmp("Content-Length: ", linebuf, 16))
		     {
		        long int pos;
			
			sscanf(linebuf+16,"%ld",&pos);
			context->playback->size=pos+off;
		     }
	     }

#ifdef DEBUG
	     printf("linebuf %p\n", linebuf);
#endif
	   break;
	   case 301: /* moved */
              printf("%s: error: file has been moved\n", __func__);
	      close(fd);
	      free(linebuf);
              return -1;
	   break;

	   case 302: /* found; tmp redirecting */
              /* this means the resource is tmp on another uri but
	       * later we must use the orig uri.
	       */
              printf("%s: error: temp resource redirecting not supported\n", __func__);
	      close(fd);
	      free(linebuf);
              return -1;
           break;  
	   case 303: /* see other; redirecting */
#ifdef DEBUG
		printf("%s: status redirecting\n", __func__);
		printf("%s: got: %s\n", __func__, linebuf);
#endif
		while ((rc = playback_getline(&linebuf, &buflen, fd)) > 0)
		{
	                int ret;
			
			if(!strncmp("Location: ", linebuf, 10)){
				close(fd);
				free(context->playback->uri);
				context->playback->uri=strdup(linebuf+10);
			
			        /* konfetti: otherwise we loose the linebuf
				 * allocation every recursion. (leek return)
				 */
			        ret = openUPNPConnection(context, off);
			        free(linebuf);
				return ret;
			}
#ifdef DEBUG
			printf("%s: RECV(%d): %s\n", __func__, rc, linebuf);
#endif
		}
		free(linebuf);
		close(fd);
		return -1;
	   
	   break;

	   case 401: /* unauthorized */
              printf("%s: error: not authorized\n", __func__);
	      close(fd);
	      free(linebuf);
              return -1;
	   break;

	   case 403: /* forbidden */
              printf("%s: error: forbidden\n", __func__);
	      close(fd);
	      free(linebuf);
              return -1;
	   break;

	   case 404: /* file not found */
              printf("%s: error: file not found\n", __func__);
	      close(fd);
	      free(linebuf);
              return -1;
	   break;
	   default:
              printf("%s: error: unexpected status code %d\n", __func__, statuscode);
/* not sure if we should close here?	      close(fd);*/
	      free(linebuf);
              return -1;
	   break;
	}

	free(linebuf);
	printf("%s < connection established\n", __func__);
	return fd;
}


static int openMmsConnection(char* url, char** content)
{
	char host[256];
	int port = 80;
	char uri[512];
	int x;
	int slash = 0;
	for(x=6;x<strlen(url)-1;x++){
		if(url[x]=='/'){
			slash=x;
			break;
		}
	}
	if (slash > 0) {
		strncpy(host,url+6,slash-6);
		host[slash-6]='\0';
		strcpy(uri,url+slash);
	} else {
		strcpy(host,url+6);
		strcpy(uri,"/");
	}
	int dp=0;
	for(x=0;x<strlen(host)-1;x++){
		if(host[x]==':'){
			dp=x;
			break;
		}
	}
	if (dp > 0) {
		port = atoi(host+dp+1);
		//host = host.substr(0, dp);
		host[dp]='\0';
	}
#ifdef DEBUG
	printf("URL: %s\n",url);
	printf("Host: %s\n",host);
	printf("URI: %s\n",uri);
	printf("Port: %d\n",port);
#endif

	struct hostent* h = gethostbyname(host);
	if (h == NULL || h->h_addr_list == NULL){
		printf("hotlookup failed\n");
		return 0;
	}
	int fd = socket(PF_INET, SOCK_STREAM, 0);
	if (fd == -1){
		printf("creating socket failed");
		return 0;
	}
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = *((in_addr_t*)h->h_addr_list[0]);
	addr.sin_port = htons(port);

#ifdef DEBUG
	printf("connecting to %s\n", url);
#endif

	if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		printf("connect failed for: %s\n", url);
		return 0;
	}
#ifdef DEBUG
	printf("connected!\n");
#endif
	char request[1024];
	strcpy(request,"GET ");
	strcat(request,uri);
	strcat(request," HTTP/1.1\n");
	strcat(request,"Host: ");
	strcat(request,host);
	strcat(request,"\n");
	strcat(request,"Accept: */*\n");
	strcat(request,"Connection: close\n");
	strcat(request,"\n");
#ifdef DEBUG
	printf("sending:\n%s",request);
#endif
	write(fd, request, strlen(request));

	int rc;
	size_t buflen = 1000;
	char* linebuf = (char*)malloc(1000);

	rc = playback_getline(&linebuf, &buflen, fd);
#ifdef DEBUG
	printf("RECV(%d): %s\n", rc, linebuf);
#endif
	if (rc <= 0)
	{
		close(fd);
		free(linebuf);
		return 0;
	}

	char proto[100];
	int statuscode = 0;
	char statusmsg[100];
	rc = sscanf(linebuf, "%99s %d %99s", proto, &statuscode, statusmsg);
	if (rc != 3 || statuscode != 200) {
		printf("wrong response: \"200 OK\" expected.\n");
#ifdef DEBUG
		printf("got: %s\n",linebuf);
#endif
		free(linebuf);
		close(fd);
		return 0;
	}	
#ifdef DEBUG
	printf("proto=%s, code=%d, msg=%s\n", proto, statuscode, statusmsg);
#endif
	while (rc > 0)
	{
		rc = playback_getline(&linebuf, &buflen, fd);
#ifdef DEBUG
		printf("RECV(%d): %s\n", rc, linebuf);
#endif
		if(rc == 24) { //ContentType
			int ret = 0;
#ifdef DEBUG
			printf("%s: Content-Type: ", __func__);
#endif
			if(!(ret = strncmp("audio/mpeg", linebuf+14, 10))) {
#ifdef DEBUG
				printf("mp3");
				printf("strdup in %s::%s:%d\n", FILENAME, __FUNCTION__,__LINE__);
#endif
				*content = strdup("mp3");
			} else  {
				printf(" Unknown (%s) %d", linebuf, ret);
			}
			printf("\n");
		}
	}
	free(linebuf);

	return fd;
}

static void getExtension(char * FILENAMEname, char ** extension) {

	int i = 0;
	int stringlength = (int) strlen(FILENAMEname);
#ifdef DEBUG
	printf("%s::%s\n", FILENAME, __FUNCTION__);
#endif
	for (i = 0; stringlength - i > 0; i++) {
		if (FILENAMEname[stringlength - i - 1] == '.') {
#ifdef DEBUG
			printf("strdup in %s::%s:%d\n", FILENAME, __FUNCTION__,__LINE__);
#endif
			*extension = strdup(FILENAMEname+(stringlength - i));
			break;
		}
	}
}

static void getUPNPExtension(char * FILENAMEname, char ** extension) {
    char* str = strstr(FILENAMEname, "ext=");
    
    if (str != NULL)
    {
       *extension = strdup(str + strlen("ext=") + 1);
       printf("%s: extension found %s\n", __func__, *extension);
       return;
    }
    printf("%s: no extension found\n", __func__);
    *extension = NULL;
}

static void getParentFolder(char * Filename, char ** folder) {
#ifdef DEBUG
	printf("%s::%s\n", FILENAME, __FUNCTION__);
#endif

	int i = 0;
	int stringlength = (int) strlen(Filename);

	for (i = 0; stringlength - i > 0; i++) {
		if (Filename[stringlength - i - 1] == '/') {
            *folder = (char *)malloc(stringlength - i - 1);
			strncpy(*folder, Filename, stringlength - i - 1);
			break;
		}
	}
}

//konfetti: explanation on wrong handling:
//pthread_t is not a pointer type and cannot set to NULL
//as it is done in PlaybackOpen. This may work but is very unproper

static pthread_t DummyThread1;
static unsigned char isThread1Created = 0; 
static pthread_t DummyThread2;
static unsigned char isThread2Created = 0; 
static pthread_t DummyThread3;
static unsigned char isThread3Created = 0; 

static void DummyThread(Context_t *context) {
#ifdef DEBUG
	printf("dummy thread started\n");
#endif	
	usleep(10000);
}

static int PlaybackOpen(Context_t  *context, char * uri) {
#ifdef DEBUG
	printf("%s::%s URI=%s\n", FILENAME, __FUNCTION__, uri);
#endif
	/* make sure that 3 thread are created
	   if the first file we play is a file without subtitles (no subtitles thread is created)
	   on playback of the second file pthread_create fails with cannot allocate memory
	   because we did not create the subtitle thread 
	   so we need to make sure that we created 3 threads before starting the first file.
	   this is a quick and dirty fix and needs to be investigated if there is a better solution*/
	int error;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        if((error = pthread_create(&DummyThread1, &attr, (void *)&DummyThread, NULL)) != 0)
        {
           fprintf(stderr, "Error creating thread in %s error:%d:%s\n", __FUNCTION__,error,strerror(error));
           isThread1Created = 0; 
        } else
           isThread1Created = 1; 
	
	if((error = pthread_create(&DummyThread2, &attr, (void *)&DummyThread, NULL)) != 0)
        {
           fprintf(stderr, "Error creating thread in %s error:%d:%s\n", __FUNCTION__,error,strerror(error));
           isThread2Created = 0; 
        } else
           isThread2Created = 1; 

	if((error = pthread_create(&DummyThread3, &attr, (void *)&DummyThread, NULL)) != 0)
        {
           fprintf(stderr, "Error creating thread in %s error:%d:%s\n", __FUNCTION__,error,strerror(error));
           isThread3Created = 0; 
        } else
           isThread3Created = 1; 
#ifdef DEBUG
	printf("joining dummy threads\n");
#endif
	if(isThread1Created) error = pthread_join (DummyThread1, NULL);
	if(isThread2Created) error = pthread_join (DummyThread2, NULL);
	if(isThread3Created) error = pthread_join (DummyThread3, NULL);

	context->playback->uri=strdup(uri);

    if (!context->playback->isPlaying) {
	    if (!strncmp("file://", uri, 7)) {
		    char * extension = NULL;
		    context->playback->isFile = 1;
		    context->playback->isHttp = 0;
		    context->playback->isUPNP = 0;

		    context->playback->fd = open(uri+7, O_RDONLY);
		    if(context->playback->fd == -1) {
#ifdef DEBUG
			    printf("context->playback->fd == -1!\n");
#endif
			    return -1;
		    }

		    getExtension(uri+7, &extension);
		    if(!extension) return -1;
		
		    if(context->container->Command(context, CONTAINER_ADD, extension) < 0)
			    return -1;
		    if (context->container->selectedContainer != NULL) {
			    if(context->container->selectedContainer->Command(context, CONTAINER_INIT, uri+7) < 0)
				    return -1;
		    } else {
			    return -1;
		    }

		    free(extension);

		    //CHECK FOR SUBTITLES
            if (context->container && context->container->textSrtContainer)
                context->container->textSrtContainer->Command(context, CONTAINER_INIT, uri+7);
	    //if(context->playback->isSubtitle<=0)
            	if (context->container && context->container->textSsaContainer)
                	context->container->textSsaContainer->Command(context, CONTAINER_INIT, uri+7);

	    } else if (!strncmp("http://", uri, 7)) {
		    char * extension = NULL;
		    context->playback->isFile = 0;
		    context->playback->isHttp = 1;
		    context->playback->isUPNP = 0;
		
		    context->playback->fd = openHttpConnection(context, &extension,0);
#ifdef DEBUG
		    printf("context->playback->fd = %d\n", context->playback->fd);
#endif
		    if(context->playback->fd == 0) {
#ifdef DEBUG
			    printf("context->playback->fd == 0!\n");
#endif
			    return -1;
		    }

		    if(!extension) getExtension(uri+7, &extension);
		    if(!extension) return -1;
		
		    if(context->container->Command(context, CONTAINER_ADD, extension) < 0)
			    return -1;
		    if (context->container->selectedContainer != NULL) {
			    if(context->container->selectedContainer->Command(context, CONTAINER_INIT, uri+7) < 0)
				    return -1;
		    } else {
			    return -1;
		    }
			
		    free(extension);	
        } /* http */ 
	else if (!strncmp("mms://", uri, 6)) {		
            char * extension = NULL;
		    context->playback->isFile = 0;
		    context->playback->isHttp = 1;
		    context->playback->isUPNP = 0;
		
		    context->playback->fd = openMmsConnection(uri, &extension);
#ifdef DEBUG
		    printf("context->playback->fd = %d\n", context->playback->fd);
#endif
		    if(context->playback->fd == 0) {
#ifdef DEBUG
			    printf("context->playback->fd == 0!\n");
#endif
			    return -1;
		    }

		    if(!extension) getExtension(uri+6, &extension);
		    if(!extension) return -1;
		
		    if(context->container->Command(context, CONTAINER_ADD, extension) < 0)
			    return -1;
		    if (context->container->selectedContainer != NULL) {
			    if(context->container->selectedContainer->Command(context, CONTAINER_INIT, uri+6) < 0)
				    return -1;
		    } else {
			    return -1;
		    }
			
		    free(extension);	
	    } /* mms */
	    else
	    if (!strncmp("upnp://", uri, 7)) {
		    char * extension = NULL;
		    context->playback->isFile = 0;
		    context->playback->isHttp = 0;
		    context->playback->isUPNP = 1;

		    context->playback->uri += 7; /* jump over upnp:// */

		    context->playback->fd = openUPNPConnection(context,0);

		    if(context->playback->fd <= 0) {
			    printf("context->playback->fd == -1!\n");
			    return -1;
		    } else
		       printf("%s: file is open at %d\n", __func__, context->playback->fd);

		    getUPNPExtension(uri+7, &extension);
		    if(!extension) return -1;

		    if(context->container->Command(context, CONTAINER_ADD, extension) < 0)
		    {
			    printf("%s: container CONTAINER_ADD failed\n", __func__);
			    return -1;
		    }
		    if (context->container->selectedContainer != NULL) {
			    if(context->container->selectedContainer->Command(context, CONTAINER_INIT, uri+7) < 0)
		            {
			        printf("%s: container CONTAINER_INIT failed\n", __func__);
				return -1;
		            }		
		    } else {
			printf("%s: selected container is null\n", __func__);
			    return -1;
		    }

		    free(extension);

            } /* upnp */	    
	     else {
		    printf("Unknown stream!\n");
		    return -1;
	    }
    } else 
        return -1;

	return 0;
}

static int PlaybackClose(Context_t  *context) {
#ifdef DEBUG
				printf("%s::%s \n", FILENAME, __FUNCTION__);
#endif

        context->container->Command(context, CONTAINER_DEL, NULL);

//FIXME KILLED BY signal 7 or 11
        if (context->container && context->container->textSrtContainer)
            context->container->textSrtContainer->Command(context, CONTAINER_DEL, NULL);

        if (context->container && context->container->textSsaContainer)
            context->container->textSsaContainer->Command(context, CONTAINER_DEL, NULL);

        context->manager->audio->Command(context, MANAGER_DEL, NULL);
        context->manager->video->Command(context, MANAGER_DEL, NULL);
        context->manager->subtitle->Command(context, MANAGER_DEL, NULL);

        close(context->playback->fd);

	context->playback->isPaused     = 0;
	context->playback->isPlaying    = 0;
	context->playback->isForwarding = 0;
	context->playback->BackWard     = 0;
	context->playback->SlowMotion   = 0;
	context->playback->Speed        = 0;

	return 0;
}

static int PlaybackPlay(Context_t  *context) {
	int ret = 0;
#ifdef DEBUG
	printf("%s::%s\n", FILENAME, __FUNCTION__);
#endif

	if (!context->playback->isPlaying) {
		context->playback->AVSync = 1;
		context->output->Command(context, OUTPUT_AVSYNC, NULL);

		context->playback->isCreationPhase = 1;	// allows the created thread to go into wait mode
		ret = context->output->Command(context, OUTPUT_PLAY, NULL);

		if (ret != 0) {
#ifdef DEBUG
			printf("%s::%s OUTPUT_PLAY failed!\n", FILENAME, __FUNCTION__);
#endif
#ifdef DEBUG
			printf("%s::%s clearing isCreationPhase!\n", FILENAME, __FUNCTION__);
#endif
			context->playback->isCreationPhase = 0;	// allow thread to go into next state
		} else {
			context->playback->isPlaying    = 1;
			context->playback->isPaused     = 0;
			context->playback->isForwarding = 0;
			context->playback->BackWard     = 0;
			context->playback->SlowMotion   = 0;
			context->playback->Speed        = 1;

#ifdef DEBUG
			printf("%s::%s clearing isCreationPhase!\n", FILENAME, __FUNCTION__);
#endif
			context->playback->isCreationPhase = 0;	// allow thread to go into next state

			ret = context->container->selectedContainer->Command(context, CONTAINER_PLAY, NULL);
			if (ret != 0) {
#ifdef DEBUG
				printf("%s::%s CONTAINER_PLAY failed!\n", FILENAME, __FUNCTION__);
#endif
			}
		
		}

	} else
		ret = -1;
	
#ifdef DEBUG
	printf("%s::%s exiting with value %d\n", FILENAME, __FUNCTION__, ret);
#endif
	
	return ret;
}

static int PlaybackPause(Context_t  *context) {
#ifdef DEBUG
	printf("%s::%s\n", FILENAME, __FUNCTION__);
#endif
	if (context->playback->isPlaying && !context->playback->isPaused) {

		if(context->playback->SlowMotion)
			context->output->Command(context, OUTPUT_CLEAR, NULL);

		context->output->Command(context, OUTPUT_PAUSE, NULL);

		context->playback->isPaused     = 1;
		//context->playback->isPlaying  = 1;
		context->playback->isForwarding = 0;
		context->playback->BackWard     = 0;
		context->playback->SlowMotion   = 0;
		context->playback->Speed        = 1;
	} else
        return -1;

	return 0;
}

static int PlaybackContinue(Context_t  *context) {
#ifdef DEBUG
	printf("%s::%s\n", FILENAME, __FUNCTION__);
#endif
	if (context->playback->isPlaying && 
        (context->playback->isPaused || context->playback->isForwarding || context->playback->BackWard || context->playback->SlowMotion)) {

		if(context->playback->SlowMotion)
			context->output->Command(context, OUTPUT_CLEAR, NULL);

		context->output->Command(context, OUTPUT_CONTINUE, NULL);

		context->playback->isPaused     = 0;
		//context->playback->isPlaying  = 1;
		context->playback->isForwarding = 0;
		context->playback->BackWard     = 0;
		context->playback->SlowMotion   = 0;
		context->playback->Speed        = 1;
	} else
        return -1;

	return 0;
}

static int PlaybackStop(Context_t  *context) {
#ifdef DEBUG
	printf("%s::%s\n", FILENAME, __FUNCTION__);
#endif
	if (context->playback->isPlaying) {

		context->playback->isPaused     = 0;
		context->playback->isPlaying    = 0;
		context->playback->isForwarding = 0;
		context->playback->BackWard     = 0;
		context->playback->SlowMotion   = 0;
		context->playback->Speed        = 0;

		context->output->Command(context, OUTPUT_STOP, NULL);
		context->container->selectedContainer->Command(context, CONTAINER_STOP, NULL);

        //PlaybackClose(context);
	} else
        return -1;

	return 0;
}

static int PlaybackTerminate(Context_t  *context) {
#ifdef DEBUG
	printf("%s::%s\n", FILENAME, __FUNCTION__);
#endif	
	int ret = 0;
	
	if ( context && context->playback && context->playback->isPlaying ) {
        //First Flush and than delete container, else e2 cant read length of file anymore
		context->output->Command(context, OUTPUT_FLUSH, NULL);
		ret = context->container->selectedContainer->Command(context, CONTAINER_STOP, NULL);

		context->playback->isPaused     = 0;
		context->playback->isPlaying    = 0;
		context->playback->isForwarding = 0;
		context->playback->BackWard     = 0;
		context->playback->SlowMotion   = 0;
		context->playback->Speed        = 0;

        //PlaybackClose(context);
	} else
		ret = -1;

#ifdef DEBUG
	printf("%s::%s exiting with value %d\n", FILENAME, __FUNCTION__, ret);
#endif
	
	return ret;
}

static int PlaybackFastForward(Context_t  *context,int* speed) {


    //Audio only forwarding not supported
	if (context->playback->isVideo && !context->playback->isHttp && !context->playback->BackWard && (!context->playback->isPaused || context->playback->isPlaying)) {

		context->playback->isForwarding = 1;

		switch(*speed) {
		case 2: 
			context->playback->Speed = 1;
			break;
		case 4:
			context->playback->Speed = 2;
			break;
		case 8:
			context->playback->Speed = 3;
			break;
		case 48: //16x
			context->playback->Speed = 4;
			break;
		case 96: //32x
			context->playback->Speed = 5;
			break;
		case 192: //64x
			context->playback->Speed = 6;
			break;
		case 384: //128x
			context->playback->Speed = 7;
			break;
	}

#ifdef DEBUG
	printf("%s::%s Speed: %d x {%d}\n", FILENAME, __FUNCTION__, *speed, context->playback->Speed);
#endif

		context->output->Command(context, OUTPUT_FASTFORWARD, NULL);
	} else
        return -1;
	return 0;
}

static pthread_t FBThread;
/* konfetti: see below */
static unsigned char isFBThreadStarted = 0;

static void FastBackwardThread(Context_t *context)
{
#ifdef DEBUG
	printf("%s::%s\n", FILENAME, __FUNCTION__);
#endif

	context->output->Command(context, OUTPUT_AUDIOMUTE, "1");
	while(context->playback && context->playback->isPlaying && context->playback->BackWard)
	{
		context->playback->isSeeking = 1;
		context->output->Command(context, OUTPUT_CLEAR, NULL);
		context->output->Command(context, OUTPUT_PAUSE, NULL);
		context->output->Command(context, OUTPUT_CLEAR, NULL);
		context->container->selectedContainer->Command(context, CONTAINER_SEEK, &context->playback->BackWard);
		context->output->Command(context, OUTPUT_CLEAR, NULL);
		context->playback->isSeeking = 0;
		context->output->Command(context, OUTPUT_CONTINUE, NULL);
		
		//context->container->selectedContainer->Command(context, CONTAINER_SEEK, &context->playback->BackWard);
		//context->output->Command(context, OUTPUT_CLEAR, "video");
		usleep(500000);
	}
	//context->output->Command(context, OUTPUT_CLEAR, NULL);
	context->output->Command(context, OUTPUT_AUDIOMUTE, "0");
        isFBThreadStarted = 0;
#ifdef DEBUG
	printf("%s::%s exit\n", FILENAME, __FUNCTION__);
#endif
}

static int PlaybackFastBackward(Context_t  *context,int* speed) {

	//Audio only backwarding not supported
	if (context->playback->isVideo && !context->playback->isHttp && !context->playback->isForwarding && (!context->playback->isPaused || context->playback->isPlaying)) {
		context->playback->BackWard = 0;

		switch(*speed) {
		case 8:
			context->playback->BackWard = -8;
			break;
		case 16:
			context->playback->BackWard = -16;
			break;
		case 32:
			context->playback->BackWard = -32;
			break;
		case 64:
			context->playback->BackWard = -64;
			break;
		case 128:
			context->playback->BackWard = -128;
			break;
		default:
			return -1;
        	}

#ifdef DEBUG
		printf("%s::%s Speed: %d x {%f}\n", FILENAME, __FUNCTION__, *speed, context->playback->BackWard);
#endif

		int error;
		pthread_attr_t attr;

		if(!isFBThreadStarted)
		{
			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

			if((error = pthread_create(&FBThread, &attr, (void *)&FastBackwardThread, context)) != 0)
			{
				fprintf(stderr, "Error creating thread in %s error:%d:%s\n", __FUNCTION__,error,strerror(error));
                                isFBThreadStarted = 0;
				return -1;
			} else
                                isFBThreadStarted = 1;
			
		}
	} else
		return -1;
	return 0;
}

static int PlaybackSlowMotion(Context_t  *context,int* speed) {


    //Audio only forwarding not supported
	if (context->playback->isVideo && !context->playback->isHttp && context->playback->isPlaying) {
		if(context->playback->isPaused)
			PlaybackContinue(context);

		switch(*speed) {
		case 2: 
			context->playback->SlowMotion = 2;
			break;
		case 4:
			context->playback->SlowMotion = 4;
			break;
		case 8:
			context->playback->SlowMotion = 8;
			break;
	}

#ifdef DEBUG
	printf("%s::%s SlowMotion: %d x {%d}\n", FILENAME, __FUNCTION__, *speed, context->playback->SlowMotion);
#endif

		context->output->Command(context, OUTPUT_SLOWMOTION, NULL);
	} else
        return -1;
	return 0;
}

static int PlaybackSeek(Context_t  *context, float * pos) {
#ifdef DEBUG
	printf("%s::%s pos: %f\n", FILENAME, __FUNCTION__, *pos);
#endif

	if (!context->playback->isHttp && context->playback->isPlaying && !context->playback->isForwarding && !context->playback->BackWard && !context->playback->SlowMotion && !context->playback->isPaused) {
		context->playback->isSeeking = 1;
		context->output->Command(context, OUTPUT_CLEAR, NULL);
		PlaybackPause(context);
		context->output->Command(context, OUTPUT_CLEAR, NULL);
		context->container->selectedContainer->Command(context, CONTAINER_SEEK, pos);
		context->output->Command(context, OUTPUT_CLEAR, NULL);
		context->playback->isSeeking = 0;

		PlaybackContinue(context);
	} else
        return -1;
	
	return 0;
}

static int PlaybackPts(Context_t  *context, unsigned long long int* pts) {
#ifdef DEBUG  
	printf("%s::%s\n", FILENAME, __FUNCTION__);
#endif	

	int ret = 0;
	
	*pts = 0;

	if (context->playback->isPlaying) {
		context->output->Command(context, OUTPUT_PTS, pts);
	} else
		ret = -1;
		
#ifdef DEBUG
	printf("%s::%s exiting with value %d\n", FILENAME, __FUNCTION__, ret);
#endif
	
	return ret;
}

static int PlaybackLength(Context_t  *context, double* length) {
#ifdef DEBUG
		printf("%s::%s\n", FILENAME, __FUNCTION__);
#endif

    *length = 0;

	if (context->playback->isPlaying) {
		if (context->container && context->container->selectedContainer)
		    context->container->selectedContainer->Command(context, CONTAINER_LENGTH, length);
	} else
        return -1;
		
	
	return 0;
}

static int PlaybackSwitchAudio(Context_t  *context, int* track) {
#ifdef DEBUG
	printf("%s::%s\n", FILENAME, __FUNCTION__);
#endif

    int curtrackid = 0;
    int nextrackid = 0;

	if (context->playback->isPlaying) {
        if (context->manager && context->manager->audio) {
            context->manager->audio->Command(context, MANAGER_GET, &curtrackid);
            context->manager->audio->Command(context, MANAGER_SET, track);
            context->manager->audio->Command(context, MANAGER_GET, &nextrackid);
        }

        if(nextrackid != curtrackid) {

	    //PlaybackPause(context);

            if (context->output && context->output->audio)
	            context->output->audio->Command(context, OUTPUT_SWITCH, (void*)"audio");

	    if (context->container && context->container->selectedContainer)
	            context->container->selectedContainer->Command(context, CONTAINER_SWITCH_AUDIO, &nextrackid);

            //PlaybackContinue(context);
        }
	} else
        return -1;
		
	
	return 0;
}

static int PlaybackSwitchSubtitle(Context_t  *context, int* track) {
#ifdef DEBUG  
	printf("%s::%s Track: %d\n", FILENAME, __FUNCTION__, *track);
#endif	

	int curtrackid = 0;
	int nextrackid = 0;
	int ret = 0;

	if (context && context->playback && context->playback->isPlaying ) {
		if (context->manager && context->manager->subtitle) {
			context->manager->subtitle->Command(context, MANAGER_GET, &curtrackid);
			context->manager->subtitle->Command(context, MANAGER_SET, track);
			context->manager->subtitle->Command(context, MANAGER_GET, &nextrackid);

#ifdef DEBUG  
			printf("%s::%s nextrackid: %d != curtrackid: %d\n", FILENAME, __FUNCTION__, nextrackid, curtrackid);
#endif

			if(nextrackid != curtrackid) {

				//PlaybackPause(context);

				if (context->container && context->container->selectedContainer) {
					context->container->selectedContainer->Command(context, CONTAINER_SWITCH_SUBTITLE, &nextrackid);

					if(nextrackid==100){
						if (context->container && context->container->textSrtContainer)
							context->container->textSrtContainer->Command(context, CONTAINER_SWITCH_SUBTITLE, &nextrackid);
					}
					else if(nextrackid==200){
						if (context->container && context->container->textSsaContainer)
							context->container->textSsaContainer->Command(context, CONTAINER_SWITCH_SUBTITLE, &nextrackid);
					}
					
					if (context->output && context->output->subtitle)
						context->output->subtitle->Command(context, OUTPUT_SWITCH, (void*)"subtitle");
					else
						ret = -1;
				} else
					ret = -1;
				
				//PlaybackContinue(context);
			}
			   
		} else
			ret = -1;
	} else
		ret = -1;
	
#ifdef DEBUG
	printf("%s::%s exiting with value %d\n", FILENAME, __FUNCTION__, ret);
#endif

	return ret;
}

static int PlaybackInfo(Context_t  *context, char** infoString) {
#ifdef DEBUG	
	printf("%s::%s\n", FILENAME, __FUNCTION__);
#endif

	int ret = 0;

	if (context->playback->isPlaying) {
		if (context->container && context->container->selectedContainer)
		    context->container->selectedContainer->Command(context, CONTAINER_INFO, infoString);
	} else
		ret = -1;

#ifdef DEBUG
	printf("%s::%s exiting with value %d\n", FILENAME, __FUNCTION__, ret);
#endif
	return ret;
}

static int Command(void* _context, PlaybackCmd_t command, void * argument) {
Context_t* context = (Context_t*) _context; /* to satisfy compiler */
#ifdef DEBUG
	printf("%s::%s Command %d\n", FILENAME, __FUNCTION__, command);
#endif
	int ret = -1;

	switch(command) {
		case PLAYBACK_OPEN: {
			ret = PlaybackOpen(context, (char*)argument);
			break;
		}
		case PLAYBACK_CLOSE: {
			ret = PlaybackClose(context);
			break;
		}
		case PLAYBACK_PLAY: {
			ret = PlaybackPlay(context);
			break;
		}
		case PLAYBACK_STOP: {
			ret = PlaybackStop(context);
			break;
		}
		case PLAYBACK_PAUSE: {	// 4
			ret = PlaybackPause(context);
			break;
		}
		case PLAYBACK_CONTINUE: {
			ret = PlaybackContinue(context);
			break;
		}
		case PLAYBACK_TERM: {
			ret = PlaybackTerminate(context);
			break;
		}
		case PLAYBACK_FASTFORWARD: {
			ret = PlaybackFastForward(context,(int*)argument);
			break;
		}
		case PLAYBACK_SEEK: {
			ret = PlaybackSeek(context, (float*)argument);
			break;
		}
		case PLAYBACK_PTS: { // 10
			ret = PlaybackPts(context, (unsigned long long int*)argument);
			break;
		}
		case PLAYBACK_LENGTH: { // 11
			ret = PlaybackLength(context, (double*)argument);
			break;
		}
		case PLAYBACK_SWITCH_AUDIO: {
			ret = PlaybackSwitchAudio(context, (int*)argument);
			break;
		}
		case PLAYBACK_SWITCH_SUBTITLE: {
			ret = PlaybackSwitchSubtitle(context, (int*)argument);
			break;
		}
		case PLAYBACK_INFO: {
			ret = PlaybackInfo(context, (char**)argument);
			break;
		}
		case PLAYBACK_SLOWMOTION: {
			ret = PlaybackSlowMotion(context,(int*)argument);
			break;
		}
		case PLAYBACK_FASTBACKWARD: {
			ret = PlaybackFastBackward(context,(int*)argument);
			break;
		}
		default:
#ifdef DEBUG
			printf("%s::%s PlaybackCmd %d not supported!\n", FILENAME, __FUNCTION__, command);
#endif
			break;
	}

#ifdef DEBUG
	printf("%s::%s exiting with value %d\n", FILENAME, __FUNCTION__, ret);
#endif

	return ret;
}


PlaybackHandler_t PlaybackHandler = {
	"Playback",
	-1,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	&Command,
	"",
	0,
};
