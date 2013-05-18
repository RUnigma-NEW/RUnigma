#include <stdio.h> 
#include <string.h>
#include <stdlib.h> 
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <sys/ioctl.h>
#include <linux/dvb/video.h>
#include <linux/dvb/audio.h>
#include <memory.h>
#include <asm/types.h>
#include <pthread.h>
#include <errno.h>

#include "common.h"
#include "output.h"
#include "stm_ioctls.h"

/* #define DEBUG */

#ifdef DEBUG
int debugdvb = 0;
#define dprintf(x...) do { if (debugdvb)printf(x); } while (0)
#endif

static const char FILENAME[] 	= "linuxdvb.c";

static const char VIDEODEV[] 	= "/dev/dvb/adapter0/video0";
static const char AUDIODEV[] 	= "/dev/dvb/adapter0/audio0";
static const char DVRDEV[] 	= "/dev/dvb/adapter0/dvr0";

static int videofd 	= -1;
static int audiofd 	= -1;
static int dvrfd 	= -1;

static int LinuxDvbPtsStart(Context_t *context);
static pthread_t PtsThread;
static unsigned char isPTSThreadCreated = 0;

unsigned long long int sCURRENT_PTS = 0;
static const unsigned int cSLEEPTIME = 500000;

pthread_mutex_t LinuxDVBmutex;

void getLinuxDVBMutex(const char *filename, const char *function, int line) {
#ifdef DEBUG
	printf("%s::%s::%d requesting mutex\n",filename, function, line);
#endif
	pthread_mutex_lock(&LinuxDVBmutex);
#ifdef DEBUG
	printf("%s::%s::%d received mutex\n",filename, function, line);
#endif  
}

void releaseLinuxDVBMutex(const char *filename, const char *function, int line) {
	pthread_mutex_unlock(&LinuxDVBmutex);
#ifdef DEBUG
	printf("%s::%s::%d released mutex\n", filename, function, line);
#endif  
}

int LinuxDvbOpen(Context_t  *context, char * type) {
	unsigned char video = !strcmp("video", type);
	unsigned char audio = !strcmp("audio", type);

#ifdef DEBUG
	printf("%s::%s v%d a%d\n", FILENAME, __FUNCTION__, video, audio);	
#endif

	if (video && videofd == -1) {
		videofd = open(VIDEODEV, O_RDWR);

        	if (videofd < 0)
        	{
		   printf("%s::%s %s\n", FILENAME, __FUNCTION__, strerror(errno));	
        	   return -1;
        	}

    		ioctl( videofd, VIDEO_CLEAR_BUFFER ,0);
		ioctl( videofd, VIDEO_SELECT_SOURCE, (void*)VIDEO_SOURCE_MEMORY);
		ioctl( videofd, VIDEO_SET_STREAMTYPE, (void*)STREAM_TYPE_PROGRAM);

	}
	if (audio && audiofd == -1) {
		audiofd = open(AUDIODEV, O_RDWR);

        	if (audiofd < 0)
        	{
		   printf("%s::%s %s\n", FILENAME, __FUNCTION__, strerror(errno));	
        	   return -1;
        	}

    		ioctl( audiofd, AUDIO_CLEAR_BUFFER,0);
		ioctl( audiofd, AUDIO_SELECT_SOURCE, (void*)AUDIO_SOURCE_MEMORY);
		ioctl( audiofd, AUDIO_SET_STREAMTYPE, (void*)STREAM_TYPE_PROGRAM);
	}
	if ((video || audio) && dvrfd == -1)
		dvrfd = open(DVRDEV, O_RDWR);

	return 0;
}

int LinuxDvbClose(Context_t  *context, char * type) {
	unsigned char video = !strcmp("video", type);
	unsigned char audio = !strcmp("audio", type);

#ifdef DEBUG
	printf("%s::%s v%d a%d\n", FILENAME, __FUNCTION__, video, audio);
#endif

	if (video && videofd != -1) {
		close(videofd);
		videofd = -1;
	}
	if (audio && audiofd != -1) {
		close(audiofd);
		audiofd = -1;
	}
	if ((video || audio) && dvrfd != -1) {
		close(dvrfd);
		dvrfd 	= -1;
	}

	return 0;
}

int LinuxDvbPlay(Context_t  *context, char * type) {
	int ret = -1;
		
	unsigned char video = !strcmp("video", type);
	unsigned char audio = !strcmp("audio", type);

#ifdef DEBUG
	printf("%s::%s v%d a%d\n", FILENAME, __FUNCTION__, video, audio);
#endif

	//if (!context->playback->isPlaying) {
		if (video && videofd != -1) {
			char * Encoding = NULL;
			context->manager->video->Command(context, MANAGER_GETENCODING, &Encoding);

#ifdef DEBUG
			printf("%s::%s V %s\n", FILENAME, __FUNCTION__, Encoding);
#endif
			if(!strcmp (Encoding, "V_MPEG2"))
				ioctl( videofd, VIDEO_SET_ENCODING, (void*)VIDEO_ENCODING_AUTO);
			else if(!strcmp (Encoding, "V_MSCOMP") || !strcmp (Encoding, "V_MS/VFW/FOURCC") || !strcmp (Encoding, "V_MKV/XVID"))
				ioctl( videofd, VIDEO_SET_ENCODING, (void*)VIDEO_ENCODING_MPEG4P2);
 			else if(!strcmp (Encoding, "V_MPEG4/ISO/AVC") || !strcmp (Encoding, "V_MPEG2/H264"))
				ioctl( videofd, VIDEO_SET_ENCODING, (void*)VIDEO_ENCODING_H264);
			else if(!strcmp (Encoding, "V_WMV"))
				ioctl( videofd, VIDEO_SET_ENCODING, (void*)VIDEO_ENCODING_WMV);
 			else
				ioctl( videofd, VIDEO_SET_ENCODING, (void*)VIDEO_ENCODING_AUTO);

			ioctl(videofd, VIDEO_PLAY, NULL);
			free(Encoding);
		}
		if (audio && audiofd != -1) {
			char * Encoding = NULL;
			context->manager->audio->Command(context, MANAGER_GETENCODING, &Encoding);

#ifdef DEBUG
			printf("%s::%s A %s\n", FILENAME, __FUNCTION__, Encoding);
#endif
			if(!strcmp (Encoding, "A_AC3"))
				ioctl( audiofd, AUDIO_SET_ENCODING, (void*)AUDIO_ENCODING_AC3);
			else if(!strcmp (Encoding, "A_MP3") || !strcmp (Encoding, "A_MPEG/L3") || !strcmp (Encoding, "A_MS/ACM"))
				ioctl( audiofd, AUDIO_SET_ENCODING, (void*)AUDIO_ENCODING_MP3);
 			else if(!strcmp (Encoding, "A_DTS"))
				ioctl( audiofd, AUDIO_SET_ENCODING, (void*)AUDIO_ENCODING_DTS);
			else if(!strcmp (Encoding, "A_AAC"))
				ioctl( audiofd, AUDIO_SET_ENCODING, (void*)AUDIO_ENCODING_AAC);
			else if(!strcmp (Encoding, "A_WMA"))
				ioctl( audiofd, AUDIO_SET_ENCODING, (void*)AUDIO_ENCODING_WMA);
           		else
                		ioctl( audiofd, AUDIO_SET_ENCODING, (void*)AUDIO_ENCODING_AUTO);

			ioctl(audiofd, AUDIO_PLAY, NULL);
			free(Encoding);
		}
		
		ret = LinuxDvbPtsStart(context);
	//}

	return ret;
}

int LinuxDvbStop(Context_t  *context, char * type) {
#ifdef DEBUG  
	printf("%s::%s\n", FILENAME, __FUNCTION__);
#endif

	int ret = 0;
	int wait_time = 20;
	
	unsigned char video = !strcmp("video", type);
	unsigned char audio = !strcmp("audio", type);

#ifdef DEBUG
	printf("%s::%s v%d a%d\n", FILENAME, __FUNCTION__, video, audio);
#endif	

	while ( (isPTSThreadCreated != 0) && (--wait_time) > 0 ) {
#ifdef DEBUG  
		printf("%s::%s Waiting for LinuxDVB thread to terminate itself, will try another %d times\n", FILENAME, __FUNCTION__, wait_time);
#endif
		usleep(cSLEEPTIME);
	}

	if (wait_time == 0) {
#ifdef DEBUG  
		printf("%s::%s Timeout waiting for LinuxDVB thread!\n", FILENAME, __FUNCTION__);
#endif
		ret = -1;
	} else {
#ifdef DEBUG  
		printf("%s::%s LinuxDVB thread is terminated\n", FILENAME, __FUNCTION__);
#endif
		ret = 0;
	}

	getLinuxDVBMutex(FILENAME, __FUNCTION__,__LINE__);
	
//	if ( context && context->playback && (context->playback->isPlaying || context->playback->isPaused) ) {
		if (video && videofd != -1) {
			ioctl(videofd, VIDEO_CLEAR_BUFFER ,0);
			ioctl(videofd, VIDEO_STOP, NULL);
		}
		if (audio && audiofd != -1) {
			ioctl(audiofd, AUDIO_CLEAR_BUFFER ,0);
			ioctl(audiofd, AUDIO_STOP, NULL);
		}
/*	} else {
#ifdef DEBUG  
		printf("%s::%s not doing ioctl!\n", FILENAME, __FUNCTION__);
#endif	  
	}
*/	
	releaseLinuxDVBMutex(FILENAME, __FUNCTION__,__LINE__);
	
	return ret;
}

