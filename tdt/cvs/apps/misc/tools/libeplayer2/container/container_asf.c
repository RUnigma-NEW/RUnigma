#include <unistd.h>

// LIBEPLAYER2 Headers
#include "common.h"
#include "container.h"
#include "manager.h"
// End of LIBEPLAYER2 Headers

// MPLAYER Headers
#include "demuxer.h"
#include "stream.h"
#include "stheader.h"
// End of MPLAYER Headers

// Extern Declarations
extern int asf_check_header(demuxer_t *demuxer);
extern int demux_asf_fill_buffer(demuxer_t *demux, demux_stream_t *ds);
extern demuxer_t* demux_open_asf(demuxer_t* demuxer);
extern void demux_close_asf(demuxer_t *demuxer);
extern void demux_seek_asf(demuxer_t *demuxer,float rel_seek_secs,float audio_delay,int flags);
// End of Extern Declarations



static const char FILENAME[] = "container_asf.c";

#define CodeToInteger(a,b,c,d)                  ((a << 0) | (b << 8) | (c << 16) | (d <<24))

static demux_stream_t *ds = NULL;
static sh_audio_t     *sh_audio = NULL;
static sh_video_t     *sh_video = NULL;

static pthread_t PlayThread;
static int hasPlayThreadStarted = 0;
//static int isFirstAudioFrame = 1;

static int whileSeeking = 0;


int ASFInit(Context_t *context, char * filename) {
#ifdef DEBUG
	printf("%s::%s\n", FILENAME, __FUNCTION__);
#endif
    int i = 0;
	
    ds = (demux_stream_t*)malloc ( sizeof(demux_stream_t));
    memset (ds,0,sizeof(demux_stream_t));

    ds->demuxer = (demuxer_t*)malloc ( sizeof(demuxer_t));
    memset (ds->demuxer,0,sizeof(demuxer_t));

    ds->demuxer->audio = (demux_stream_t*)malloc ( sizeof(demux_stream_t));
    memset (ds->demuxer->audio,0,sizeof(demux_stream_t));

    ds->demuxer->video = (demux_stream_t*)malloc ( sizeof(demux_stream_t));
    memset (ds->demuxer->video,0,sizeof(demux_stream_t));

    ds->demuxer->stream = (stream_t*)malloc ( sizeof(stream_t));
    memset (ds->demuxer->stream,0,sizeof(stream_t));

    ds->demuxer->stream->fd = context->playback->fd;


    read(ds->demuxer->stream->fd,ds->demuxer->stream->buffer, 2048);


    ds->demuxer->stream->start_pos	= 0;
    ds->demuxer->stream->flags		= 6;
    ds->demuxer->stream->sector_size	= 0;
    ds->demuxer->stream->buf_pos	= 0;
    ds->demuxer->stream->buf_len	= 2048;
    ds->demuxer->stream->pos		= 2048;
    ds->demuxer->stream->start_pos	= 0;
    ds->demuxer->stream->type		= STREAMTYPE_STREAM;
    ds->demuxer->stream->end_pos = 0;
    ds->demuxer->stream->eof		= 0;
    ds->demuxer->stream->cache_pid	= 0;


    
    ds->demuxer->video->id = -1;
    ds->demuxer->audio->id = -1;

    //printf("%s::%d\n", __FUNCTION__, __LINE__);
    asf_check_header(ds->demuxer);

    demux_open_asf(ds->demuxer);
    //printf("%s::%d\n", __FUNCTION__, __LINE__);


	for(i = 0; i < MAX_V_STREAMS; i++) {
		if(ds->demuxer->v_streams[i] == NULL) continue;

		sh_video = (sh_video_t *)ds->demuxer->v_streams[i];

		if(sh_video)
		{
			printf("V: %d\n", sh_video->vid);

			if(i == 0) {
				//ds->demuxer->video->sh = sh_video;
				ds->demuxer->video->id = sh_video->vid;
			}

			char * vcodec = (char *)&sh_video->bih->biCompression;

			printf("biCompression: %s\n", vcodec);

			if (!strncmp(vcodec, "wmv3",4 ) || !strncmp(vcodec, "WMV3", 4))
			{
				Track_t Video = {
					"eng",
					"V_WMV",
					sh_video->vid,
				};
				context->manager->video->Command(context, MANAGER_ADD, &Video);

			} 
			else if (!strncmp(vcodec, "wvc1", 4) || !strncmp(vcodec, "WVC1", 4))
			{
				Track_t Video = {
					"eng",
					"V_VC1",
					sh_video->vid,
				};
				context->manager->video->Command(context, MANAGER_ADD, &Video);
			}
		}
	}

	for(i = 0; i < MAX_A_STREAMS; i++) {
		if(ds->demuxer->a_streams[i] == NULL) continue;
				
		sh_audio = (sh_audio_t *)ds->demuxer->a_streams[i];

		if(sh_audio)
		{
			printf("A: %d\n", sh_audio->aid);

			if(i == 0) {
				ds->demuxer->audio->id = sh_audio->aid;
				//ds->demuxer->audio->sh = sh_audio;
			}

			printf("WF: 0x%08x\n", sh_audio->wf->wFormatTag);

			if(sh_audio->wf->wFormatTag == 0x161) // WMA2
			{
				Track_t Audio = {
					"und",
					"A_WMA",
					sh_audio->aid,
				};
				context->manager->audio->Command(context, MANAGER_ADD, &Audio);
			} 
			else if(sh_audio->wf->wFormatTag == 0x162) // WMAP
			{
				Track_t Audio = {
					"und",
					"A_WMA",
					sh_audio->aid,
				};
				//Currently not supported
				context->manager->audio->Command(context, MANAGER_ADD, &Audio);
			}

			
		}
	}

	//exit(0);

	return 0;
}


