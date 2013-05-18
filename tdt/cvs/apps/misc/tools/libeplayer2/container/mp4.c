#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include "container.h"
#include "common.h"

#include "mp4.h"


pthread_mutex_t mutex;
static const char FILENAME[] = "mp4.c";

static Mp4Info_t*       Mp4Info      = NULL;
static int              TrackCount   = -1;
static pthread_t PlayThread;
static unsigned char isPlayThreadCreated = 0;

static off_t stream_off =0;

/* #define DEBUG
*/

static off_t myftello(Context_t* context,FILE* stream){
	if ((context->playback->isHttp) || (context->playback->isUPNP))
	   return stream_off;
	return ftello (stream);
}

static size_t myfread(Context_t* context,void* ptr,size_t size, size_t count, FILE* stream){
	size_t n=fread (ptr, size, count, stream);
	stream_off+=(n*size);
	
#ifdef DEBUG
	if (n != count) printf("%s: count %d != n %d\n", __func__, count, n);
#endif	
	return n;
}

static int myfseeko(Context_t* context,FILE** stream,off_t off, int whence){
	
#ifdef DEBUG
	printf("%s ",__FUNCTION__);
	printf("Stream Pos: 0x%lx ",(long unsigned int) myftello(context, *stream));
	printf("Offset: 0x%lx (%lu)", (long unsigned int) off, (long unsigned int) off);
	
	if(whence==SEEK_SET)
	   printf("SEEK_SET\n"); 
        else 
	   printf("SEEK_CUR\n");
#endif	

	if ((context->playback->isHttp) || (context->playback->isUPNP))
	{
	    int fd;
		if(whence==SEEK_SET){
			if(off<stream_off || off-stream_off>32000){
				char *ext = NULL;
				
				fclose(*stream);
				close(context->playback->fd);
				stream_off=off;
				
				if (context->playback->isHttp)
				   fd = openHttpConnection(context, &ext ,off);
				else
				   fd = openUPNPConnection(context, off);

                                if (fd < 0) 
				   return fd;

				context->playback->fd=fd;
				if (ext != NULL)
				   free(ext);
				
				*stream            = fdopen(context->playback->fd, "r");

			}else
			{
				char t;
				int x,n = off - stream_off;
				
				for( x = 0; x < n; x++)
				    myfread(context,&t,1,1, *stream);
			}
		}else if(whence==SEEK_CUR){
			if(off<0 || off>32000){
				char *ext = NULL;
				fclose( *stream);
				close(context->playback->fd);
				stream_off+=off;
				
				if (context->playback->isHttp)
				   fd = openHttpConnection(context, &ext ,stream_off);
				else
				   fd = openUPNPConnection(context, stream_off);

                                if (fd < 0) 
				   return fd;

				context->playback->fd=fd;
				if (ext != NULL)
				   free(ext);
				*stream            = fdopen(context->playback->fd, "r");
			}else{
				char t;
				int x;
				for( x = 0; x < off; x++)
				   myfread(context,&t,1,1, *stream);
			}
		}
		return 0;
	}
	return fseeko (*stream, off, whence);
	
}

/*{{{  CheckCode*/
static bool CheckCode (unsigned int Code, Mp4Container_t* Container)
{
    unsigned int i;
    for (i = 0; i < RECOGNISED_CODES; i++)
    {
        if (Code == CodeTable[i].Code)
            break;
    }
    if (i == RECOGNISED_CODES)
    {
        printf ("%s: ERROR - Unrecognised code '%.4s'\n", __FUNCTION__, (char*)&Code);
        return true;
    }
    /*
    if (CodeTable[i].Parent != *Container)
        printf("%s: '%.4s' record encountered - terminating '%.4s'\n", __FUNCTION__, (char*)&Code, (char*)Container);
    */
    *Container  = CodeTable[i].Next;
    return CodeTable[i].Skip;
}
/*}}}  */
/*{{{  FreeTrackData*/
static void FreeTrackData (Mp4Track_t* Track)
{
    if (Track == NULL)
        return;

    /* free chunk map */
    if (Track->ChunkMap != NULL)
    {
        free (Track->ChunkMap);
        Track->ChunkMap             = NULL;
        Track->ChunkMapCount        = 0;
    }
    /* free chunks */
    if (Track->Chunk != NULL)
    {
        free (Track->Chunk);
        Track->Chunk                = NULL;
        Track->ChunkCount           = 0;
    }
    /* free decoding time table */
    if (Track->DecodingTime != NULL)
    {
        free (Track->DecodingTime);
        Track->DecodingTime         = NULL;
        Track->DecodingTimeCount    = 0;
    }
    /* free composition time table */
    if (Track->CompositionTime != NULL)
    {
        free (Track->CompositionTime);
        Track->CompositionTime      = NULL;
        Track->CompositionTimeCount = 0;
    }
    /* free sample table */
    if (Track->KeyFrameTable != NULL)
    {
        free (Track->KeyFrameTable);
        Track->KeyFrameTable        = NULL;
        Track->KeyFrameTableCount   = 0;
    }
    /* free samples */
    if (Track->Samples != NULL)
    {
        free (Track->Samples);
        Track->Samples              = NULL;
        Track->SampleCount          = 0;
    }
    /* free sequence data table */
    if (Track->SequenceData != NULL)
    {
        free (Track->SequenceData);
        Track->SequenceData         = NULL;
    }
}
/*}}}  */
/*{{{  BuildTrackMap*/
static PlayerStatus_t BuildTrackMap (Mp4Track_t* Track)
{
    int                 i, j;
    unsigned int        LastChunk;
    unsigned int        Sample;
    unsigned int        Count;
    unsigned int        DecodingTime;
    unsigned int        CompositionTime;
    unsigned int        CompositionEntry;
    unsigned int        CompositionCount;

#ifdef DEBUG
    printf ("%s: MOV track type '%.4s': %d chunks, %d samples\n", __FUNCTION__,
            (char*)&Track->Type, Track->ChunkCount, Track->SampleCount);
#endif

    /* transfer data from chunk map to chunks */
    i           = Track->ChunkMapCount;
    LastChunk   = Track->ChunkCount;
    while (i > 0)
    {
        --i;
        for (j = Track->ChunkMap[i].FirstChunk; j < LastChunk; j++)
        {
            Track->Chunk[j].Description =  Track->ChunkMap[i].SampleDescriptionIndex;
            Track->Chunk[j].SampleCount =  Track->ChunkMap[i].SamplesPerChunk;
        }
        LastChunk       = Track->ChunkMap[i].FirstChunk;
    }

    /* free chunk map table */
    free (Track->ChunkMap);
    Track->ChunkMap             = NULL;
    Track->ChunkMapCount        = 0;

    /* Cross check */
    Sample      = 0;
    for (i = 0; i < Track->ChunkCount; i++)
    {
        Track->Chunk[i].FirstSample     = Sample;
        Sample          += Track->Chunk[i].SampleCount;
    }

    Count       = 0;
    for (i = 0; i < Track->DecodingTimeCount; i++)
        Count           += Track->DecodingTime[i].Count;

    if (Count != Sample)
    {
        printf ("%s: Sample Table and Chunk Map sample count differ (%i vs %i)\n", __FUNCTION__, Count, Sample);
        if (Count > Sample)
            Sample      = Count;
    }

    /* Create video sample sizes for fixed-size video frames */
    if ((Track->SampleCount == 0) && (Track->Type != CodeToInteger('s','o','u','n')))
    {
        Track->SampleCount      = Sample;
        Track->Samples          = malloc (sizeof(Mp4Sample_t) * Sample);
        if (Track->Samples == NULL)
        {
            printf("%s: ERROR - Unable to create track sample list - not enough memory (%d)\n", __FUNCTION__, sizeof(Mp4Sample_t) * Sample);
            return PlayerError;
        }

        for (i = 0; i < Sample; i++)
            Track->Samples[i].Length    = Track->SampleSize;
        Track->SampleSize       = 0;
    }

    if (Track->SampleCount == 0)
    {
        if ((Track->DecodingTimeCount == 1) || ((Track->DecodingTimeCount == 2) && Track->DecodingTime[1].Count == 1))
             Track->Duration    = Track->DecodingTime[0].Delta;
        else
        {
            printf("%s: ERROR - Constant samplesize & variable duration not yet supported\n",__FUNCTION__);
            return PlayerError;
        }
    }

    if (Track->SampleCount < Sample)
    {
        printf ("%s: DecodingTimeCount or Chunk Map bigger than sample count (%i vs %i)\n", __FUNCTION__,
                Sample, Track->SampleCount);
        Track->SampleCount      = Sample;
        Track->Samples          = realloc (Track->Samples, sizeof(Mp4Sample_t) * Sample);
    }

    Sample                      = 0;
    DecodingTime                = 0;
    CompositionEntry            = 0;
    CompositionCount            = 0;
    Track->DecodingTimeValues   = false;
    Track->TimeDelta            = 0xffffffff;

    for (i = 0; i < Track->DecodingTimeCount; i++)
    {
        printf ("%s: TimeScale %d, Delta %d\n", __FUNCTION__, Track->TimeScale, Track->DecodingTime[i].Delta);
        Track->TimeDelta        = Track->DecodingTime[0].Delta;

        if (Track->CompositionTimeCount == 0)
        {
            printf ("    *** Composition Data not present.\n" );
            Track->DecodingTimeValues       = true;
        }
        for (j = 0; j < Track->DecodingTime[i].Count; j++)
        {
            if (Track->CompositionTimeCount > 0)
                CompositionTime             = DecodingTime + Track->CompositionTime[CompositionEntry].Offset;
            else
                CompositionTime             = DecodingTime;
            /*
            The following adjustment forces the first PTS to be reduced by 1 delta.  If this isn't
            applied the composition offset for the first frame would be non-zero and so the video
            will play 1 frame behind the audio which does not usually have composition offsets.

            This is equivalent to initialising DecodingTime to -Track->DecodingTime[0].Delta;

            In one closed gop example, Delta is 1001, Timescale is 24000 giving a frame rate of
            23.976fps and the composition entries go 1001, 3003, 0, 0.  This would result in the
            PTS of the first video frame being 3753 rather than 0.

            In an open gop example, Delta is 1001, Timescale is 24000 giving a frame rate of
            23.976fps and the composition entries go 3003, 1001, 2002.  This would result in the
            PTS of the first video frame being 11261 rather than 7507.
            */
            if ((j == 0) && (i == 0) && (Track->CompositionTimeCount > 0))
                Track->Samples[Sample].Pts      = ((unsigned long long)(CompositionTime - Track->DecodingTime[i].Delta) * 90000ull) / Track->TimeScale;
            else
            {
                Track->Samples[Sample].Pts      = ((unsigned long long)CompositionTime * 90000ull) / Track->TimeScale;
                DecodingTime                   += Track->DecodingTime[i].Delta;
            }
            Sample++;
            if (Track->CompositionTimeCount > 0)
            {
                CompositionCount++;
                if (CompositionCount == Track->CompositionTime[CompositionEntry].Count)
                {
                    CompositionEntry++;
                    CompositionCount            = 0;
                    if ((CompositionEntry == Track->CompositionTimeCount) && (Sample < Track->SampleCount))
                        printf ("%s: Error run out of composition data Sample %d of %d\n", __FUNCTION__,
                                Sample, Track->SampleCount);
                }
            }

        }
    }

    /* Derive sample offsets in file */
    for (i = 0; i < Track->ChunkCount; i++)
    {
        off_t   Offset  = Track->Chunk[i].Offset;
        for (j=Track->Chunk[i].FirstSample; j<(Track->Chunk[i].FirstSample+Track->Chunk[i].SampleCount); j++)
        {
            Track->Samples[j].Offset    = Offset;
            Offset                     += Track->Samples[j].Length;
        }
    }

    /* free time tables */
    free (Track->DecodingTime);
    Track->DecodingTime         = NULL;
    Track->DecodingTimeCount    = 0;
    free (Track->CompositionTime);
    Track->CompositionTime         = NULL;
    Track->CompositionTimeCount    = 0;

    /* Fill in keyframe info then free keyframe table. All frames are key if table not present */
    if (Track->KeyFrameTableCount == 0)
        for (i = 0; i < Track->SampleCount; i++)
            Track->Samples[i].Flags                             = MP4_KEY_FRAME;
    else
        for (i = 0; i < Track->KeyFrameTableCount; i++)
            Track->Samples[Track->KeyFrameTable[i]-1].Flags     = MP4_KEY_FRAME;

    free (Track->KeyFrameTable);
    Track->KeyFrameTable        = NULL;
    Track->KeyFrameTableCount   = 0;

    return PlayerNoError;
}