int LinuxDvbPause(Context_t  *context, char * type) {
	unsigned char video = !strcmp("video", type);
	unsigned char audio = !strcmp("audio", type);

#ifdef DEBUG
	printf("%s::%s v%d a%d\n", FILENAME, __FUNCTION__, video, audio);
#endif

	getLinuxDVBMutex(FILENAME, __FUNCTION__,__LINE__);
	
	//if (!context->playback->isPaused) {
		if (video && videofd != -1) {
			ioctl(videofd, VIDEO_FREEZE, NULL);
		}
		if (audio && audiofd != -1) {
			ioctl(audiofd, AUDIO_PAUSE, NULL);
		}
	//}
	releaseLinuxDVBMutex(FILENAME, __FUNCTION__,__LINE__);


	return 0;
}

int LinuxDvbContinue(Context_t  *context, char * type) {
	unsigned char video = !strcmp("video", type);
	unsigned char audio = !strcmp("audio", type);

#ifdef DEBUG
	printf("%s::%s v%d a%d\n", FILENAME, __FUNCTION__, video, audio);
#endif

	//if (context->playback->isPaused || context->playback->isForwarding || context->playback->SlowMotion) {
		if (video && videofd != -1) {
			ioctl(videofd, VIDEO_CONTINUE, NULL);
		}
		if (audio && audiofd != -1) {
			ioctl(audiofd, AUDIO_CONTINUE, NULL);
		}
	//}

#ifdef DEBUG
	printf("%s::%s exiting\n", FILENAME, __FUNCTION__);
#endif

	return 0;
}

int LinuxDvbAudioMute(Context_t  *context, char *flag) {

#ifdef DEBUG
	printf("%s::%s\n", FILENAME, __FUNCTION__);
#endif

	//if (context->playback->isPaused || context->playback->isForwarding || context->playback->SlowMotion) {
		if (audiofd != -1) {
			if(*flag == '1')
				ioctl(audiofd, AUDIO_SET_MUTE, 1);
			else
				ioctl(audiofd, AUDIO_SET_MUTE, 0);
		}
	//}

#ifdef DEBUG
	printf("%s::%s exiting\n", FILENAME, __FUNCTION__);
#endif

	return 0;
}


int LinuxDvbFlush(Context_t  *context, char * type) {
	unsigned char video = !strcmp("video", type);
	unsigned char audio = !strcmp("audio", type);

#ifdef DEBUG		  
	printf("%s::%s v%d a%d\n", FILENAME, __FUNCTION__, video, audio);
#endif	

	//if (context->playback->isPlaying || context->playback->isPaused) {
		if ( (video && videofd != -1) || (audio && audiofd != -1) ) {	// trick to create only one Mutex
			getLinuxDVBMutex(FILENAME, __FUNCTION__,__LINE__);
			
			if (video && videofd != -1) {
				ioctl(videofd, VIDEO_FLUSH ,NULL);
				ioctl(videofd, VIDEO_STOP, NULL);
			}
			
			if (audio && audiofd != -1) {
				ioctl(audiofd, AUDIO_FLUSH ,NULL);
				ioctl(audiofd, AUDIO_STOP, NULL);
			}
			
			releaseLinuxDVBMutex(FILENAME, __FUNCTION__,__LINE__);
		}
	//}

#ifdef DEBUG
	printf("%s::%s exiting\n", FILENAME, __FUNCTION__);
#endif
	return 0;
}

int LinuxDvbFastForward(Context_t  *context, char * type) {
	int ret = 0;
	
	unsigned char video = !strcmp("video", type);
	unsigned char audio = !strcmp("audio", type);

#ifdef DEBUG
	printf("%s::%s v%d a%d\n", FILENAME, __FUNCTION__, video, audio);
#endif	
	
	//if (context->playback->isPlaying && !context->playback->isPaused) {
		if ( (video && videofd != -1) || (audio && audiofd != -1) ) {	// trick to create only one Mutex
			getLinuxDVBMutex(FILENAME, __FUNCTION__,__LINE__);
			
			if (video && videofd != -1) {
				ioctl(videofd, VIDEO_FAST_FORWARD, context->playback->Speed);
			}
			if (audio && audiofd != -1) { //not supported
			//	ioctl(audiofd, AUDIO_FAST_FORWARD, context->playback->Speed);
				ret = -1;
			}
			
			releaseLinuxDVBMutex(FILENAME, __FUNCTION__,__LINE__);
		}
	//}

#ifdef DEBUG
	printf("%s::%s exiting with value %d\n", FILENAME, __FUNCTION__, ret);
#endif

	return ret;
}

int LinuxDvbSlowMotion(Context_t  *context, char * type) {
	int ret = 0;
	
	unsigned char video = !strcmp("video", type);
	unsigned char audio = !strcmp("audio", type);

#ifdef DEBUG
	printf("%s::%s v%d a%d\n", FILENAME, __FUNCTION__, video, audio);
#endif	
	
	//if (context->playback->isPlaying && !context->playback->isPaused) {
		if ( (video && videofd != -1) || (audio && audiofd != -1) ) {	// trick to create only one Mutex
			getLinuxDVBMutex(FILENAME, __FUNCTION__,__LINE__);
			
			if (video && videofd != -1) {
				ioctl(videofd, VIDEO_SLOWMOTION, context->playback->SlowMotion);
			}
			if (audio && audiofd != -1) { //not supported
			//	ioctl(audiofd, AUDIO_SLOWMOTION, context->playback->SlowMotion);
				ret = -1;
			}
			
			releaseLinuxDVBMutex(FILENAME, __FUNCTION__,__LINE__);
		}
	//}

#ifdef DEBUG
	printf("%s::%s exiting with value %d\n", FILENAME, __FUNCTION__, ret);
#endif

	return ret;
}

int LinuxDvbAVSync(Context_t  *context, char * type) {
#ifdef DEBUG
	unsigned char video = !strcmp("video", type);
#endif
	unsigned char audio = !strcmp("audio", type);

#ifdef DEBUG
	printf("%s::%s v%d a%d\n", FILENAME, __FUNCTION__, video, audio);
#endif

	if (audio && audiofd != -1) {
		getLinuxDVBMutex(FILENAME, __FUNCTION__,__LINE__);
		ioctl(audiofd, AUDIO_SET_AV_SYNC, context->playback->AVSync);
		releaseLinuxDVBMutex(FILENAME, __FUNCTION__,__LINE__);
	}

	return 0;
}

int LinuxDvbClear(Context_t  *context, char * type) {
	unsigned char video = !strcmp("video", type);
	unsigned char audio = !strcmp("audio", type);

#ifdef DEBUG
	printf("%s::%s v%d a%d\n", FILENAME, __FUNCTION__, video, audio);
#endif

	//if (context->playback->isPlaying || context->playback->isPaused) {
		if ( (video && videofd != -1) || (audio && audiofd != -1) ) {	// trick to create only one Mutex
			getLinuxDVBMutex(FILENAME, __FUNCTION__,__LINE__);
			if (video && videofd != -1) {
				ioctl(videofd, VIDEO_CLEAR_BUFFER ,0);
			}
			if (audio && audiofd != -1) {
				ioctl(audiofd, AUDIO_CLEAR_BUFFER ,0);
			}
			releaseLinuxDVBMutex(FILENAME, __FUNCTION__,__LINE__);
		}
	//}

#ifdef DEBUG
	printf("%s::%s exiting\n", FILENAME, __FUNCTION__);
#endif

	return 0;
}