extern unsigned char * STREAMHEADERBUFFER;
extern unsigned int STREAMHEADERBUFFERSIZE;

typedef struct
{
	unsigned char       privateData[4]; 
	unsigned int       width;
	unsigned int       height;
	unsigned int       framerate;
}bwmv_t;

extern unsigned char ASF_PRIVATE_DATA[4];


#define INVALID_PTS_VALUE                       0x200000000ull
void ASFGenerateParcel(Context_t *context, const demuxer_t *demuxer) {

	demux_stream_t * video = demuxer->video;
	demux_stream_t * audio = demuxer->audio;

	unsigned long long int Pts = 0;

	//printf("P\n");
	 if (audio->first != NULL) {
		//printf("A\n");
		demux_packet_t * current = audio->first;
		//printf("PTS: %f\n", current->pts);
		Pts = (current->pts * 90000);

		while (current != NULL) {
			context->output->audio->Write(context, current->buffer, current->len, Pts, STREAMHEADERBUFFER, STREAMHEADERBUFFERSIZE, 0, "audio");

		demux_packet_t *dn = current->next;
        free_demux_packet(current);
        current = dn;
			//current = current->next;

			Pts = INVALID_PTS_VALUE;
		}

	audio->first = audio->last = NULL;
    audio->packs = 0; // !!!!!
    audio->bytes = 0;
    if (audio->current)
        free_demux_packet(audio->current);
    audio->current = NULL;
    audio->buffer = NULL;
    audio->buffer_pos = audio->buffer_size;
    audio->pts = 0;
    audio->pts_bytes = 0;

	}

	if (video->first != NULL) {
		//printf("V\n");
		demux_packet_t * current = video->first;

		//if (!(current->flags&0x10)) {  //current frame isn't a keyframe
			//printf("\tNORMALFRAME,                 ");
		//	Pts = INVALID_PTS_VALUE;
		//} else {
		//	//printf("\tKEYFRAME,                    ");
		//	Pts = (current->pts * 90000);
		//}


		while (current != NULL) {
			Pts = (current->pts * 90000);
        	//printf("PTS: %f (%u)\n", current->pts, Pts);

			sh_video = demuxer->video->sh; sh_video->ds = demuxer->video;

			bwmv_t * priv = (bwmv_t *)malloc(sizeof(bwmv_t));
			memcpy (priv->privateData, ASF_PRIVATE_DATA, 4);
			priv->width = sh_video->bih->biWidth;
			priv->height = sh_video->bih->biHeight;
			priv->framerate = 333667;

			context->output->video->Write(context, current->buffer, current->len, Pts, (unsigned char*) priv, sizeof(bwmv_t), 0, "video");

			free(priv);

		demux_packet_t *dn = current->next;
        free_demux_packet(current);
        current = dn;
			//current = current->next;
			//Pts = INVALID_PTS_VALUE;
		}

	video->first = video->last = NULL;
    video->packs = 0; // !!!!!
    video->bytes = 0;
    if (video->current)
        free_demux_packet(video->current);
    video->current = NULL;
    video->buffer = NULL;
    video->buffer_pos = video->buffer_size;
    video->pts = 0;
    video->pts_bytes = 0;

	}
}