/*}}}  */
/*{{{  FindHeader*/
/*
#define AC3_START_CODE                  0x0b77
#define AC3_START_CODE_MASK             0xffff
#define MP3_START_CODE                  0xffe0
#define MP3_START_CODE_MASK             0xffe0
#define AAC_START_CODE                  0xffe0
#define AAC_START_CODE_MASK             0xffe0


static unsigned int FindHeader (Context_t* context)
// Return offset from start of file of first header 
{
    int                 j;
    //Mp4Info_t*          Mp4Info         = (Mp4Info_t*)Context->MediaContext;
    Mp4Track_t*         AudioTrack      = &Mp4Info->Track[Mp4Info->AudioTrack];
    unsigned int        AudioChunk      = AudioTrack->ChunkToPlay;
    Mp4Sample_t*        Sample          = &AudioTrack->Samples[AudioTrack->Chunk[AudioChunk].FirstSample];
    unsigned char*      Data            = Context->Data;
    unsigned int        StartCode;
    unsigned int        StartCodeMask;

    switch (Context->Audio->Encoding)
    {
        case AUDIO_ENCODING_AC3:
            StartCode           = AC3_START_CODE;
            StartCodeMask       = AC3_START_CODE_MASK;
            break;
        case AUDIO_ENCODING_MP3:
            StartCode           = MP3_START_CODE;
            StartCodeMask       = MP3_START_CODE_MASK;
            break;
        case AUDIO_ENCODING_AAC:
            StartCode           = AAC_START_CODE;
            StartCodeMask       = AAC_START_CODE_MASK;
            break;
        default:
            return Sample->Pts;
    }

    fseeko (Context->File, AudioTrack->Chunk[AudioChunk].Offset, SEEK_SET);
    fread  (Data, 1, Sample->Length, Context->File);
    for (j = 0; j < Sample->Length; j++)
    {
        if ((((Data[j] << 8) | Data[j+1]) & StartCodeMask) == StartCode)
            break;
    }
    if (j < Sample->Length)
    {
        unsigned int    PtsDiff = (Sample+1)->Pts - Sample->Pts;

        printf("%s: In Pts %u, Out Pts %u\n", __FUNCTION__, Sample->Pts, Sample->Pts + ((j * PtsDiff) / Sample->Length));
        return Sample->Pts + ((j * PtsDiff) / Sample->Length);
    }
    return Sample->Pts;
}*/