static void LinuxDvbPtsThread(Context_t *context) {
	int count;
#ifdef DEBUG
	printf("%s::%s\n", FILENAME, __FUNCTION__);
#endif

	while ( context->playback->isCreationPhase ) {
#ifdef DEBUG
		printf("%s::%s Thread waiting for end of init phase...\n", FILENAME, __FUNCTION__);
#endif		    	  
	}

#ifdef DEBUG
	printf("%s::%s Running!\n", FILENAME, __FUNCTION__);
#endif		    	  

	while ( context && context->playback && context->playback->isPlaying /*videofd != -1 || audiofd != -1*/) {
	  
		getLinuxDVBMutex(FILENAME, __FUNCTION__,__LINE__);
	  
		if (videofd != -1)
			  ioctl(videofd, VIDEO_GET_PTS, (void*)&sCURRENT_PTS);
		else if (audiofd != -1)
			ioctl(audiofd, AUDIO_GET_PTS, (void*)&sCURRENT_PTS);
		else 
			sCURRENT_PTS = 0;

		releaseLinuxDVBMutex(FILENAME, __FUNCTION__,__LINE__);
		
		/* bei einer zu grossen sleeptime auf einmal, kann es vorkommen, das
		   der thread nicht erkennt, wenn ein playback zu ende ist weil er im sleep wartet.
			 Es kann dann passieren, das er durchlaeuft, weil schon der naechste playback
			 gestartet wurde, das kann dann zum absturz fuehren */
		count = 0;
		while(context && context->playback && context->playback->isPlaying && count < 10)
		{
			count++;
			usleep(100000);
		}
		//usleep(cSLEEPTIME);
	}

#ifdef DEBUG
	printf("%s::%s terminating\n",FILENAME, __FUNCTION__);
#endif

	isPTSThreadCreated = 0;
}

static int LinuxDvbPtsStart(Context_t *context) {
#ifdef DEBUG
	printf("%s::%s\n", FILENAME, __FUNCTION__);
#endif
	int error;
	int ret = 0;

#ifdef DEBUG
	if ( context && context->playback && context->playback->isCreationPhase ) {
		printf("%s::%s is Creation Phase\n", FILENAME, __FUNCTION__);
	} else {
		printf("%s::%s is NOT Creation Phase\n", FILENAME, __FUNCTION__);
	}
#endif  
	
	if (isPTSThreadCreated == 0) {
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		if ((error = pthread_create(&PtsThread, &attr, (void *)&LinuxDvbPtsThread, context)) != 0) {
#ifdef DEBUG
			printf("%s::%s Error creating thread, error:%d:%s\n", FILENAME, __FUNCTION__,error,strerror(error));
#endif
			isPTSThreadCreated = 0;
			ret = -1;
		} else {
#ifdef DEBUG
			printf("%s::%s Created thread\n", FILENAME, __FUNCTION__);
#endif
                        isPTSThreadCreated = 1;
		}
	} else {
#ifdef DEBUG
		printf("%s::%s PtsThread already exists, ignoring creation!\n", FILENAME, __FUNCTION__);
#endif
		ret = 0; // FIXME?
	}

#ifdef DEBUG
	printf("%s::%s exiting with value %d\n", FILENAME, __FUNCTION__, ret);
#endif

	return ret;
}

int LinuxDvbPts(Context_t  *context, unsigned long long int* pts) {
#ifdef DEBUG
	printf("%s::%s\n", FILENAME, __FUNCTION__);
#endif

	//if (context->playback->isPlaying) {
		*((unsigned long long int *)pts)=(unsigned long long int)sCURRENT_PTS;
	
	//} else
	//	*((unsigned long long int *)pts)=(unsigned long long int)0;

	return 0;
}

int LinuxDvbSwitch(Context_t  *context, char * type) {
	unsigned char audio = !strcmp("audio", type);
	unsigned char video = !strcmp("video", type);

#ifdef DEBUG
	printf("%s::%s v%d a%d\n", FILENAME, __FUNCTION__, video, audio);
#endif

	if ( (video && videofd != -1) || (audio && audiofd != -1) ) {	// trick to create only one Mutex
		getLinuxDVBMutex(FILENAME, __FUNCTION__,__LINE__);

		if (audio && audiofd != -1) {
			char * Encoding = NULL;
			if (context && context->manager && context->manager->audio) {
				context->manager->audio->Command(context, MANAGER_GETENCODING, &Encoding);
		
				ioctl(audiofd, AUDIO_STOP ,NULL);
				ioctl(audiofd, AUDIO_CLEAR_BUFFER ,NULL);
#ifdef DEBUG
				printf("%s::%s A %s\n", FILENAME, __FUNCTION__, Encoding);
#endif

				if(!strcmp (Encoding, "A_AC3"))
					ioctl( audiofd, AUDIO_SET_ENCODING, (void*)AUDIO_ENCODING_AC3);
				else if(!strcmp (Encoding, "A_MP3") || !strcmp (Encoding, "A_MPEG/L3"))
					ioctl( audiofd, AUDIO_SET_ENCODING, (void*)AUDIO_ENCODING_MP3);
				else if(!strcmp (Encoding, "A_DTS"))
					ioctl( audiofd, AUDIO_SET_ENCODING, (void*)AUDIO_ENCODING_DTS);
				else if(!strcmp (Encoding, "A_AAC"))
					ioctl( audiofd, AUDIO_SET_ENCODING, (void*)AUDIO_ENCODING_AAC);
				else
					ioctl( audiofd, AUDIO_SET_ENCODING, (void*)AUDIO_ENCODING_AUTO);

				ioctl(audiofd, AUDIO_PLAY, NULL); 
				free(Encoding);
			}
#ifdef DEBUG
			else
				printf("%s::%s no context for Audio\n", FILENAME, __FUNCTION__);
#endif
		}
		
		if (video && videofd != -1) {
			char * Encoding = NULL;
			if (context && context->manager && context->manager->video) {
				context->manager->video->Command(context, MANAGER_GETENCODING, &Encoding);
		
				ioctl(videofd, VIDEO_STOP ,NULL);
				ioctl(videofd, VIDEO_CLEAR_BUFFER ,NULL);
#ifdef DEBUG
				printf("%s::%s V %s\n", FILENAME, __FUNCTION__, Encoding);
#endif
				if(!strcmp (Encoding, "V_MPEG2"))
					ioctl( videofd, VIDEO_SET_ENCODING, (void*)VIDEO_ENCODING_AUTO);
				else if(!strcmp (Encoding, "V_MSCOMP") || !strcmp (Encoding, "V_MS/VFW/FOURCC") || !strcmp (Encoding, "V_MKV/XVID"))
					ioctl( videofd, VIDEO_SET_ENCODING, (void*)VIDEO_ENCODING_MPEG4P2);
				else if(!strcmp (Encoding, "V_MPEG4/ISO/AVC") || !strcmp (Encoding, "V_MPEG2/H264"))
					ioctl( videofd, VIDEO_SET_ENCODING, (void*)VIDEO_ENCODING_H264);
				else
					ioctl( videofd, VIDEO_SET_ENCODING, (void*)VIDEO_ENCODING_AUTO);
				
				ioctl(videofd, VIDEO_PLAY, NULL); 
				free(Encoding);
			} 
#ifdef DEBUG
			else
				printf("%s::%s no context for Video\n", FILENAME, __FUNCTION__);
#endif
		}
	
		releaseLinuxDVBMutex(FILENAME, __FUNCTION__,__LINE__);
	
	} 

#ifdef DEBUG
	printf("%s::%s exiting\n", FILENAME, __FUNCTION__);
#endif

	return 0;
}

//////////////////////////////////////////////////////////

#define AC3_START_CODE                  0x0b77
#define AC3_START_CODE_MASK             0xffff
#define MP3_START_CODE                  0xffe0
#define MP3_START_CODE_MASK             0xffe0
#define AAC_AUDIO_PES_START_CODE        0xcf
#define AAC_START_CODE                  0xffe0
#define AAC_HEADER_LENGTH       7

typedef struct AACHeader_s
{
    unsigned char       Data[AAC_HEADER_LENGTH];
}AACHeader_t;

#define MPEG_AUDIO_PES_START_CODE               0xc0
#define PES_MAX_HEADER_SIZE                     64
#define PRIVATE_STREAM_1_PES_START_CODE         0xbd
#define MPEG_VIDEO_PES_START_CODE               0xe0
#define H264_VIDEO_PES_START_CODE               0xe2
#define PES_START_CODE_RESERVED_4               0xfd
#define PES_VERSION_FAKE_START_CODE     0x31
#define PCM_PES_START_CODE        		0xbd
    
/*#define BIG_READS*/
#if defined (BIG_READS)
#define BLOCK_COUNT                             8
#else
#define BLOCK_COUNT                             1
#endif
#define TP_PACKET_SIZE                          188
#define BD_TP_PACKET_SIZE                       192
#define NUMBER_PACKETS                          (199*BLOCK_COUNT)
#define BUFFER_SIZE                             (TP_PACKET_SIZE*NUMBER_PACKETS)
#define PADDING_LENGTH                          (1024*BLOCK_COUNT)