static void ASFThread(Context_t *context) {
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

	while ( context && context->playback && context->playback->isPlaying ) {
	    //printf("%s -->\n", __FUNCTION__);

		//IF MOVIE IS PAUSED, WAIT
		if (context->playback->isPaused) {
#ifdef DEBUG
			printf("%s::%s paused\n", FILENAME, __FUNCTION__);
#endif
			usleep(100000);
			continue;
		}

		if (context->playback->isSeeking || whileSeeking) {
#ifdef DEBUG
			printf("%s::%s seeking\n", FILENAME, __FUNCTION__);
#endif
			usleep(100000);			
			continue;
		}

		//getASFMutex(FILENAME, __FUNCTION__,__LINE__);



		if ( !demux_asf_fill_buffer(ds->demuxer,ds) ) {
#ifdef DEBUG
			printf("%s::%s demux_asf_fill_buffer failed!\n", FILENAME, __FUNCTION__);
#endif
			//releaseASFMutex(FILENAME, __FUNCTION__,__LINE__);
			
			break;
		} else {		
			ASFGenerateParcel(context, ds->demuxer);
			if (ds->demuxer->sub != NULL && ds->demuxer->sub->first != NULL) {
				ds_free_packs(ds->demuxer->sub);
			}
			
			/* Dont free the packs, as the fragments would be deleted
			if (ds->demuxer->audio != NULL && ds->demuxer->audio->first != NULL) {
				ds_free_packs(ds->demuxer->audio);
			}
			
			if (ds->demuxer->video != NULL && ds->demuxer->video->first != NULL) {
				ds_free_packs(ds->demuxer->video);
			}
			*/

			//releaseASFMutex(FILENAME, __FUNCTION__,__LINE__);
		}
	}

	hasPlayThreadStarted = 0;	// prevent locking situation when calling PLAYBACK_TERM

	context->playback->Command(context, PLAYBACK_TERM, NULL);

#ifdef DEBUG
	printf("%s::%s terminating\n",FILENAME, __FUNCTION__);
#endif
}


static int ASFPlay(Context_t *context) {
#ifdef DEBUG
	printf("%s::%s\n", FILENAME, __FUNCTION__);
#endif

	int error;
	int ret = 0;
	pthread_attr_t attr;

#ifdef DEBUG
	if ( context && context->playback && context->playback->isPlaying ) {
		printf("%s::%s is Playing\n", FILENAME, __FUNCTION__);
	} else {
		printf("%s::%s is NOT Playing\n", FILENAME, __FUNCTION__);
	}
#endif
	
	if (hasPlayThreadStarted == 0) {
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

		if((error=pthread_create(&PlayThread, &attr, (void *)&ASFThread, context)) != 0) {
#ifdef DEBUG
			  printf("%s::%s Error creating thread, error:%d:%s\n", FILENAME, __FUNCTION__,error,strerror(error));
#endif
			  hasPlayThreadStarted = 0;
			  ret = -1;
		} else {
#ifdef DEBUG
			  printf("%s::%s Created thread\n", FILENAME, __FUNCTION__);
#endif
			  hasPlayThreadStarted = 1;
		}
	} else {
#ifdef DEBUG
		printf("%s::%s A thread already exists!\n", FILENAME, __FUNCTION__);
#endif
		ret = -1;
	}
	
#ifdef DEBUG
	printf("%s::%s exiting with value %d\n", FILENAME, __FUNCTION__, ret);
#endif

	return ret;    
}