/*}}}  */
/*{{{  SeekH264Mp4*/
/*static PlayerStatus_t SeekH264Mp4 (Context_t* Context, int MilliSeconds)
{
    int                 i, j;
    //Mp4Info_t*          Mp4Info         = (Mp4Info_t*)Context->MediaContext;
    Mp4Track_t*         AudioTrack      = NULL;
    unsigned long long  AudioPts        = 0ull;

    Mp4Track_t*         VideoTrack      = NULL;
    unsigned long long  VideoPts        = 0ull;

    unsigned long long  DesiredPts;
    unsigned long long  LastPts;
    bool                SampleFound     = false;

    if (Mp4Info->AudioTrack != -1)
        AudioTrack              = &Mp4Info->Track[Mp4Info->AudioTrack];
    if (Mp4Info->VideoTrack != -1)
        VideoTrack              = &Mp4Info->Track[Mp4Info->VideoTrack];

    DesiredPts  = VideoTrack->Pts + (MilliSeconds * 90);
    LastPts     = (unsigned long long)VideoTrack->Samples[VideoTrack->SampleCount-1].Pts;
    if (DesiredPts >= LastPts)
    {
        printf ("%s  Error - Attempt to seek beyond end of movie Pts = %llu, LastPts %llu\n", __FUNCTION__, DesiredPts, LastPts);
        return PlayerError;
    }

    AudioIoctl (Context, AUDIO_CLEAR_BUFFER, NULL);
    VideoIoctl (Context, VIDEO_CLEAR_BUFFER, NULL);

    printf ("%s  Seeking to %u of %u seconds\n", __FUNCTION__, (unsigned int)(DesiredPts/90000), (unsigned int)(LastPts/90000));

    for (i = VideoTrack->ChunkToPlay; (i < VideoTrack->ChunkCount) && !SampleFound; i++)
    {
        for (j=VideoTrack->Chunk[i].FirstSample; j<(VideoTrack->Chunk[i].FirstSample+VideoTrack->Chunk[i].SampleCount); j++)
        {
            VideoPts    = (unsigned long long)VideoTrack->Samples[j].Pts;
            if ((VideoPts >= DesiredPts) && ((VideoTrack->Samples[j].Flags & MP4_KEY_FRAME) != 0))
            {
                VideoTrack->ChunkToPlay         = i;
                VideoTrack->SampleToPlay        = j;
                SampleFound                     = true;
                break;
            }
        }
    }
    if (i < VideoTrack->ChunkCount)
    {
        VideoTrack->Pts         = VideoPts;
        SampleFound             = false;
    }
    for (i = AudioTrack->ChunkToPlay; (i < AudioTrack->ChunkCount) && !SampleFound; i++)
    {
        for (j=AudioTrack->Chunk[i].FirstSample; j<(AudioTrack->Chunk[i].FirstSample+AudioTrack->Chunk[i].SampleCount); j++)
        {
            AudioPts    = (unsigned long long)AudioTrack->Samples[j].Pts;
            if (AudioPts >= VideoPts)
            {
                AudioTrack->ChunkToPlay         = i;
                AudioTrack->SampleToPlay        = j;
                SampleFound                     = true;
                break;
            }
        }
    }
    if (i < AudioTrack->ChunkCount)
        AudioTrack->Pts     = AudioPts;

    printf ("%s  Audio Chunk %u, Sample %u, Pts = %llu, Video Chunk %u, Sample %u, Pts = %llu\n", __FUNCTION__,
                 AudioTrack->ChunkToPlay, AudioTrack->SampleToPlay, AudioTrack->Pts,
                 VideoTrack->ChunkToPlay, VideoTrack->SampleToPlay, VideoTrack->Pts);

    return PlayerNoError;
}*/
/*}}}  */
/*{{{  PlayH264Mp4Audio */

//static unsigned char*      AudioData = NULL;
//static unsigned char*      Data = NULL;

/*}}}  */
/*{{{  PlayH264Mp4File*/
static void PlayH264Mp4File (Context_t* Context)
{
    /*{{{   locals*/
	unsigned char*      Data                    = NULL; //Context->Data+PADDING_LENGTH;

	FILE*               Mp4File                 = fdopen(Context->playback->fd, "r"); //Context->File;



	Mp4Track_t*         VideoTrack              = NULL;
	off_t               VideoPosition;
	unsigned long long  VideoPts                = 0ull;
	unsigned int        SampleSize              = 0;
	int                 VideoChunk              = 0;
	int                 VideoSample             = 0;

	Mp4Track_t*         AudioTrack              = NULL;
	//Context_t*    Context                 = (Context_t*)ThreadParam;

	unsigned char*      AudioData               = NULL;
	
	off_t               AudioPosition;
	//unsigned int        SampleSize              = 0;
	unsigned long long  AudioPts                = 0ull;
	int                 AudioChunk              = 0;
	int                 AudioSample             = 0;
	float		    frameRate		    = 0;

#ifdef DEBUG
        printf("%s >\n", __func__);
#endif
	
	if (Mp4Info->VideoTrack != -1)
	{
		VideoTrack                      = &Mp4Info->Track[Mp4Info->VideoTrack];
		VideoTrack->ChunkToPlay         = 0;
		VideoTrack->SampleToPlay        = 0;
		VideoChunk                      = 1;    /* Force to be different from ChunkToPlay so reload pts */
		VideoTrack->Pts                 = 0ull;
		/* Quack: find framerate */
		frameRate			= VideoTrack-> TimeScale;
		while(frameRate>100.0)frameRate/=10.0;
	}

	if ((Mp4Info->AudioTrack != -1) && (AudioTrack == NULL))
	{
#ifdef DEBUG
		printf ("%s: Initialising AudioTrack\n", __FUNCTION__);
#endif
		AudioTrack                      = &Mp4Info->Track[Mp4Info->AudioTrack];
		AudioTrack->ChunkToPlay         = 0;
		AudioTrack->SampleToPlay        = 0;
		AudioChunk                      = 1;    /* Force to be different from ChunkToPlay so reload pts */
		//AudioTrack->Pts                 = FindHeader (Context);
	}
	
	while(Context->playback->isPlaying) {

		//IF MOVIE IS PAUSE, WAIT 
		while (Context->playback->isPaused) {printf("paused\n"); usleep(100000);}
		pthread_mutex_lock(&mutex);
		// Values are recalculated in case we have skipped
		VideoChunk          = VideoTrack->ChunkToPlay;
		AudioChunk          = AudioTrack->ChunkToPlay;

		if(AudioChunk >= AudioTrack->ChunkCount && VideoChunk >= VideoTrack->ChunkCount) return; /* hellmaster1024: EoF detection */
            
#ifdef DEBUG
		printf("Audio: %d, Video: %d\n", Context->playback->isAudio, Context->playback->isVideo);
#endif

		/*i merged the functionality of playThread and audioThread because streaming works much better this way*/
		if(Context->playback->isAudio && (AudioTrack->Samples[AudioTrack->SampleToPlay].Offset < VideoTrack->Samples[VideoTrack->SampleToPlay].Offset)){
#ifdef DEBUG
		printf("audio comes first %llu ",AudioTrack->Samples[AudioTrack->SampleToPlay].Offset);
		printf("then video %llu\n",VideoTrack->Samples[VideoTrack->SampleToPlay].Offset);
#endif
			if (Context->playback->isAudio) {
				if (AudioChunk < AudioTrack->ChunkCount) {
					for (AudioSample=AudioTrack->SampleToPlay; AudioSample<(AudioTrack->Chunk[AudioChunk].FirstSample+AudioTrack->Chunk[AudioChunk].SampleCount); AudioSample++) {
			
						if (!Context->playback->isPlaying)return;
			
						SampleSize = AudioTrack->Samples[AudioSample].Length;
						AudioPosition       = AudioTrack->Samples[AudioSample].Offset;
						AudioPts = AudioTrack->Samples[AudioSample].Pts;
							
						AudioData = malloc(SampleSize);
						
						if (myfseeko(Context, &Mp4File, AudioPosition, SEEK_SET) < 0)
						{
						    printf("%s: line %d seek failed\n", __func__, __LINE__);
						    free(AudioData);
                                                    fclose(Mp4File);
		                                    return;
						} 

						if (myfread(Context, AudioData, 1, SampleSize, Mp4File) == 0)
						{
						    printf("%s: line %d read failed\n", __func__, __LINE__);
						    free(AudioData);
                                                    fclose(Mp4File);
		                                    return;
						}
						
						Context->output->audio->Write(Context, AudioData, SampleSize, AudioPts, AudioTrack->SequenceData, AudioTrack->SequenceDataLength, 0, "audio");
						
						free(AudioData);
	
					}
					AudioTrack->SampleToPlay    = AudioSample;
					AudioChunk++;
					AudioTrack->ChunkToPlay     = AudioChunk;
				}
			}
		}else{
#ifdef DEBUG
			printf("video comes first %llu ",VideoTrack->Samples[VideoTrack->SampleToPlay].Offset);
			printf("then audio %llu\n",AudioTrack->Samples[AudioTrack->SampleToPlay].Offset);
#endif
			if (Context->playback->isVideo) {
				if (VideoChunk < VideoTrack->ChunkCount) {
					for (VideoSample=VideoTrack->SampleToPlay; VideoSample<(VideoTrack->Chunk[VideoChunk].FirstSample+VideoTrack->Chunk[VideoChunk].SampleCount); VideoSample++) {
						if (!Context->playback->isPlaying)return;
	
						SampleSize = VideoTrack->Samples[VideoSample].Length;
						VideoPosition   = VideoTrack->Samples[VideoSample].Offset;
						VideoPts = VideoTrack->Samples[VideoSample].Pts;
						Data = malloc(SampleSize);
	
						if (myfseeko(Context, &Mp4File, VideoPosition, SEEK_SET) < 0)
						{
						   printf("%s: line %d seek failed\n", __func__, __LINE__);
		                                   free(Data);
                                                   fclose(Mp4File);
						   return;
						}
						
						if (myfread(Context, Data, 1, SampleSize, Mp4File) == 0)
						{
						   printf("%s: line %d read failed\n", __func__, __LINE__);
		                                   free(Data);
                                                   fclose(Mp4File);
						   return;
						}
						
						Context->output->video->Write(Context, Data, SampleSize, VideoPts, VideoTrack->SequenceData, VideoTrack->SequenceDataLength, frameRate, "video");
	
						free(Data);
            				}
					VideoTrack->SampleToPlay    = VideoSample;
					VideoChunk++;
					VideoTrack->ChunkToPlay     = VideoChunk;
				}
			}
		}
	pthread_mutex_unlock(&mutex);
	}
#ifdef DEBUG
        printf("%s <\n", __func__);
#endif
}
/*}}}  */

