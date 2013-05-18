#ifndef MP4_H_
#define MP4_H_

#include <stdio.h>/*
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>*/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __cplusplus
#ifndef bool
#define bool    unsigned char
#define false   0
#define true    1
#endif
#endif


#define DEBUG_LEVEL                     0
#define dprintf(x, fmt, args...)        if (DEBUG_LEVEL) printf(fmt, ## args)

#define PlayerStatus_t int
#define PlayerNoError 0
#define PlayerError 1
#define PlayerNoMemory 2

#define CodeToInteger(a,b,c,d)                  ((a << 0) | (b << 8) | (c << 16) | (d <<24))


/*{{{  defines*/
#define BE2ME16(x)                      (((x & 0x00ff) << 8) | (x >> 8))
#define BE2ME(x)                        (((x & 0xff) << 24) | ((x & 0xff00) << 8) | ((x >> 8) & 0xff00) | ((x>>24) & 0xff))
#define BE2ME64(x)                      ( ((unsigned long long)(x & 0xff) << 56)                    |       \
                                          ((unsigned long long)(x & 0xff00) << 40)                  |       \
                                          ((unsigned long long)(x & 0xff0000) << 24)                |       \
                                          ((unsigned long long)(x & 0xff000000) << 8)               |       \
                                          ((unsigned long long)(x & 0xff00000000ULL) >> 8)          |       \
                                          ((unsigned long long)(x & 0xff0000000000ULL) >> 24)       |       \
                                          ((unsigned long long)(x & 0xff000000000000ULL) >> 40)     |       \
                                          ((unsigned long long)(x & 0xff00000000000000ULL) >> 56) )


#define MP4_KEY_FRAME                   0x01

/*}}}  */
/*{{{  typedefs*/
typedef struct Mp4Chunk_s
{
    unsigned int                FirstSample;
    unsigned int                SampleCount;
    unsigned int                Description;
    off_t                       Offset;
} Mp4Chunk_t;

typedef struct Mp4ChunkMap_s
{
    unsigned int                FirstChunk;
    unsigned int                SamplesPerChunk;
    unsigned int                SampleDescriptionIndex;
} Mp4ChunkMap_t;

typedef struct Mp4TimeToSample_s
{
    unsigned int                Count;
    unsigned int                Delta;
} Mp4TimeToSample_t;

typedef struct Mp4CompositionOffset_s
{
    unsigned int                Count;
    unsigned int                Offset;
} Mp4CompositionOffset_t;

typedef struct Mp4Sample_s
{
    unsigned int                Pts;
    unsigned int                Flags;
    unsigned int                Length;
    off_t                       Offset;
} Mp4Sample_t;

typedef struct Mp4Track_s
{
    unsigned int                Type;

    unsigned int                SampleSize;
    unsigned int                TimeScale;
    unsigned long long          Duration;
  int width,height;

    unsigned int                SampleCount;
    Mp4Sample_t*                Samples;
    bool                        DecodingTimeValues;

    unsigned int                ChunkCount;
    Mp4Chunk_t*                 Chunk;
    unsigned int                ChunkToPlay;
    unsigned int                SampleToPlay;
    unsigned long long          Pts;

    unsigned int                SequenceDataLength;
    unsigned char*              SequenceData;

    /* Items read from file then discarded before play */
    unsigned int                ChunkMapCount;
    Mp4ChunkMap_t*              ChunkMap;
    unsigned int                DecodingTimeCount;
    Mp4TimeToSample_t*          DecodingTime;
    unsigned int                CompositionTimeCount;
    Mp4CompositionOffset_t*     CompositionTime;
    unsigned int                KeyFrameTableCount;
    unsigned int*               KeyFrameTable;

    unsigned int                TimeDelta;              // Added by nick extracted from the data above that is discarded
} Mp4Track_t;

typedef struct Mp4Info_s
{
    Mp4Track_t                  Track[10];
    unsigned int                VideoTrack;
    unsigned int                AudioTrack;
} Mp4Info_t;

typedef enum Container_e
{
    File                        = CodeToInteger('f','i','l','e'),
    MovieBox                    = CodeToInteger('m','o','o','v'),
    TrackBox                    = CodeToInteger('t','r','a','k'),
    MediaBox                    = CodeToInteger('m','d','i','a'),
    MediaInformationBox         = CodeToInteger('m','i','n','f'),
    SampleTableBox              = CodeToInteger('s','t','b','l'),
} Mp4Container_t;

typedef struct Code_s
{
    unsigned int                Code;
    Mp4Container_t                 Parent;
    Mp4Container_t                 Next;
    bool                        Skip;
} Code_t;

typedef struct avc1_s
{
    unsigned char       Reserved1[6];
    unsigned char       RefenceIndex[2];        /* BE short dataReferenceIndex */
    unsigned char       Reserved2[16];
    unsigned char       Width[2];               /* BE short Width              */
    unsigned char       Height[2];              /* BE short Height             */
    unsigned char       Reserved3[14];
    unsigned char       CompressorName[32];     /* Should contain coding name  */
    unsigned char       Reserved4[4];
}avc1_t;

typedef avc1_t          mp4v_t;

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


typedef struct mp4a_s
{
    unsigned char       Reserved1[6];
    unsigned char       RefenceIndex[2];        /* BE short dataReferenceIndex */
    unsigned char       Version[2];             /* BE short soundVersion       */
    unsigned char       Reserved2[6];
    unsigned char       Channels[2];            /* BE short channels           */
    unsigned char       SampleSize[2];          /* BE short sampleSize         */
    unsigned char       PacketSize[2];          /* BE short packetSize         */
    unsigned char       TimeScale[4];           /* BE long timeScale           */
    unsigned char       Reserved3[2];
} mp4a_t;