static int ASFStop(Context_t *context) {
#ifdef DEBUG
	printf("%s::%s\n", FILENAME, __FUNCTION__);
#endif

	int i;
	int ret = 0;
	int wait_time = 20;
	
	while ( (hasPlayThreadStarted != 0) && (wait_time--) > 0 ) {
		#ifdef DEBUG  
		printf("%s::%s Waiting for ASF thread to terminate itself, will try another %d times\n", FILENAME, __FUNCTION__, wait_time);
		#endif
		usleep(100000);
	}

	if (wait_time == 0) {
#ifdef DEBUG  
		printf("%s::%s Timeout waiting for ASF thread!\n", FILENAME, __FUNCTION__);
#endif
		ret = -1;
	} else {
		//getASFMutex(FILENAME, __FUNCTION__,__LINE__);
		
		if (ds && ds->demuxer) {		  
			demux_close_asf(ds->demuxer);

			free(ds->demuxer->stream);
			ds->demuxer->stream = NULL;

			free(ds->demuxer->sub);
			ds->demuxer->sub = NULL;

			free(ds->demuxer->video);
			ds->demuxer->video = NULL;
		
			free(ds->demuxer->audio);
			ds->demuxer->audio = NULL;
			
			for(i=0;i<MAX_A_STREAMS;i++){
				free(ds->demuxer->a_streams[i]);
				ds->demuxer->a_streams[i]=NULL;
			}
			
			for(i=0;i<MAX_V_STREAMS;i++){
				free(ds->demuxer->v_streams[i]);
				ds->demuxer->v_streams[i]=NULL;
			}
			
			for(i=0;i<MAX_S_STREAMS;i++){
				free(ds->demuxer->s_streams[i]);
				ds->demuxer->s_streams[i]=NULL;
			}
			
			free(ds->demuxer);   
			ds->demuxer = NULL;
		}

		free(ds);
		ds = NULL;

		//releaseASFMutex(FILENAME, __FUNCTION__,__LINE__);
	}

	return ret;
}

static int Command(void  *_context, ContainerCmd_t command, void * argument) {
Context_t  *context = (Context_t*) _context;
	#ifdef DEBUG
	printf("%s::%s Command %d\n", FILENAME, __FUNCTION__, command);
	#endif

	int ret = 0;
	
	switch(command) {
		case CONTAINER_INIT: {
			char * FILENAME = (char *)argument;
			ret = ASFInit(context, FILENAME);
			break;
		}
		case CONTAINER_PLAY: {
			ret = ASFPlay(context);
			break;
		}
		case CONTAINER_STOP: {
			ret = ASFStop(context);
			break;
		}
		case CONTAINER_SEEK: {
            //if (ds && ds->demuxer)
			//    demux_audio_seek (ds->demuxer, 100.0, 100, 0);
			break;
		}
		case CONTAINER_LENGTH: {
			//double length = 0;
			//if (ds != NULL && ds->demuxer  != NULL && AUDIOGetLength(ds->demuxer, &length)!=0)
			//	*((double*)argument) = (double)0;
			//else
			//	*((double*)argument) = (double)length;
			break;
		}
		case CONTAINER_INFO: {
			//if (ds && ds->demuxer)
			//	AUDIOGetInfo(ds->demuxer, (char **)argument);
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

static char *ASFCapabilities[] = { "asf", "wmv", "wma", NULL };

Container_t ASFContainer = {
	"ASF",
	&Command,
	ASFCapabilities,
};