/*{{{  StartMp4*/
static int StartMp4 (Context_t* context)
{
    FILE*                       Mp4File         = fdopen(context->playback->fd, "r");
    unsigned int                Code;
    unsigned long long int      Length          = 0;
    off_t                       BoxStart        = 0;
    off_t                       BoxEnd          = 0;
    int                         i;
    //Mp4Info_t*                  Mp4Info;
    unsigned int                TrackSelect     = 0;
    Mp4Container_t                 Container       = File;
    unsigned int                EntryCount;
    bool                        NewTrack        = true; /* Flag so that track updated by both mdhd and hdlr */
    int Status = 0;
    
    //Context->StartOffset        = 0;
#ifdef DEBUG
    printf("%s >\n", __func__);
    printf(" start %X\n", (unsigned int) myftello(context, Mp4File));
#endif
    
    TrackCount      = -1;
    
    pthread_mutex_init(&mutex,0);
    
    Mp4Info     = (Mp4Info_t*)malloc (sizeof(Mp4Info_t));
    if (Mp4Info == NULL)
    {
        printf( "%s: Unable to create MP4 context\n", __FUNCTION__);
        return -PlayerNoMemory;
    }
    else
        memset (Mp4Info, 0, sizeof(Mp4Info_t));

    Mp4Info->VideoTrack         = -1;
    Mp4Info->AudioTrack         = -1;

#ifdef DEBUG
    printf ("%s::%s (%d)\n", FILENAME, __FUNCTION__, __LINE__);
#endif

    while (!feof (Mp4File))
    {
        /* Read next box length and type */
        BoxStart        = myftello (context,Mp4File);
        myfread(context, &Length, sizeof(unsigned int), 1, Mp4File);
        myfread(context, &Code,   sizeof(unsigned int), 1, Mp4File);

        if (feof(Mp4File))
	{
#ifdef DEBUG
            printf("end of file\n");
#endif
            /* this must not be an error, but we must leave loop */
	    break;
        }

        Length          = BE2ME (Length);                               /* Convert Length from Little to Big endian */
        if (Length == 1)                                                /* Length == 1 indicates 64 bit length follows */
        {
            myfread(context, &Length, sizeof(unsigned long long int), 1, Mp4File);
            Length      = BE2ME64 (Length);                             /* Convert Length from Little to Big endian */
        }
        else if (Length == 0)
        {
            printf ("%s: Last Box '%.4s'\n", __FUNCTION__, (char*)(&Code));
            break;
        }
        else if (Length < 8)                                            /* Chunk too short */
        {
            printf ("%s: Invalid chunk length %lld\n", __FUNCTION__, Length);
            break;
        }
        BoxEnd          = BoxStart + Length;
	
        if (CheckCode (Code, &Container)){               /* true indicates skip contents of box */
	    if (
	        ((context->playback->isHttp) || (context->playback->isUPNP)) && 
		 (context->playback->size>0 && context->playback->size<=BoxEnd))
	    {
		printf("skipping seek because offset>filesize\n");
		if (myfseeko(context, &Mp4File, BoxStart, SEEK_SET) < 0)
		{
		   printf("seek1 failed\n");
		   goto StartMp4_error;
		}
		break;
	    }
	    
            if (myfseeko(context, &Mp4File, BoxEnd, SEEK_SET) < 0)
            {
		   printf("seek2 failed->BoxEnd %lld, BoxStart %lld, Length %lld\n", BoxEnd, BoxStart, Length);
		   goto StartMp4_error;

	    }
	}

        switch (Code)
        {
            case CodeToInteger('m','d','h','d'):
            /*{{{  */
            {
                unsigned int    Version;
                unsigned int    TimeScale;

                if (NewTrack)
                {
                    TrackCount++;

#ifdef DEBUG
                    printf("NewTrack Count %d\n", TrackCount);
#endif
                    TrackSelect         = TrackCount;
                    NewTrack            = false;
                }
                else
                    NewTrack            = true;

                myfread(context, &Version, sizeof(unsigned int), 1, Mp4File);
                if (Version == 0)       /* Indicates 32 bit structure */
                {
                    unsigned int    Duration;

                    if (myfseeko(context, &Mp4File, sizeof(unsigned int) * 2, SEEK_CUR) < 0)
		    {
		       printf("seek3 failed\n");
		       goto StartMp4_error;
		    }

                    if (myfread(context, &TimeScale, sizeof(unsigned int), 1, Mp4File) == 0)
		       goto StartMp4_error;

                    if (myfread(context, &Duration,  sizeof(unsigned int), 1, Mp4File) == 0)
		       goto StartMp4_error;
			
                    Mp4Info->Track[TrackSelect].Duration    = (unsigned long long)BE2ME(Duration);
                }
                else
                {
                    unsigned long long  Duration;

                    if (myfseeko(context, &Mp4File, sizeof(unsigned long long) * 2, SEEK_CUR) < 0)
		    {
		       printf("seek4 failed\n");
		       goto StartMp4_error;
                    }
                    if (myfread(context, &TimeScale, sizeof(unsigned int), 1, Mp4File) == 0)
		       goto StartMp4_error;
                    if (myfread(context, &Duration,  sizeof(unsigned long long), 1, Mp4File) == 0)
		       goto StartMp4_error;

                    Mp4Info->Track[TrackSelect].Duration    = BE2ME64(Duration);
                }
                Mp4Info->Track[TrackSelect].TimeScale       = BE2ME(TimeScale);
#ifdef DEBUG
                printf ("%s: 'mdhd' Track %d, TimeScale = %u, Duration = %llu (%llu)\n", __FUNCTION__, TrackSelect,
                        Mp4Info->Track[TrackSelect].TimeScale, Mp4Info->Track[TrackSelect].Duration,
                        Mp4Info->Track[TrackSelect].Duration/Mp4Info->Track[TrackSelect].TimeScale);
#endif

                if (myfseeko(context, &Mp4File, BoxEnd, SEEK_SET) < 0)
		{
		       printf("seek5 failed\n");
		       goto StartMp4_error;
		}
                break;
            }
            /*}}}  */
            case CodeToInteger('h','d','l','r'):
            /*{{{  */
            {
                unsigned int  HandlerType;

                if (myfseeko(context, &Mp4File, sizeof(unsigned int) * 2, SEEK_CUR) < 0)     /* skip version and pre_defined */
		{
		       printf("seek6 failed\n");
		       goto StartMp4_error;
                }
		myfread(context, &HandlerType, sizeof(unsigned int), 1, Mp4File);

                if (NewTrack)
                {
                    TrackCount++;

#ifdef DEBUG
                    printf("NewTrack Count %d\n", TrackCount);
#endif

                    TrackSelect         = TrackCount;
                    NewTrack            = false;
                }
                else
                    NewTrack            = true;

                if (HandlerType == CodeToInteger('s','o','u','n'))
                    Mp4Info->AudioTrack     = TrackSelect;
                else if (HandlerType == CodeToInteger('v','i','d','e'))
                    Mp4Info->VideoTrack     = TrackSelect;
                Mp4Info->Track[TrackSelect].Type    = HandlerType;
#ifdef DEBUG
                printf ("%s: Track %d type = '%.4s'\n", __FUNCTION__, TrackSelect, (char*)&HandlerType);
#endif

                if (myfseeko(context, &Mp4File, BoxEnd, SEEK_SET) < 0)
		{
		       printf("seek7 failed\n");
		       goto StartMp4_error;
                }
		break;
            }
            /*}}}  */
            case CodeToInteger('s','t','t','s'):
            /*{{{  */
            {
                if (myfseeko(context, &Mp4File, sizeof(unsigned int), SEEK_CUR) < 0)                        /* skip version no */
		{
		       printf("seek8 failed\n");
		       goto StartMp4_error;
                }
		
                myfread(context, &EntryCount, sizeof(unsigned int), 1, Mp4File);
                EntryCount      = BE2ME (EntryCount);

#ifdef DEBUG
                printf("%s: stts Found %d\n",__FUNCTION__, EntryCount);
#endif
                Mp4Info->Track[TrackSelect].DecodingTime        = (Mp4TimeToSample_t*)calloc (EntryCount, sizeof(Mp4TimeToSample_t));
                if (Mp4Info->Track[TrackSelect].DecodingTime == NULL)
                {
                    printf ("%s: ERROR - unable to create Sample table - need %d bytes\n", __FUNCTION__, EntryCount * sizeof(Mp4TimeToSample_t));
                    return -PlayerNoMemory;
                }
                else
                {
                    Mp4Info->Track[TrackSelect].DecodingTimeCount       = EntryCount;
                    myfread(context, Mp4Info->Track[TrackSelect].DecodingTime, sizeof(Mp4TimeToSample_t), EntryCount, Mp4File);
                    for (i=0; i<EntryCount; i++)            /* convert sample table to little endian */
                    {
                        Mp4Info->Track[TrackSelect].DecodingTime[i].Count = BE2ME (Mp4Info->Track[TrackSelect].DecodingTime[i].Count);
                        Mp4Info->Track[TrackSelect].DecodingTime[i].Delta = BE2ME (Mp4Info->Track[TrackSelect].DecodingTime[i].Delta);
                    }
                }
                break;
            }
            /*}}}  */
            case CodeToInteger('c','t','t','s'):
            /*{{{  */
            {
                if (myfseeko(context, &Mp4File, sizeof(unsigned int), SEEK_CUR) < 0)                        /* skip version no */
		{
		       printf("seek9 failed\n");
		       goto StartMp4_error;
                }
                myfread(context, &EntryCount, sizeof(unsigned int), 1, Mp4File);
                EntryCount      = BE2ME (EntryCount);

#ifdef DEBUG
                printf("%s: stts Found %d\n",__FUNCTION__, EntryCount);
#endif
                Mp4Info->Track[TrackSelect].CompositionTime        = (Mp4CompositionOffset_t*)calloc (EntryCount, sizeof(Mp4CompositionOffset_t));
                if (Mp4Info->Track[TrackSelect].CompositionTime == NULL)
                {
                    printf ("%s: ERROR - unable to create Sample table - need %d bytes\n", __FUNCTION__, EntryCount * sizeof(Mp4TimeToSample_t));
                    return -PlayerNoMemory;
                }
                else
                {
                    Mp4Info->Track[TrackSelect].CompositionTimeCount       = EntryCount;
                    myfread(context, Mp4Info->Track[TrackSelect].CompositionTime, sizeof(Mp4CompositionOffset_t), EntryCount, Mp4File);
                    for (i=0; i<EntryCount; i++)            /* convert sample table to little endian */
                    {
                        Mp4Info->Track[TrackSelect].CompositionTime[i].Count  = BE2ME (Mp4Info->Track[TrackSelect].CompositionTime[i].Count);
                        Mp4Info->Track[TrackSelect].CompositionTime[i].Offset = BE2ME (Mp4Info->Track[TrackSelect].CompositionTime[i].Offset);
                    }
                }
                break;
            }
            /*}}}  */
            case CodeToInteger('s','t','s','s'):
            /*{{{  */
            {
                if (myfseeko(context, &Mp4File, sizeof(unsigned int), SEEK_CUR) < 0)                        /* skip version no */
		{
		       printf("seek10 failed\n");
		       goto StartMp4_error;
                }

                myfread(context, &EntryCount, sizeof(unsigned int), 1, Mp4File);
                EntryCount      = BE2ME (EntryCount);

#ifdef DEBUG
                printf("%s: 'stss' Found %d key frames\n",__FUNCTION__, EntryCount);
#endif
                Mp4Info->Track[TrackSelect].KeyFrameTable       = (unsigned int*)calloc (EntryCount, sizeof(unsigned int));
                if (Mp4Info->Track[TrackSelect].KeyFrameTable == NULL)
                {
                    printf ("%s: ERROR - unable to create KeyFrame table - need %d bytes\n", __FUNCTION__, EntryCount * sizeof(unsigned int));
                    return -PlayerNoMemory;
                }
                else
                {
                    Mp4Info->Track[TrackSelect].KeyFrameTableCount      = EntryCount;
                    myfread(context, Mp4Info->Track[TrackSelect].KeyFrameTable, sizeof(unsigned int), EntryCount, Mp4File);
                    for (i=0; i<EntryCount; i++)            /* convert sample table to little endian */
                        Mp4Info->Track[TrackSelect].KeyFrameTable[i]    = BE2ME (Mp4Info->Track[TrackSelect].KeyFrameTable[i]);
                }
                if (myfseeko(context, &Mp4File, BoxEnd, SEEK_SET) < 0)
		{
		       printf("seek11 failed\n");
		       goto StartMp4_error;
                }
                break;
            }
            /*}}}  */
            case CodeToInteger('s','t','c','o'):
            /*{{{  */
            {
                unsigned int*  ChunkOffset;

                if (myfseeko(context, &Mp4File, sizeof(unsigned int), SEEK_CUR) < 0)                        /* skip version no */
		{
		       printf("seek12 failed\n");
		       goto StartMp4_error;
                }

                myfread(context, &EntryCount, sizeof(unsigned int), 1, Mp4File);
                EntryCount          = BE2ME (EntryCount);

                ChunkOffset         = (unsigned int*)malloc (EntryCount  * sizeof(unsigned int));
                if (ChunkOffset == NULL)
                {
                    printf ("%s: ERROR - unable to create Chunk offset list need %d bytes\n", __FUNCTION__, EntryCount  * sizeof(unsigned int));
                    return -PlayerNoMemory;
                }
                else
                {
                    myfread(context, ChunkOffset, sizeof(unsigned int), EntryCount, Mp4File);
                    Mp4Info->Track[TrackSelect].Chunk       = (Mp4Chunk_t*)malloc (sizeof(Mp4Chunk_t) * EntryCount);
                    if (Mp4Info->Track[TrackSelect].Chunk == NULL)
                    {
                        printf ("%s: ERROR - unable to create Chunk list - need %d bytes\n", __FUNCTION__, sizeof(Mp4Chunk_t) * EntryCount);
                        return -PlayerNoMemory;
                    }
                    else
                    {
                        Mp4Info->Track[TrackSelect].ChunkCount = EntryCount;
                        for (i=0; i<EntryCount; i++)
                            Mp4Info->Track[TrackSelect].Chunk[i].Offset    = BE2ME(ChunkOffset[i]);
                    }
                    free (ChunkOffset);
                }
                break;
            }
            /*}}}  */
            case CodeToInteger('c','o','6','4'):
            /*{{{  */
            {
                unsigned long long*  ChunkOffset;

                if (myfseeko(context, &Mp4File, sizeof(unsigned int), SEEK_CUR) < 0)                        /* skip version no */
		{
		       printf("seek13 failed\n");
		       goto StartMp4_error;
                }

                myfread(context, &EntryCount, sizeof(unsigned int), 1, Mp4File);
                EntryCount          = BE2ME (EntryCount);

                ChunkOffset         = (unsigned long long*)malloc (EntryCount  * sizeof(unsigned long long int));
                if (ChunkOffset == NULL)
                {
                    printf ("%s: ERROR - unable to create Chunk offset list need %u bytes\n", __FUNCTION__, EntryCount * sizeof(unsigned long long int));
                    return -PlayerError;
                }
                else
                {
                    myfread(context, ChunkOffset, sizeof(unsigned long long int), EntryCount, Mp4File);
                    Mp4Info->Track[TrackSelect].Chunk       = (Mp4Chunk_t*)malloc (sizeof(Mp4Chunk_t) * EntryCount);
                    if (Mp4Info->Track[TrackSelect].Chunk == NULL)
                    {
                        printf ("%s: ERROR - unable to create Chunk list - need %u bytes\n", __FUNCTION__, sizeof(Mp4Chunk_t) * EntryCount);
                        return -PlayerError;
                    }
                    else
                    {
                        Mp4Info->Track[TrackSelect].ChunkCount = EntryCount;
                        for (i=0; i<EntryCount; i++)
                            Mp4Info->Track[TrackSelect].Chunk[i].Offset    = BE2ME64(ChunkOffset[i]);
                    }
                    free (ChunkOffset);
                }
                break;
            }
            /*}}}  */
            case CodeToInteger('s','t','s','c'):
            /*{{{  */
            {
                if (myfseeko(context, &Mp4File, sizeof(unsigned int), SEEK_CUR) < 0)                        /* skip version no */
		{
		       printf("seek13 failed\n");
		       goto StartMp4_error;
                }

                myfread(context, &EntryCount, sizeof(unsigned int), 1, Mp4File);
                EntryCount          = BE2ME (EntryCount);

                Mp4Info->Track[TrackSelect].ChunkMap       = (Mp4ChunkMap_t*)malloc (sizeof(Mp4ChunkMap_t) * EntryCount);
                if (Mp4Info->Track[TrackSelect].ChunkMap == NULL)
                {
                    printf ("%s: ERROR - unable to create Chunk map - need %d bytes\n", __FUNCTION__, sizeof(Mp4ChunkMap_t) * EntryCount);
                    return -PlayerNoMemory;
                }
                else
                {
                    myfread(context, Mp4Info->Track[TrackSelect].ChunkMap, sizeof(Mp4ChunkMap_t), EntryCount, Mp4File);
                    Mp4Info->Track[TrackSelect].ChunkMapCount  = EntryCount;

                    for (i=0; i<EntryCount; i++)
                    {
                        Mp4Info->Track[TrackSelect].ChunkMap[i].FirstChunk             = BE2ME(Mp4Info->Track[TrackSelect].ChunkMap[i].FirstChunk) - 1;
                        Mp4Info->Track[TrackSelect].ChunkMap[i].SamplesPerChunk        = BE2ME(Mp4Info->Track[TrackSelect].ChunkMap[i].SamplesPerChunk);
                        Mp4Info->Track[TrackSelect].ChunkMap[i].SampleDescriptionIndex = BE2ME(Mp4Info->Track[TrackSelect].ChunkMap[i].SampleDescriptionIndex);
                    }
                }
                break;
            }
            /*}}}  */
            case CodeToInteger('s','t','s','z'):
            /*{{{  */
            {
                unsigned int        SampleSize;

                if (myfseeko(context, &Mp4File, sizeof(unsigned int), SEEK_CUR) < 0)                        /* skip version no */
		{
		       printf("seek14 failed\n");
		       goto StartMp4_error;
                }

                myfread(context, &SampleSize, sizeof(unsigned int), 1, Mp4File);
                myfread(context, &EntryCount, sizeof(unsigned int), 1, Mp4File);
                SampleSize          = BE2ME (SampleSize);
                EntryCount          = BE2ME (EntryCount);

                Mp4Info->Track[TrackSelect].SampleSize     = SampleSize;
                Mp4Info->Track[TrackSelect].SampleCount    = EntryCount;

                if (SampleSize == 0)
                {
                    unsigned int*   SampleSizeList   = malloc (EntryCount * sizeof(unsigned int));
                    if (SampleSizeList == NULL)
                    {
                        printf ("%s: ERROR - unable to create Sample size list - need %d bytes\n", __FUNCTION__, EntryCount * sizeof(unsigned int));
                        return -PlayerNoMemory;
                    }
                    else
                    {
                        Mp4Info->Track[TrackSelect].Samples = (Mp4Sample_t*)calloc (EntryCount, sizeof(Mp4Sample_t));
                        if (Mp4Info->Track[TrackSelect].Samples == NULL)
                        {
                            printf ("%s: ERROR - unable to create Samples list - need %d bytes\n", __FUNCTION__, EntryCount * sizeof(Mp4Sample_t));
                            return -PlayerNoMemory;
                        }
                        else
                        {
                            myfread(context, SampleSizeList, sizeof(unsigned int), EntryCount, Mp4File);
                            for (i=0; i<EntryCount; i++)
                                Mp4Info->Track[TrackSelect].Samples[i].Length       = BE2ME(SampleSizeList[i]);

                        }
                        free (SampleSizeList);
                    }
                }
                break;
            }
            /*}}}  */
            case CodeToInteger('s','t','s','d'):
            /*{{{  */
            {
                /* This contains the Sequence Parameters and Picture Parameters */
                if (myfseeko(context, &Mp4File, sizeof(unsigned int), SEEK_CUR) < 0)                        /* skip blank word */
		{
		       printf("seek15 failed\n");
		       goto StartMp4_error;
                }

                myfread(context, &EntryCount, sizeof(unsigned int), 1, Mp4File);

                EntryCount          = BE2ME (EntryCount);
                for (i = 0; i < EntryCount; i++)
                {
                    unsigned int  EntryLength;
                    unsigned int  RecordType;

                    myfread(context, &EntryLength, sizeof(unsigned int), 1, Mp4File);
                    EntryLength     = BE2ME(EntryLength);
                    if (EntryLength < (sizeof(unsigned int) * 2))
                    {
                        printf ("%s: ERROR - Sample entry too short (%d)\n", __FUNCTION__, EntryLength);
                        break;
                    }
                    else if (i == 0)
                    {
                        myfread(context, &RecordType, sizeof(unsigned int), 1, Mp4File);
                        if ((RecordType == CodeToInteger('a','v','c','1')) && (Mp4Info->Track[TrackSelect].Type == CodeToInteger('v','i','d','e')))
                        {
                            if (myfseeko(context, &Mp4File, sizeof(avc1_t), SEEK_CUR) < 0)                      /* skip avc1 record */
		            {
			       printf("seek16 failed\n");
			       goto StartMp4_error;
                            }
                            myfread(context, &EntryLength, sizeof(unsigned int), 1, Mp4File);
                            myfread(context, &RecordType, sizeof(unsigned int), 1, Mp4File);
                            EntryLength     = BE2ME(EntryLength);

                            if (RecordType == CodeToInteger('a','v','c','C'))
                            {
                                Mp4Info->Track[TrackSelect].SequenceDataLength = EntryLength - (sizeof(unsigned int) * 2);
                                Mp4Info->Track[TrackSelect].SequenceData       = malloc(EntryLength - (sizeof(unsigned int) * 2));
                                if (Mp4Info->Track[TrackSelect].SequenceData == NULL)
                                {
                                    printf ("%s: ERROR - unable to create video sequence data need %d bytes\n", __FUNCTION__, Mp4Info->Track[TrackSelect].SequenceDataLength);
                                    return -PlayerNoMemory;
                                }
                                myfread(context, Mp4Info->Track[TrackSelect].SequenceData, 1, EntryLength - (sizeof(unsigned int) * 2), Mp4File);
                                //Context->Video->Encoding = VIDEO_ENCODING_H264;
                                Track_t Video = {
					"und",
					"V_MPEG4/ISO/AVC",
					TrackSelect,
				};
				context->manager->video->Command(context, MANAGER_ADD, &Video);
                            }
                        }
                        else if ((RecordType == CodeToInteger('m','p','4','a')) && (Mp4Info->Track[TrackSelect].Type == CodeToInteger('s','o','u','n')))
                        {
                            mp4a_t        Mp4aHeader;
                            unsigned int  SampleIndex;
                            unsigned int  TimeScale;
                            unsigned int  Channels;
                            unsigned int  Profile = 1;

                            myfread(context, &Mp4aHeader, sizeof(mp4a_t), 1, Mp4File);

                            TimeScale = (Mp4aHeader.TimeScale[0] << 24) | (Mp4aHeader.TimeScale[1] << 16) |
                                        (Mp4aHeader.TimeScale[2] << 8 ) |  Mp4aHeader.TimeScale[3];
                            for (SampleIndex = 0; SampleIndex < SAMPLE_RATES; SampleIndex++)
                                if (TimeScale == AacSampleRates[SampleIndex])
                                    break;
                            if (SampleIndex == SAMPLE_RATES)
                                printf ("%s: ERROR unrecognised sample rate  %d\n", __FUNCTION__, TimeScale);
                            Channels  = (Mp4aHeader.Channels[0] << 8)   | Mp4aHeader.Channels[1];

                            DefaultAACHeader[2] = ((Profile & 0x03) << 6)  | (SampleIndex << 2) | ((Channels >> 2) & 0x01);
                            /*DefaultAACHeader[2] = (SampleIndex << 2) | ((Channels >> 2) & 0x01);*/
                            DefaultAACHeader[3] = (Channels & 0x03) << 6;

                            Mp4Info->Track[TrackSelect].SequenceDataLength = sizeof (AACHeader_t);
                            Mp4Info->Track[TrackSelect].SequenceData       = malloc(AAC_HEADER_LENGTH);
                            if (Mp4Info->Track[TrackSelect].SequenceData == NULL)
                            {
                                printf ("%s: ERROR - unable to create audio sequence data need %d bytes\n", __FUNCTION__, AAC_HEADER_LENGTH);
                                return -PlayerNoMemory;
                            }
                            memcpy (Mp4Info->Track[TrackSelect].SequenceData, DefaultAACHeader, AAC_HEADER_LENGTH);
                            
/*
82	/// Sign            Length          Position         Description
83	///
84	/// A                12             (31-20)          Sync code
0 0-7
----
1 0-3
85	/// B                 1              (19)            ID
1 4
86	/// C                 2             (18-17)          layer
1 5-6
87	/// D                 1              (16)            protect absent
1 7
----
88	/// E                 2             (15-14)          profile
2 0-1
89	/// F                 4             (13-10)          sample freq index
2 2-5
90	/// G                 1              (9)             private
2 6
91	/// H                 3             (8-6)            channel config
2 7
----
3 0-1
92	/// I                 1              (5)             original/copy
3 2
93	/// J                 1              (4)             home
3 3
94	/// K                 1              (3)             copyright id
3 4
95	/// L                 1              (2)             copyright start
3 5
96	/// M                 13         (1-0,31-21)         frame length
3 6-7
----
4 0-7
----
5 0-2
97	/// N                 11           (20-10)           adts buffer fullness
5 3-7
----
6 0-5
98	/// O                 2             (9-8)            num of raw data blocks in frame
6 6-7
*/                            
                            /*
                            fread (&EntryLength, sizeof(unsigned int), 1, Mp4File);
                            fread (&RecordType,  sizeof(unsigned int), 1, Mp4File);
                            EntryLength     = BE2ME(EntryLength);
                            if (RecordType == CodeToInteger('e','s','d','s'))
                            {
                                fread (Mp4Info->Track[TrackSelect].SequenceData, 1, EntryLength - (sizeof(unsigned int) * 2), Mp4File);
                            }
                            */
                            
                            Track_t Audio = {
									"und",
									"A_AAC",
									TrackSelect,
								};
								context->manager->audio->Command(context, MANAGER_ADD, &Audio);
                        }
                        else  if ((Mp4Info->Track[TrackSelect].Type == CodeToInteger('v','i','d','e')) ||
                                  (Mp4Info->Track[TrackSelect].Type == CodeToInteger('s','o','u','n')))
                        {
                            int            k;
                            unsigned char  Buffer[128];
                            myfread(context, Buffer, 1, EntryLength-8, Mp4File);
                            for (k = 0; k < EntryLength -8; k++)
                            {
                                printf("%02x ", Buffer[k]);
                                if (((k+1)&31)==0)
                                    printf("\n");
                            }
                            printf("\n");
                            for (k = 0; k < EntryLength -8; k++)
                            {
                                printf("%c  ", ((Buffer[k]<32)||(Buffer[k]>127)) ? '.' : Buffer[k]);
                                if (((k+1)&31)==0)
                                    printf("\n");
                            }
                            printf("\n");
                            printf ("%s: Encountered '%.4s' record for '%.4s' track\n", __FUNCTION__, (char*)&RecordType, (char*)&Mp4Info->Track[TrackSelect].Type);
                            //Context->Video->Encoding = VIDEO_ENCODING_MPEG4P2;
                                Track_t Video = {
									"und",
									"V_MS/VFW/FOURCC",
									TrackSelect,
								};
								context->manager->video->Command(context, MANAGER_ADD, &Video);
                        }
                    }
                }
                if (myfseeko(context, &Mp4File, BoxEnd, SEEK_SET) < 0)
		{
		       printf("seek17 failed\n");
		       goto StartMp4_error;
                }
                break;
            }
            /*}}}  */
        }

    }

#ifdef DEBUG
    printf ("%s::%s (%d)\n", FILENAME, __FUNCTION__, __LINE__);
#endif

    if (Mp4Info->VideoTrack != -1)
        BuildTrackMap (&Mp4Info->Track[Mp4Info->VideoTrack]);
    if (Mp4Info->AudioTrack != -1)
        BuildTrackMap (&Mp4Info->Track[Mp4Info->AudioTrack]);

    /* free all non video/audio data */
    for (i = 0; i < TrackCount; i++)
        if ((i != Mp4Info->VideoTrack) && (i != Mp4Info->AudioTrack))
            FreeTrackData (&Mp4Info->Track[i]);

    /*Context->Audio->Encoding = AUDIO_ENCODING_AAC;//MP3;

    if (Context->Video->Required)
        Status    = VideoIoctl (Context, VIDEO_SET_ENCODING,  (void*)Context->Video->Encoding);

    if ((Status == PlayerNoError) && (Context->Audio->Required))
        Status    = AudioIoctl (Context, AUDIO_SET_ENCODING,  (void*)Context->Audio->Encoding);
	*/
#ifdef DEBUG
    printf("%s <\n", __func__);
#endif
    return Status;

StartMp4_error:
#ifdef DEBUG
    printf("StartMp4_error <\n");
#endif
    stream_off=0;

    /* free all non video/audio data */
    for (i = 0; i < TrackCount; i++)
        if ((i != Mp4Info->VideoTrack) && (i != Mp4Info->AudioTrack))
            FreeTrackData (&Mp4Info->Track[i]);

    if (Mp4Info != NULL)
       free(Mp4Info);
    Mp4Info = NULL;

    return -PlayerError;

}
/*}}}  */