#define MAX_PES_PACKET_SIZE                     65400
#define INVALID_PTS_VALUE                       0x200000000ull

typedef struct MKV_BitPacker_s
{
    unsigned char*      Ptr;                                    /* write pointer */
    unsigned int        BitBuffer;                              /* bitreader shifter */
    int                 Remaining;                              /* number of remaining in the shifter */
#ifdef DEBUG_PUTBITS
    int                 debug;
#endif /* DEBUG_PUTBITS */
} MKV_BitPacker_t;
static void MKV_PutBits(MKV_BitPacker_t * ld, unsigned int code, unsigned int length)
{
    unsigned int bit_buf;
    int bit_left;

    bit_buf = ld->BitBuffer;
    bit_left = ld->Remaining;

#ifdef DEBUG_PUTBITS
    if (ld->debug)
        dprintf("code = %d, length = %d, bit_buf = 0x%x, bit_left = %d\n", code, length, bit_buf, bit_left);
#endif /* DEBUG_PUTBITS */

    if (length < bit_left)
    {
        /* fits into current buffer */
        bit_buf = (bit_buf << length) | code;
        bit_left -= length;
    }
    else
    {
        /* doesn't fit */
        bit_buf <<= bit_left;
        bit_buf |= code >> (length - bit_left);
        ld->Ptr[0] = (char)(bit_buf >> 24);
        ld->Ptr[1] = (char)(bit_buf >> 16);
        ld->Ptr[2] = (char)(bit_buf >> 8);
        ld->Ptr[3] = (char)bit_buf;
        ld->Ptr   += 4;
        length    -= bit_left;
        bit_buf    = code & ((1 << length) - 1);
        bit_left   = 32 - length;
    }

#ifdef DEBUG_PUTBITS
    if (ld->debug)
        dprintf("bit_left = %d, bit_buf = 0x%x\n", bit_left, bit_buf);
#endif /* DEBUG_PUTBITS */

    /* writeback */
    ld->BitBuffer = bit_buf;
    ld->Remaining = bit_left;
}