#define MP4ESDescrTag                   0x03
typedef struct esds_s
{
    unsigned char       Version;                /* configurationVersion        */
    unsigned char       Flags[3];               /* flags                       */
    unsigned char       Tag;                    /* Descriptor tag              */
    unsigned char       Length;                 /* Descriptor length           */
    unsigned char       Descriptor[1];          /* rest of descriptor          */
} esds_t;


#define AAC_HEADER_LENGTH       7
typedef struct AACHeader_s
{
    unsigned char       Data[AAC_HEADER_LENGTH];
}AACHeader_t;
/*}}}  */
/*{{{  statics*/
#define RECOGNISED_CODES        29
static const Code_t   CodeTable[RECOGNISED_CODES] =
{
    { CodeToInteger('f','t','y','p'),           File,                   File,                   true},
    { CodeToInteger('m','d','a','t'),           File,                   File,                   true},
    { CodeToInteger('f','r','e','e'),           File,                   File,                   true},
    { CodeToInteger('s','k','i','p'),           File,                   File,                   true},
    { CodeToInteger('j','u','n','k'),           File,                   File,                   true},
    { CodeToInteger('m','o','o','v'),           File,                   MovieBox,               false},
    { CodeToInteger('m','v','h','d'),           MovieBox,               MovieBox,               true},
    { CodeToInteger('t','r','a','k'),           MovieBox,               TrackBox,               false},
    { CodeToInteger('t','k','h','d'),           TrackBox,               TrackBox,               true},
    { CodeToInteger('e','d','t','s'),           TrackBox,               TrackBox,               true},
    { CodeToInteger('m','d','i','a'),           TrackBox,               MediaBox,               false},
    { CodeToInteger('m','d','h','d'),           MediaBox,               MediaBox,               false},
    { CodeToInteger('h','d','l','r'),           MediaBox,               MediaBox,               false},
    { CodeToInteger('m','i','n','f'),           MediaBox,               MediaInformationBox,    false},
    { CodeToInteger('v','m','h','d'),           MediaInformationBox,    MediaInformationBox,    true},
    { CodeToInteger('s','m','h','d'),           MediaInformationBox,    MediaInformationBox,    true},
    { CodeToInteger('h','m','h','d'),           MediaInformationBox,    MediaInformationBox,    true},
    { CodeToInteger('n','m','h','d'),           MediaInformationBox,    MediaInformationBox,    true},
    { CodeToInteger('d','i','n','f'),           MediaInformationBox,    MediaInformationBox,    true},
    { CodeToInteger('s','t','b','l'),           MediaInformationBox,    SampleTableBox,         false},
    { CodeToInteger('s','t','s','s'),           SampleTableBox,         SampleTableBox,         false},
    { CodeToInteger('s','t','t','s'),           SampleTableBox,         SampleTableBox,         false},
    { CodeToInteger('s','t','c','o'),           SampleTableBox,         SampleTableBox,         false},
    { CodeToInteger('c','o','6','4'),           SampleTableBox,         SampleTableBox,         false},
    { CodeToInteger('s','t','s','c'),           SampleTableBox,         SampleTableBox,         false},
    { CodeToInteger('s','t','s','z'),           SampleTableBox,         SampleTableBox,         false},
    { CodeToInteger('s','t','s','d'),           SampleTableBox,         SampleTableBox,         false},
    { CodeToInteger('c','t','t','s'),           SampleTableBox,         SampleTableBox,         false},
    { CodeToInteger('u','d','t','a'),           SampleTableBox,         SampleTableBox,         true},
};

#define SAMPLE_RATES            16
static const int AacSampleRates[SAMPLE_RATES] =
{
    96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050,
    16000, 12000, 11025,  8000,  7350,     0,     0,     0
};

static unsigned char DefaultAACHeader[]    =  { 0xff, 0xf1, 0x00, 0x00, 0x00, 0x1f, 0xfc };
    /*
    0xffffff           12 bits          Sync word
    IsMpeg2             1               Mpeg2?
    0x00                2               Layer
    0x01                1               Protection absent
    Profile             2               Profile
    Frequency           4               Sampling Frequency
    0x00                1               Private
    Channels            3               Channel configuration
    0x00                1               Original
    0x00                1               Home
    0x00                1               Copyright Id
    0x00                1               Copyright Id start
    DataLength         13               AAC Frame Length
    0x7FF              11               ADTS buffer fullness
    DataBlocks          2               No. raw data blocks
    */

/*
82	/// Sign            Length          Position         Description
83	///
84	/// A                12             (31-20)          Sync code
85	/// B                 1              (19)            ID
86	/// C                 2             (18-17)          layer
87	/// D                 1              (16)            protect absent
88	/// E                 2             (15-14)          profile
89	/// F                 4             (13-10)          sample freq index
90	/// G                 1              (9)             private
91	/// H                 3             (8-6)            channel config
92	/// I                 1              (5)             original/copy
93	/// J                 1              (4)             home
94	/// K                 1              (3)             copyright id
95	/// L                 1              (2)             copyright start
96	/// M                 13         (1-0,31-21)         frame length
97	/// N                 11           (20-10)           adts buffer fullness
98	/// O                 2             (9-8)            num of raw data blocks in frame
*/     

/*}}}  */

static bool CheckCode (unsigned int Code, Mp4Container_t* Container);


#endif