static void Mp4Thread(Context_t *context) {
    
#ifdef DEBUG
    printf("%s::%s>\n", FILENAME, __FUNCTION__);
#endif
    
    PlayH264Mp4File (context);
    
    context->playback->Command(context, PLAYBACK_TERM, NULL);

/* fixme: no more data to free here? */
    stream_off=0;

#ifdef DEBUG
    printf("%s::%s<\n", FILENAME, __FUNCTION__);
#endif
}


/*{{{  PlayMp4*/
static int PlayMp4 (Context_t* context)
{
#ifdef DEBUG
    printf("%s::%s>\n", FILENAME, __FUNCTION__);
#endif
    if (isPlayThreadCreated == 0) {
	  pthread_attr_t attr;
	  pthread_attr_init(&attr);
	  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
          if(pthread_create(&PlayThread, &attr, (void *)&Mp4Thread, context) != 0)
          {
            fprintf(stderr, "Error creating thread in %s error:%d:%s\n", __FUNCTION__,errno,strerror(errno));
	    isPlayThreadCreated = 0;
            return -1;
          } else
	    isPlayThreadCreated = 1;
    }
#ifdef DEBUG
    printf("%s::%s<\n", FILENAME, __FUNCTION__);
#endif
    return 0;
}
/*}}}  */