static void MKV_FlushBits(MKV_BitPacker_t * ld)
{
    ld->BitBuffer <<= ld->Remaining;
    while (ld->Remaining < 32)
    {
#ifdef DEBUG_PUTBITS
        if (ld->debug)
            dprintf("flushing 0x%2.2x\n", ld->BitBuffer >> 24);
#endif /* DEBUG_PUTBITS */
        *ld->Ptr++ = ld->BitBuffer >> 24;
        ld->BitBuffer <<= 8;
        ld->Remaining += 8;
    }
    ld->Remaining = 32;
    ld->BitBuffer = 0;
}
int MKV_InsertPesHeader (unsigned char *data, int size, unsigned char stream_id, unsigned long long int pts, int pic_start_code)
{
    MKV_BitPacker_t ld2 = {data, 0, 32};

#ifdef DEBUG
    if (size>MAX_PES_PACKET_SIZE)
        dprintf("%s: Packet bigger than 63.9K eeeekkkkk\n",__FUNCTION__);
#endif

    MKV_PutBits(&ld2,0x0  ,8);
    MKV_PutBits(&ld2,0x0  ,8);
    MKV_PutBits(&ld2,0x1  ,8);  // Start Code
    MKV_PutBits(&ld2,stream_id ,8);  // Stream_id = Audio Stream
    //4
    MKV_PutBits(&ld2,size + 3 + (pts != INVALID_PTS_VALUE ? 5:0) + (pic_start_code ? (5) : 0),16); // PES_packet_length
    //6 = 4+2
    MKV_PutBits(&ld2,0x2  ,2);  // 10
    MKV_PutBits(&ld2,0x0  ,2);  // PES_Scrambling_control
    MKV_PutBits(&ld2,0x0  ,1);  // PES_Priority
    MKV_PutBits(&ld2,0x0  ,1);  // data_alignment_indicator
    MKV_PutBits(&ld2,0x0  ,1);  // Copyright
    MKV_PutBits(&ld2,0x0  ,1);  // Original or Copy
    //7 = 6+1

    if (pts!=INVALID_PTS_VALUE)
        MKV_PutBits(&ld2,0x2 ,2);
    else
        MKV_PutBits(&ld2,0x0 ,2);  // PTS_DTS flag

    MKV_PutBits(&ld2,0x0 ,1);  // ESCR_flag
    MKV_PutBits(&ld2,0x0 ,1);  // ES_rate_flag
    MKV_PutBits(&ld2,0x0 ,1);  // DSM_trick_mode_flag
    MKV_PutBits(&ld2,0x0 ,1);  // additional_copy_ingo_flag
    MKV_PutBits(&ld2,0x0 ,1);  // PES_CRC_flag
    MKV_PutBits(&ld2,0x0 ,1);  // PES_extension_flag
    //8 = 7+1

    if (pts!=INVALID_PTS_VALUE)
        MKV_PutBits(&ld2,0x5,8);
    else
        MKV_PutBits(&ld2,0x0 ,8);  // PES_header_data_length
    //9 = 8+1

    if (pts!=INVALID_PTS_VALUE)
    {
        MKV_PutBits(&ld2,0x2,4);
        MKV_PutBits(&ld2,(pts>>30) & 0x7,3);
        MKV_PutBits(&ld2,0x1,1);
        MKV_PutBits(&ld2,(pts>>15) & 0x7fff,15);
        MKV_PutBits(&ld2,0x1,1);
        MKV_PutBits(&ld2,pts & 0x7fff,15);
        MKV_PutBits(&ld2,0x1,1);
    }
    //14 = 9+5

    if (pic_start_code)
    {
        MKV_PutBits(&ld2,0x0 ,8);
        MKV_PutBits(&ld2,0x0 ,8);
        MKV_PutBits(&ld2,0x1 ,8);  // Start Code
        MKV_PutBits(&ld2,pic_start_code & 0xff ,8);  // 00, for picture start
        MKV_PutBits(&ld2,(pic_start_code >> 8 )&0xff,8);  // For any extra information (like in mpeg4p2, the pic_start_code)
        //14 + 4 = 18
    }

    MKV_FlushBits(&ld2);

    return (ld2.Ptr - data);

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dts                                                                       //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define PES_AUDIO_PRIVATE_HEADER_SIZE   16                                // consider maximum private header size.
#define PES_AUDIO_HEADER_SIZE           (32 + PES_AUDIO_PRIVATE_HEADER_SIZE)
#define PES_AUDIO_PACKET_SIZE           2028
#define SPDIF_AUDIO_PACKET_SIZE         (1024 * sizeof(unsigned int) * 2) // stereo 32bit samples.
static int dts(const unsigned char *PLAYERData, int DataLength, unsigned long long int Pts)
{
#ifdef DEBUG
		printf("%s::%s\n", FILENAME, __FUNCTION__);
#endif
    int i = 0;
    unsigned char               PesHeader[PES_AUDIO_HEADER_SIZE];
    memset (PesHeader, '0', PES_AUDIO_HEADER_SIZE);

    unsigned char * Data = 0;
    Data = (unsigned char *) malloc(DataLength);
    memcpy(Data, PLAYERData, DataLength);

    /* 16-bit byte swap all data before injecting it */
    for (i=0; i<DataLength; i+=2) 
    {
        unsigned char Tmp = Data[i];
        Data[i] = Data[i+1];
        Data[i+1] = Tmp;
    }
                        
    int HeaderLength    = MKV_InsertPesHeader (PesHeader, DataLength, MPEG_AUDIO_PES_START_CODE/*PRIVATE_STREAM_1_PES_START_CODE*/, Pts, 0);
    unsigned char* PacketStart = malloc(DataLength + HeaderLength);
    memcpy (PacketStart, PesHeader, HeaderLength);
    memcpy (PacketStart + HeaderLength, PLAYERData, DataLength);

    int len = write(audiofd,PacketStart,DataLength+HeaderLength);

    free(PacketStart);
    free(Data);

    return len;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// mp3                                                                       //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static int mp3 (const unsigned char *PLAYERData, int DataLength, unsigned long long int Pts)
{
#ifdef DEBUG
		printf("%s::%s %llu\n", FILENAME, __FUNCTION__, Pts);
#endif

    unsigned char  PesHeader[PES_MAX_HEADER_SIZE];

    int HeaderLength = MKV_InsertPesHeader (PesHeader, DataLength,MPEG_AUDIO_PES_START_CODE, Pts, 0);

        unsigned char* PacketStart = malloc(DataLength + HeaderLength);
    memcpy (PacketStart, PesHeader, HeaderLength);
    memcpy (PacketStart + HeaderLength, PLAYERData, DataLength);

    int len = write(audiofd,PacketStart,DataLength+HeaderLength);

    if (len < 0)
    {
       printf("%s: %s\n", __func__, strerror(errno));
    }

    free(PacketStart);

#ifdef DEBUG
    printf("mp3_Write-< len=%d, audiofd %d\n", len, audiofd);
#endif
    return len;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wma                                                                       //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int wmaInitialHeaderRequired = 1;

static int wma (const unsigned char *PLAYERData, int DataLength, unsigned long long int Pts, const void *private_data, unsigned int private_size)
{
#ifdef DEBUG
		printf("%s::%s %llu\n", FILENAME, __FUNCTION__, Pts);
#endif

    int len = 0;

    if (wmaInitialHeaderRequired) {

        unsigned char  PesHeader[PES_MAX_HEADER_SIZE];
        int HeaderLength = MKV_InsertPesHeader (PesHeader, private_size,MPEG_AUDIO_PES_START_CODE, 0, 0);

        unsigned char* PacketStart = malloc(private_size + HeaderLength);
        memcpy (PacketStart, PesHeader, HeaderLength);
        memcpy (PacketStart + HeaderLength, private_data, private_size);

        len = write(audiofd,PacketStart,private_size+HeaderLength);

        free(PacketStart);

        wmaInitialHeaderRequired = 0;
    }

    if(DataLength > 0 && PLAYERData)
    {
        unsigned char  PesHeader[PES_MAX_HEADER_SIZE];

        int HeaderLength = MKV_InsertPesHeader (PesHeader, DataLength,MPEG_AUDIO_PES_START_CODE, Pts, 0);

        unsigned char* PacketStart = malloc(DataLength + HeaderLength);
        memcpy (PacketStart, PesHeader, HeaderLength);
        memcpy (PacketStart + HeaderLength, PLAYERData, DataLength);

        len = write(audiofd,PacketStart,DataLength+HeaderLength);

        free(PacketStart);
    }

#ifdef DEBUG
    printf("wma-< len=%d\n", len);
#endif
    return len;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ac3                                                                       //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static int ac3 (const unsigned char *PLAYERData, const int DataLength, unsigned long long int Pts)
{
#ifdef DEBUG
		printf("%s::%s\n", FILENAME, __FUNCTION__);
#endif

    unsigned char  PesHeader[PES_MAX_HEADER_SIZE];

    int HeaderLength = MKV_InsertPesHeader (PesHeader, DataLength, PRIVATE_STREAM_1_PES_START_CODE, Pts, 0);

    unsigned char* PacketStart = malloc(DataLength + HeaderLength);
    memcpy (PacketStart, PesHeader, HeaderLength);
    memcpy (PacketStart + HeaderLength, PLAYERData, DataLength);

    int len = write(audiofd,PacketStart,DataLength+HeaderLength);

    free(PacketStart);

    return len;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// aac_mpeg4                                                                       //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static unsigned char DefaultAACHeader[]    =  { 0xff, 0xf1, /*0x00, 0x00*/0x50, 0x80, 0x00, 0x1f, 0xfc };//Trick: ob es bei allen geht??

int aac_mpeg4 (const unsigned char *PLAYERData, const int DataLength, unsigned long long int Pts, const void *private_data, unsigned int private_size)
{
#ifdef DEBUG
		printf("%s::%s\n", FILENAME, __FUNCTION__);
#endif
    if(private_data==NULL)
    {
#ifdef DEBUG
      printf("private_data = NULL\n");
#endif
      private_data=DefaultAACHeader;
      private_size=7;
    }
    unsigned char PesHeader[PES_MAX_HEADER_SIZE];
    unsigned char ExtraData[AAC_HEADER_LENGTH];
    unsigned int  PacketLength    = DataLength + AAC_HEADER_LENGTH;

    memcpy (ExtraData, (AACHeader_t*)private_data, AAC_HEADER_LENGTH);
    ExtraData[3]       |= (PacketLength >> 12) & 0x3;
    ExtraData[4]        = (PacketLength >> 3) & 0xff;
    ExtraData[5]       |= (PacketLength << 5) & 0xe0;

    unsigned int  HeaderLength = MKV_InsertPesHeader (PesHeader, PacketLength, AAC_AUDIO_PES_START_CODE, Pts, 0);

    unsigned char* PacketStart = malloc(HeaderLength + sizeof(ExtraData) + DataLength);
    memcpy (PacketStart, PesHeader, HeaderLength);
    memcpy (PacketStart + HeaderLength, ExtraData, sizeof(ExtraData));
    memcpy (PacketStart + HeaderLength + sizeof(ExtraData), PLAYERData, DataLength);

    int len = write(audiofd, PacketStart, HeaderLength + DataLength + sizeof(ExtraData));
    
    free(PacketStart);
    
    return len;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// mpeg2                                                                       //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static int mpeg2(const unsigned char *PLAYERData, int DataLength, long long int Pts)
{
#ifdef DEBUG
		printf("%s::%s\n", FILENAME, __FUNCTION__);
#endif
    unsigned char               PesHeader[PES_MAX_HEADER_SIZE];
    int len = 0;

    
    int Position = 0;

    while(1) {
        int PacketLength = (DataLength-Position) <= MAX_PES_PACKET_SIZE ? (DataLength-Position) : MAX_PES_PACKET_SIZE;

#ifdef DEBUG
        int Remaining = DataLength - Position - PacketLength;
        printf("PacketLength=%d, Remaining=%d, Position=%d\n", PacketLength, Remaining, Position);
#endif
        int HeaderLength = MKV_InsertPesHeader (PesHeader, PacketLength, 0xe0, Pts, 0);
            unsigned char* PacketStart = malloc(PacketLength + HeaderLength);
        memcpy (PacketStart, PesHeader, HeaderLength);
        memcpy (PacketStart + HeaderLength, PLAYERData + Position, PacketLength);

        len = write(videofd,PacketStart,PacketLength+HeaderLength);

#ifdef DEBUG
        printf("%s len=%d\n", __func__, len);
        if (len < 0)
	{
	   printf("%s: %s\n", __func__, strerror(errno));
	}
#endif

        free(PacketStart);

        Position += PacketLength;
        Pts = INVALID_PTS_VALUE;
        if (Position == DataLength)
            break;
    }


    return len;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// xvid                                                                        //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static int divx(const unsigned char *PLAYERData, int DataLength, long long int Pts, unsigned int mpf)
{
#ifdef DEBUG
    printf("%s::%s\n", FILENAME, __FUNCTION__);
    printf("\tmpf=%d\n", mpf);
#endif
    unsigned char  PesHeader[PES_MAX_HEADER_SIZE];

    //XVID IS LIKE DIVX5    

	unsigned char  FakeHeaders[64]; // 64bytes should be enough to make the fake headers
	unsigned int   FakeHeaderLength;
	unsigned char  Version             = 5;
	unsigned int   FakeStartCode       = (Version << 8) | PES_VERSION_FAKE_START_CODE;
	unsigned int   HeaderLength = 0;

	MKV_BitPacker_t ld = {FakeHeaders, 0, 32};
	memset(FakeHeaders, 0, sizeof(FakeHeaders));

    //{{{  Create info record for frame parser
        {
        // divx4 & 5
        // VOS
        //PutBits(&ld, 0x0, 8);
        //PutBits(&ld, 0x0, 8);
        MKV_PutBits(&ld, 0x1b0, 32);      // startcode 
        MKV_PutBits(&ld, 0, 8);           // profile = reserved 
        MKV_PutBits(&ld, 0x1b2, 32);      // startcode (user data)
        MKV_PutBits(&ld, 0x53545443, 32); // STTC - an embedded ST timecode from an avi file
        MKV_PutBits(&ld, mpf , 32);
                                                      // microseconds per frame
        MKV_FlushBits(&ld);
    }
    //}}}

    FakeHeaderLength    = (ld.Ptr - (FakeHeaders));

    HeaderLength        = MKV_InsertPesHeader (PesHeader, DataLength, MPEG_VIDEO_PES_START_CODE, Pts, FakeStartCode);
    unsigned char* PacketStart = malloc(DataLength + HeaderLength + FakeHeaderLength);
    memcpy (PacketStart, PesHeader, HeaderLength);
    memcpy (PacketStart + HeaderLength, FakeHeaders, FakeHeaderLength);
    memcpy (PacketStart + HeaderLength + FakeHeaderLength, PLAYERData, DataLength);

    int len = write(videofd,PacketStart,DataLength+HeaderLength+FakeHeaderLength);

    free(PacketStart);

#ifdef DEBUG
    printf("xvid_Write-< len=%d\n", len);
#endif
    return len;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// h264                                                                        //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static int h264InitialHeaderRequired = 1;
static unsigned int        NalLengthBytes          = 1;
typedef struct avcC_s
{
    unsigned char       Version;                /* configurationVersion        */
    unsigned char       Profile;                /* AVCProfileIndication        */
    unsigned char       Compatibility;          /* profile_compatibility       */
    unsigned char       Level;                  /* AVCLevelIndication          */
    unsigned char       NalLengthMinusOne;      /* held in bottom two bits     */
    unsigned char       NumParamSets;           /* held in bottom 5 bits       */
    unsigned char       Params[1];              /* {length,params}{length,params}...sequence then picture*/
}avcC_t;

const unsigned char Head[]                  = {0, 0, 0, 1};


static int h264(const unsigned char *PLAYERData, int DataLength, unsigned long long int Pts, const void *private_data, unsigned int private_size, unsigned int TimeDelta, unsigned int TimeScale)
{
#ifdef DEBUG
		printf("%s::%s %d %d\n", FILENAME, __FUNCTION__,TimeDelta,TimeScale);
#endif

    //unsigned int            TimeDelta = 23976;
    //unsigned int            TimeScale = 1000;

    unsigned char*          PacketStart = NULL;
    unsigned int            PacketStartSIZE = 0;
    unsigned int            HeaderLength;
    unsigned char           PesHeader[PES_MAX_HEADER_SIZE];
    unsigned long long int  VideoPts = Pts;

    int len = 0;

    if (h264InitialHeaderRequired) {
        unsigned char*  HeaderData = malloc(BUFFER_SIZE+PADDING_LENGTH);
        avcC_t*         avcCHeader          = (avcC_t*)private_data;
        int             i;
        unsigned int    ParamSets;
        unsigned int    ParamOffset;
        unsigned int    InitialHeaderLength     = 0;
        unsigned int    ParametersLength;

#ifdef DEBUG
        if (avcCHeader->Version != 1)
            printf ("%s: Error unknown avcC version (%x). Expect problems.\n",__FUNCTION__, avcCHeader->Version);
#endif
	
#define NALU_TYPE_PLAYER2_CONTAINER_PARAMETERS          24
#define CONTAINER_PARAMETERS_VERSION                    0x00

        ParametersLength                      = 0;
        
        HeaderData[ParametersLength++]        = 0x00;                                         // Start code
        HeaderData[ParametersLength++]        = 0x00;
        HeaderData[ParametersLength++]        = 0x01;
        HeaderData[ParametersLength++]        = NALU_TYPE_PLAYER2_CONTAINER_PARAMETERS;
        // Container message version - changes when/if we vary the format of the message
        HeaderData[ParametersLength++]        = CONTAINER_PARAMETERS_VERSION; 
        HeaderData[ParametersLength++]        = 0xff;                                         // Field separator 

        if( TimeDelta == 0xffffffff )
            TimeDelta       = (TimeScale > 1000) ? 1001 : 1;

        HeaderData[ParametersLength++]        = (TimeScale >> 24) & 0xff;         // Output the timescale
        HeaderData[ParametersLength++]        = (TimeScale >> 16) & 0xff;
        HeaderData[ParametersLength++]        = 0xff;
        HeaderData[ParametersLength++]        = (TimeScale >> 8) & 0xff;
        HeaderData[ParametersLength++]        = TimeScale & 0xff;
        HeaderData[ParametersLength++]        = 0xff;

        HeaderData[ParametersLength++]        = (TimeDelta >> 24) & 0xff;         // Output frame period
        HeaderData[ParametersLength++]        = (TimeDelta >> 16) & 0xff;
        HeaderData[ParametersLength++]        = 0xff;
        HeaderData[ParametersLength++]        = (TimeDelta >> 8) & 0xff;
        HeaderData[ParametersLength++]        = TimeDelta & 0xff;
        HeaderData[ParametersLength++]        = 0xff;
        HeaderData[ParametersLength++]        = 0x80;                                         // Rsbp trailing bits

        HeaderLength = MKV_InsertPesHeader (PesHeader, ParametersLength, MPEG_VIDEO_PES_START_CODE, INVALID_PTS_VALUE, 0);

        PacketStart = malloc(HeaderLength + ParametersLength);
        PacketStartSIZE = HeaderLength + ParametersLength;
        memcpy (PacketStart, PesHeader, HeaderLength);
        memcpy (PacketStart + HeaderLength, HeaderData, ParametersLength);
        len += write (videofd, PacketStart, HeaderLength + ParametersLength);

        NalLengthBytes  = (avcCHeader->NalLengthMinusOne & 0x03) + 1;
        ParamSets       = avcCHeader->NumParamSets & 0x1f;
#ifdef DEBUG
        printf( "%s: avcC contents:\n", __FUNCTION__);
        printf( "    version:                       %d\n", avcCHeader->Version);
        printf( "    profile:                       %d\n", avcCHeader->Profile);
        printf( "    profile compatibility:         %d\n", avcCHeader->Compatibility);
        printf( "    level:                         %d\n", avcCHeader->Level);
        printf( "    nal length bytes:              %d\n", NalLengthBytes);
        printf( "    number of sequence param sets: %d\n", ParamSets);
#endif

        ParamOffset     = 0;
        for (i = 0; i < ParamSets; i++) {
            unsigned int  PsLength = (avcCHeader->Params[ParamOffset] << 8) + avcCHeader->Params[ParamOffset+1];
#ifdef DEBUG
            dprintf( "        sps %d has length           %d\n", i, PsLength);
#endif

            if (HeaderLength + InitialHeaderLength + sizeof(Head) > PacketStartSIZE) {
                PacketStart = realloc(PacketStart, HeaderLength + InitialHeaderLength + sizeof(Head));
                PacketStartSIZE = HeaderLength + InitialHeaderLength + sizeof(Head);
            }

            memcpy (PacketStart + HeaderLength + InitialHeaderLength, Head, sizeof(Head));
            InitialHeaderLength        += sizeof(Head);

            if (HeaderLength + InitialHeaderLength + PsLength > PacketStartSIZE) {
                PacketStart = realloc(PacketStart, HeaderLength + InitialHeaderLength + PsLength);
                PacketStartSIZE = HeaderLength + InitialHeaderLength + PsLength;
            }

            memcpy (PacketStart + HeaderLength + InitialHeaderLength, &avcCHeader->Params[ParamOffset+2], PsLength);

            InitialHeaderLength        += PsLength;
            ParamOffset                += PsLength+2;
        }

        ParamSets                       = avcCHeader->Params[ParamOffset];
#ifdef DEBUG
        dprintf( "    number of picture param sets:  %d\n", ParamSets);
#endif
	ParamOffset++;
        for (i = 0; i < ParamSets; i++) {
            unsigned int  PsLength      = (avcCHeader->Params[ParamOffset] << 8) + avcCHeader->Params[ParamOffset+1];
#ifdef DEBUG
            dprintf ("        pps %d has length           %d\n", i, PsLength);
#endif
            if (HeaderLength + InitialHeaderLength + sizeof(Head) > PacketStartSIZE) {
                PacketStart = realloc(PacketStart, HeaderLength + InitialHeaderLength + sizeof(Head));
                PacketStartSIZE = HeaderLength + InitialHeaderLength + sizeof(Head);
            }

            memcpy (PacketStart + HeaderLength + InitialHeaderLength, Head, sizeof(Head));
            InitialHeaderLength        += sizeof(Head);

            if (HeaderLength + InitialHeaderLength + PsLength > PacketStartSIZE) {
                PacketStart = realloc(PacketStart, HeaderLength + InitialHeaderLength + PsLength);
                PacketStartSIZE = HeaderLength + InitialHeaderLength + PsLength;
            }

            memcpy (PacketStart + HeaderLength + InitialHeaderLength, &avcCHeader->Params[ParamOffset+2], PsLength);
            InitialHeaderLength        += PsLength;
            ParamOffset                += PsLength+2;
        }

        HeaderLength    = MKV_InsertPesHeader (PesHeader, InitialHeaderLength, MPEG_VIDEO_PES_START_CODE, INVALID_PTS_VALUE, 0);
        memcpy (PacketStart, PesHeader, HeaderLength);
                
        len += write (videofd, PacketStart, HeaderLength + InitialHeaderLength);

        h264InitialHeaderRequired           = 0;

        free(PacketStart);
        free(HeaderData);
    }

    unsigned int SampleSize    = DataLength;
    unsigned int NalStart      = 0;
    unsigned int VideoPosition = 0;
                        
    do {
        unsigned int   NalLength;
        unsigned char  NalData[4];
        int           NalPresent       = 1;

        memcpy (NalData, PLAYERData+VideoPosition, NalLengthBytes);
        VideoPosition+= NalLengthBytes;
        NalLength       = (NalLengthBytes == 1) ?  NalData[0] :
        (NalLengthBytes == 2) ? (NalData[0] <<  8) |  NalData[1] :
        (NalLengthBytes == 3) ? (NalData[0] << 16) | (NalData[1] <<  8) |  NalData[2] :
                                (NalData[0] << 24) | (NalData[1] << 16) | (NalData[2] << 8) | NalData[3];

#ifdef DEBUG
        printf("NalStart = %u + NalLength = %u > SampleSize = %u\n", NalStart, NalLength, SampleSize);
#endif

        if (NalStart + NalLength > SampleSize) {
#ifdef DEBUG
            printf("%s: nal length past end of buffer - size %u frame offset %u left %u\n",__FUNCTION__,
                NalLength, NalStart , SampleSize - NalStart );
#endif
	    NalStart    = SampleSize;
        } else {
            NalStart               += NalLength + NalLengthBytes;
            while (NalLength > 0) {
                unsigned int   PacketLength     = (NalLength < BUFFER_SIZE) ? NalLength : BUFFER_SIZE;
                int            ExtraLength      = 0;
                unsigned char* PacketStart;

                NalLength      -= PacketLength;
                    
                if (NalPresent) {
                    PacketStart     = malloc(sizeof(Head) + PacketLength);
                    memcpy (PacketStart+sizeof(Head), PLAYERData+VideoPosition, PacketLength);
                    VideoPosition    += PacketLength;

                    memcpy (PacketStart, Head, sizeof(Head));
                    ExtraLength    = sizeof(Head);
                } else {
                    PacketStart     = malloc(PacketLength);
                    memcpy (PacketStart, PLAYERData+VideoPosition, PacketLength);
                    VideoPosition    += PacketLength;
                }

                PacketLength   += ExtraLength;

#ifdef DEBUG
								printf ("%s:  pts=%llu\n", __FUNCTION__, VideoPts);
#endif

                HeaderLength    = MKV_InsertPesHeader (PesHeader, PacketLength, MPEG_VIDEO_PES_START_CODE, VideoPts, 0);
                                        

                unsigned char*    WritePacketStart = malloc(HeaderLength + PacketLength);
                memcpy (WritePacketStart,              PesHeader,   HeaderLength);
                memcpy (WritePacketStart+HeaderLength, PacketStart, PacketLength);
                free(PacketStart);

                PacketLength   += HeaderLength;
                len += write (videofd, WritePacketStart, PacketLength);
                free(WritePacketStart);

                NalPresent      = 0;
                VideoPts        = INVALID_PTS_VALUE;
            }
        }
    } while (NalStart < SampleSize);

    return len;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wmv                                                                        //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define PES_PRIVATE_DATA_FLAG                           0x80
#define PES_PRIVATE_DATA_LENGTH                         8

#define PES_LENGTH_BYTE_0                               5
#define PES_LENGTH_BYTE_1                               4
#define PES_FLAGS_BYTE                                  7
#define PES_EXTENSION_DATA_PRESENT                      0x01
#define PES_HEADER_DATA_LENGTH_BYTE                     8

int MKV_InsertVideoPrivateDataHeader(unsigned char *data, int payload_size)
{
    MKV_BitPacker_t ld2 = {data, 0, 32};
    int         i;

    MKV_PutBits (&ld2, PES_PRIVATE_DATA_FLAG, 8);
    MKV_PutBits (&ld2, payload_size & 0xff, 8);
    MKV_PutBits (&ld2, (payload_size >> 8) & 0xff, 8);
    MKV_PutBits (&ld2, (payload_size >> 16) & 0xff, 8);

    for (i = 4; i < (PES_PRIVATE_DATA_LENGTH+1); i++)
        MKV_PutBits (&ld2, 0, 8);

    MKV_FlushBits (&ld2);

    return PES_PRIVATE_DATA_LENGTH+1;

}

#define PES_MIN_HEADER_SIZE                     9
#define WMV3_PRIVATE_DATA_LENGTH				4
#define VC1_VIDEO_PES_START_CODE				0xfd
static int wmvInitialHeaderRequired = 1;
typedef struct
{
	unsigned char      privateData[WMV3_PRIVATE_DATA_LENGTH]; 
	unsigned int       width;
	unsigned int       height;
	unsigned int       framerate;
}awmv_t;

    #define METADATA_STRUCT_A_START             12
    #define METADATA_STRUCT_B_START             24
    #define METADATA_STRUCT_B_FRAMERATE_START   32
    #define METADATA_STRUCT_C_START             8
static const unsigned char  Metadata[]          = {0x00,    0x00,   0x00,   0xc5,
                                                           0x04,    0x00,   0x00,   0x00,
                                                           0xc0,    0x00,   0x00,   0x00,   /* Struct C set for for advanced profile*/
                                                           0x00,    0x00,   0x00,   0x00,   /* Struct A */
                                                           0x00,    0x00,   0x00,   0x00,
                                                           0x0c,    0x00,   0x00,   0x00,
                                                           0x60,    0x00,   0x00,   0x00,   /* Struct B */
                                                           0x00,    0x00,   0x00,   0x00,
                                                           0x00,    0x00,   0x00,   0x00};


static int wmv(const unsigned char *PLAYERData, int DataLength, unsigned long long int Pts, const awmv_t *private_data, unsigned int private_size)
{
	int len = 0;

#ifdef DEBUG
	printf("%s::%s\n", FILENAME, __FUNCTION__);
#endif

	if(wmvInitialHeaderRequired)
	{
		unsigned char               PesPacket[PES_MIN_HEADER_SIZE+128];
		unsigned char*              PesPtr;
		unsigned int                MetadataLength;

#ifdef DEBUG
		printf("Framerate: %u\n", private_data->framerate);
                printf("biWidth: %d\n", private_data->width);
                printf("biHeight: %d\n", private_data->height);
#endif
		PesPtr          = &PesPacket[PES_MIN_HEADER_SIZE];

		memcpy (PesPtr, Metadata, sizeof(Metadata));
		PesPtr         += METADATA_STRUCT_C_START;

		memcpy (PesPtr, private_data->privateData, WMV3_PRIVATE_DATA_LENGTH);
		PesPtr             += WMV3_PRIVATE_DATA_LENGTH;

		/* Metadata Header Struct A */
		*PesPtr++           = (private_data->height >>  0) & 0xff;
		*PesPtr++           = (private_data->height >>  8) & 0xff;
		*PesPtr++           = (private_data->height >> 16) & 0xff;
		*PesPtr++           =  private_data->height >> 24;
		*PesPtr++           = (private_data->width  >>  0) & 0xff;
		*PesPtr++           = (private_data->width  >>  8) & 0xff;
		*PesPtr++           = (private_data->width  >> 16) & 0xff;
		*PesPtr++           =  private_data->width  >> 24;

		PesPtr             += 12;       /* Skip flag word and Struct B first 8 bytes */

		*PesPtr++           = (private_data->framerate >>  0) & 0xff;
		*PesPtr++           = (private_data->framerate >>  8) & 0xff;
		*PesPtr++           = (private_data->framerate >> 16) & 0xff;
		*PesPtr++           =  private_data->framerate >> 24;

		MetadataLength      = PesPtr - &PesPacket[PES_MIN_HEADER_SIZE];

		int HeaderLength        = MKV_InsertPesHeader (PesPacket, MetadataLength, VC1_VIDEO_PES_START_CODE, INVALID_PTS_VALUE, 0);

					/*int i , j;
                    printf ("%s: (%d), Data = ", __FUNCTION__, HeaderLength+MetadataLength);
                    for(j=0; j< HeaderLength+MetadataLength;) {
                    	for (i=0; i<16 && j < HeaderLength+MetadataLength; i++, j++)
                        	printf ("%02x ", PesPacket[j]);
                    	printf ("\n");
                    }*/

		len = write(videofd,PesPacket, HeaderLength+MetadataLength);
		wmvInitialHeaderRequired = 0;
	}

//o 00 00 01 fd 27 c9 80 81 | 0e . 21 00 1b bb a1 80 b8 27 00 00 00 00 00 00
//d 00 00 01 fd 27 c9 80 81 | 0e . 31 00 1b d2 d5 80 b8 27 00 00 00 00 00 00

	if(DataLength > 0 && PLAYERData)
    {
        unsigned char  PesHeader[PES_MAX_HEADER_SIZE];
        int HeaderLength = MKV_InsertPesHeader (PesHeader, DataLength, VC1_VIDEO_PES_START_CODE, Pts, 0);

		{
			unsigned int        FrameSize       = DataLength/*BufferDataLength*/;
			unsigned int        PesLength;
			unsigned int        PrivateHeaderLength;

			PrivateHeaderLength     = MKV_InsertVideoPrivateDataHeader (&PesHeader[HeaderLength],
						                            FrameSize);
			/* Update PesLength */
			PesLength               = PesHeader[PES_LENGTH_BYTE_0] + (PesHeader[PES_LENGTH_BYTE_1] << 8) + PrivateHeaderLength;
			PesHeader[PES_LENGTH_BYTE_0]            = PesLength & 0xff;
			PesHeader[PES_LENGTH_BYTE_1]            = (PesLength >> 8) & 0xff;
			PesHeader[PES_HEADER_DATA_LENGTH_BYTE] += PrivateHeaderLength;
			PesHeader[PES_FLAGS_BYTE]              |= PES_EXTENSION_DATA_PRESENT;

			HeaderLength                           += PrivateHeaderLength;
		}

        unsigned char* PacketStart = malloc(DataLength + HeaderLength);
        memcpy (PacketStart, PesHeader, HeaderLength);
        memcpy (PacketStart + HeaderLength, PLAYERData, DataLength);

		/*int j;
        printf ("%s: (%6d | %04x) ", __FUNCTION__, DataLength+HeaderLength, DataLength+HeaderLength);
        for(j = 0; j < HeaderLength; j++) {
           	printf ("%02x ", PacketStart[j]);
        }
		printf ("  |  ");
		for(j = 0; j < 4; j++) {
           	printf ("%02x ", PacketStart[HeaderLength+j]);
        }
		printf ("\n");*/

        len = write(videofd,PacketStart,DataLength+HeaderLength);

        free(PacketStart);
    }

    //dprintf("xvid_Write-< len=%d\n", len);
    return len;
}





static int Write(void  *_context, unsigned char *PLAYERData, int DataLength, unsigned long long int Pts, unsigned char *Private, const int PrivateLength, float FrameRate, char * type) {
   Context_t  *context = (Context_t*) _context;

	int ret = 0;
	int result;
	
#ifdef DEBUG
	printf("%s::%s DataLength=%u PrivateLength=%u Pts=%llu FrameRate=%f\n", FILENAME, __FUNCTION__, DataLength, PrivateLength, Pts, FrameRate);
#endif

	unsigned char video = !strcmp("video", type);
	unsigned char audio = !strcmp("audio", type);
	unsigned int TimeDelta;
	unsigned int TimeScale;
#ifdef DEBUG
	printf("%s::%s v%d a%d\n", FILENAME, __FUNCTION__, video, audio);
#endif

	if (video) {
			char * Encoding = NULL;
			context->manager->video->Command(context, MANAGER_GETENCODING, &Encoding);

#ifdef DEBUG
			printf("%s::%s Encoding = %s\n", FILENAME, __FUNCTION__, Encoding);
#endif

			if(!strcmp (Encoding, "V_MPEG2") || !strcmp (Encoding, "V_MPEG2/H264"))
				result = mpeg2(PLAYERData, DataLength, Pts);
			else if(!strcmp (Encoding, "V_MPEG4/ISO/AVC")){
				TimeDelta = FrameRate*1000.0;
				TimeScale = 1000;
				result = h264(PLAYERData, DataLength, Pts, Private, PrivateLength,TimeDelta,TimeScale);
			}
 			else if(!strcmp (Encoding, "V_MSCOMP") || !strcmp (Encoding, "V_MS/VFW/FOURCC") || !strcmp (Encoding, "V_MKV/XVID"))
				result = divx(PLAYERData, DataLength, Pts, FrameRate);
			else if(!strcmp (Encoding, "V_WMV"))
				result = wmv(PLAYERData, DataLength, Pts, (const awmv_t *)Private, PrivateLength);
			else {
#ifdef DEBUG
			  printf("unknown video codec %s\n",Encoding);
#endif
			}
			free(Encoding);
	} else if (audio) {
			char * Encoding = NULL;
			context->manager->audio->Command(context, MANAGER_GETENCODING, &Encoding);

#ifdef DEBUG
			printf("%s::%s Encoding = %s\n", FILENAME, __FUNCTION__, Encoding);
#endif

			if(!strcmp (Encoding, "A_AC3"))
				result = ac3(PLAYERData, DataLength, Pts);
			else if(!strcmp (Encoding, "A_MP3") || !strcmp (Encoding, "A_MPEG/L3") || !strcmp (Encoding, "A_MS/ACM"))
				result = mp3(PLAYERData, DataLength, Pts);
			else if(!strcmp (Encoding, "A_AAC"))
				result = aac_mpeg4 (PLAYERData, DataLength, Pts, Private, PrivateLength);
 			else if(!strcmp (Encoding, "A_DTS"))
				result = dts(PLAYERData, DataLength, Pts);
 			else if(!strcmp (Encoding, "A_WMA"))
				result = wma(PLAYERData, DataLength, Pts, (const unsigned char *) Private, PrivateLength);
			else {
#ifdef DEBUG
			  printf("unknown audio codec %s\n",Encoding);
#endif
			}
			free(Encoding);
	}

	return ret;
}

static int Command(void  *_context, OutputCmd_t command, void * argument) {
   Context_t  *context = (Context_t*) _context;
#ifdef DEBUG
	printf("%s::%s Command %d\n", FILENAME, __FUNCTION__, command);
#endif
	
	int ret = 0;

	switch(command) {
		case OUTPUT_OPEN: {
			ret = LinuxDvbOpen(context, (char*)argument);
			break;
		}
		case OUTPUT_CLOSE: {
			LinuxDvbClose(context, (char*)argument);
			h264InitialHeaderRequired = 1;
			wmaInitialHeaderRequired = 1;
			wmvInitialHeaderRequired = 1;
			sCURRENT_PTS = 0;
			break;
		}
		case OUTPUT_PLAY: {	// 4
			h264InitialHeaderRequired = 1;
			wmaInitialHeaderRequired = 1;
			wmvInitialHeaderRequired = 1;
			sCURRENT_PTS = 0;
			LinuxDvbPlay(context, (char*)argument);
			break;
		}
		case OUTPUT_STOP: {
			LinuxDvbStop(context, (char*)argument);
			h264InitialHeaderRequired = 1;
			wmaInitialHeaderRequired = 1;
			wmvInitialHeaderRequired = 1;
			sCURRENT_PTS = 0;
			break;
		}
		case OUTPUT_FLUSH: {
			LinuxDvbFlush(context, (char*)argument);
			h264InitialHeaderRequired = 1;
			wmaInitialHeaderRequired = 1;
			wmvInitialHeaderRequired = 1;
			sCURRENT_PTS = 0;
			break;
		}
		case OUTPUT_PAUSE: {
			LinuxDvbPause(context, (char*)argument);
			break;
		}
		case OUTPUT_CONTINUE: {
			LinuxDvbContinue(context, (char*)argument);
			break;
		}
		case OUTPUT_FASTFORWARD: {
			return LinuxDvbFastForward(context, (char*)argument);
			break;
		}
		case OUTPUT_AVSYNC: {
			LinuxDvbAVSync(context, (char*)argument);
			break;
		}
		case OUTPUT_CLEAR: {
			LinuxDvbClear(context, (char*)argument);
			break;
		}
		case OUTPUT_PTS: {
			unsigned long long int pts = 0;
			LinuxDvbPts(context, &pts);
			*((unsigned long long int*)argument) = (unsigned long long int)pts;
			break;
		}
		case OUTPUT_SWITCH: {
			LinuxDvbSwitch(context, (char*)argument);
			break;
		}
		case OUTPUT_SLOWMOTION: {
			return LinuxDvbSlowMotion(context, (char*)argument);
			break;
		}
		case OUTPUT_AUDIOMUTE: {
			return LinuxDvbAudioMute(context, (char*)argument);
			break;
		}
		default:
#ifdef DEBUG		  
			printf("%s::%s ContainerCmd %d not supported!\n", FILENAME, __FUNCTION__, command);
#endif	
			break;
	}

#ifdef DEBUG
	printf("%s::%s exiting with value %d\n", FILENAME, __FUNCTION__, ret);
#endif

	return ret;
}

static char *LinuxDvbCapabilities[] = { "audio", "video", NULL };

struct Output_s LinuxDvbOutput = {
	"LinuxDvb",
	&Command,
	&Write,
	LinuxDvbCapabilities,

};