/*{{{  StopMp4*/
static void StopMp4 (Context_t* context) {
    int i;
#ifdef DEBUG
    printf("%s::%s>\n", FILENAME, __FUNCTION__);
#endif

    int result=0;
    if(isPlayThreadCreated != 0)
       result = pthread_join (PlayThread, NULL);
    if(result != 0) 
    {
          printf("ERROR: Stop PlayThread\n");
    }

    isPlayThreadCreated = 0;

    usleep(100000);
    
    if (Mp4Info != NULL)
    {
#ifdef old
        if (Mp4Info->AudioTrack != -1)
            FreeTrackData (&Mp4Info->Track[Mp4Info->AudioTrack]);
        if (Mp4Info->VideoTrack != -1)
            FreeTrackData (&Mp4Info->Track[Mp4Info->VideoTrack]);
#else
	/* free all non video/audio data */
	for (i = 0; i < TrackCount; i++)
            if ((i != Mp4Info->VideoTrack) && (i != Mp4Info->AudioTrack))
        	FreeTrackData (&Mp4Info->Track[i]);
#endif
        free (Mp4Info);
        Mp4Info       = NULL;
    }
    
    stream_off=0;

#ifdef DEBUG
    printf("%s::%s<\n", FILENAME, __FUNCTION__);
#endif
}
/*}}}  */
/*{{{  SeekMp4*/
static PlayerStatus_t SeekMp4 (Context_t* context, bool Forward)
{
    char * Encoding = NULL;

#ifdef DEBUG
    printf("%s::%s>\n", FILENAME, __FUNCTION__);
#endif

    context->manager->video->Command(context, MANAGER_GETENCODING, &Encoding);

    //if(!strcmp (Encoding, "V_MPEG4/ISO/AVC"))
    //    return SeekH264Mp4 (context, Forward ? 30000 : -30000);
    free(Encoding);
    printf ("%s: Mp4 file skip not yest implemented.\n", __FUNCTION__);

#ifdef DEBUG
    printf("%s::%s<\n", FILENAME, __FUNCTION__);
#endif
    return PlayerNoError;
}
/*}}}  */

void seek_mp4(Context_t  *Context,float rel_seek_secs,float audio_delay,int flags){
	Mp4Track_t*         VideoTrack              = NULL;
	unsigned long long  targetPts                = 0ull;
	unsigned long long  VideoPts                = 0ull;
	unsigned int        SampleSize              = 0;
	int                 VideoChunk              = 0;
	int                 VideoSample             = 0;

	Mp4Track_t*         AudioTrack              = NULL;
	//Context_t*    Context                 = (Context_t*)ThreadParam;
	// make shure the playthread has enuogh time to go in pause mode
	// TODO: use a mutex
	
	pthread_mutex_lock(&mutex);
	unsigned long long  AudioPts                = 0ull;
	int                 AudioChunk              = 0;
	int                 AudioSample             = 0;

#ifdef DEBUG
    printf("%s::%s>\n", FILENAME, __FUNCTION__);
    printf("SeekMP4: %f sec\n",rel_seek_secs);
#endif

	if (Mp4Info->VideoTrack != -1)
	{
		VideoTrack                      = &Mp4Info->Track[Mp4Info->VideoTrack];
		printf("SeekMP4: pos: %d \n",VideoTrack->Samples[VideoTrack->SampleToPlay].Pts);
		
		printf("SamplesToPlay ->%d max %d\n", VideoTrack->SampleToPlay, VideoTrack->SampleCount);
		
		targetPts = VideoTrack->Samples[VideoTrack->SampleToPlay].Pts + (rel_seek_secs*90000);
		
		printf("SeekMP4: target: %d \n", (int) targetPts);
		
		VideoTrack->ChunkToPlay         = 0;
		VideoTrack->SampleToPlay        = 0;
		VideoChunk                      = 1;    /* Force to be different from ChunkToPlay so reload pts */
		VideoTrack->Pts                 = 0ull;
	}

	if ((Mp4Info->AudioTrack != -1) && (AudioTrack == NULL))
	{
		// printf ("%s: Initialising AudioTrack\n", __FUNCTION__);
		AudioTrack                      = &Mp4Info->Track[Mp4Info->AudioTrack];
		AudioTrack->ChunkToPlay         = 0;
		AudioTrack->SampleToPlay        = 0;
		AudioChunk                      = 1;    /* Force to be different from ChunkToPlay so reload pts */
		//AudioTrack->Pts                 = FindHeader (Context);
	}

	VideoChunk          = VideoTrack->ChunkToPlay;
	AudioChunk          = AudioTrack->ChunkToPlay;
	
	if (Context->playback->isVideo) {
		while(VideoPts<targetPts)
		if (VideoChunk < VideoTrack->ChunkCount) {
			for (VideoSample=VideoTrack->SampleToPlay; VideoSample<(VideoTrack->Chunk[VideoChunk].FirstSample+VideoTrack->Chunk[VideoChunk].SampleCount); VideoSample++) {
				SampleSize = VideoTrack->Samples[VideoSample].Length;
				VideoPts = VideoTrack->Samples[VideoSample].Pts;
			}
			VideoTrack->SampleToPlay    = VideoSample;
			VideoChunk++;
			VideoTrack->ChunkToPlay     = VideoChunk;		
		}
		else 
		    break;
		
		printf("videoPTS: %d ", (int) VideoPts);
		printf("chunk:%d ",VideoChunk);
		printf("sample:%d\n",VideoSample);
	}
	if (Context->playback->isAudio) {
		while(AudioPts<targetPts)
		if (AudioChunk < AudioTrack->ChunkCount) {
			for (AudioSample=AudioTrack->SampleToPlay; AudioSample<(AudioTrack->Chunk[AudioChunk].FirstSample+AudioTrack->Chunk[AudioChunk].SampleCount); AudioSample++) {
				SampleSize = AudioTrack->Samples[AudioSample].Length;
				AudioPts = AudioTrack->Samples[AudioSample].Pts;
			}
			AudioTrack->SampleToPlay    = AudioSample;
			AudioChunk++;
			AudioTrack->ChunkToPlay     = AudioChunk;
		}else break;
		printf("audioPTS: %d ", (int) AudioPts);
		printf("chunk:%d ",AudioChunk);
		printf("sample:%d\n",AudioSample);
	}
	pthread_mutex_unlock(&mutex);
#ifdef DEBUG
    printf("%s::%s<\n", FILENAME, __FUNCTION__);
#endif
}

static double Mp4GetLength() {

#ifdef DEBUG
    printf("%s::%s<>\n", FILENAME, __FUNCTION__);
#endif
    if (Mp4Info->VideoTrack != -1)
        return Mp4Info->Track[Mp4Info->VideoTrack].Duration/Mp4Info->Track[Mp4Info->VideoTrack].TimeScale;
    else if (Mp4Info->AudioTrack != -1)
        return Mp4Info->Track[Mp4Info->AudioTrack].Duration/Mp4Info->Track[Mp4Info->AudioTrack].TimeScale;
    return 0;
}

static int Command(void* _context, ContainerCmd_t command, void * argument) {
   Context_t  *context = (Context_t*) _context;
   
#ifdef DEBUG
    printf("%s::%s>\n", FILENAME, __FUNCTION__);
#endif

	switch(command) {
		case CONTAINER_INIT: {
			//char * FILENAME = (char *)argument;
			return StartMp4(context);
			break;
		}
		case CONTAINER_PLAY: {
			return PlayMp4(context);
			break;
		}
		case CONTAINER_STOP: {
			StopMp4(context);
			break;
		}
		case CONTAINER_SEEK: {
			seek_mp4 (context, (float)*((float*)argument), 0.0, 0);
			break;
		}
		case CONTAINER_LENGTH: {
					double length = Mp4GetLength();
					*((double*)argument) = (double)length;
					break;
				}
		default:
			printf("ConatinerCmd not supported!");
			break;
	}

#ifdef DEBUG
    printf("%s::%s<\n", FILENAME, __FUNCTION__);
#endif
	return 0;
}

static char *Mp4Capabilities[] = { "mp4", "mp4a", NULL };

Container_t Mp4Container = {
	"MP4",
	&Command,
	Mp4Capabilities,

};
