#ifndef _DEMUXER_H
#define _DEMUXER_H

#include <stdlib.h>
#include <inttypes.h>

#include "stream.h"
#include "aviheader.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MP_INPUT_BUFFER_PADDING_SIZE 8

#define HAVE_BUILTIN_EXPECT

#ifdef HAVE_BUILTIN_EXPECT

#define likely(x) __builtin_expect ((x) != 0, 1)
#define unlikely(x) __builtin_expect ((x) != 0, 0)

#else

#define likely(x) (x)
#define unlikely(x) (x)

#endif

#define FFMAX(a,b) ((a) > (b) ? (a) : (b))
#define FFMIN(a,b) ((a) > (b) ? (b) : (a))

#define MAX_PACKS 4096
#define MAX_PACK_BYTES 0x800000

#define DEMUXER_TYPE_PLAYLIST (2<<16)

#define DEMUXER_TYPE_UNKNOWN 0
#define DEMUXER_TYPE_MPEG_ES 1
#define DEMUXER_TYPE_MPEG_PS 2
#define DEMUXER_TYPE_AVI 3
#define DEMUXER_TYPE_AVI_NI 4
#define DEMUXER_TYPE_AVI_NINI 5
#define DEMUXER_TYPE_ASF 6
#define DEMUXER_TYPE_AUDIO 17
#define DEMUXER_TYPE_MPEG4_ES 27
#define DEMUXER_TYPE_MPEG_TS 29
#define DEMUXER_TYPE_H264_ES 30
#define DEMUXER_TYPE_MATROSKA 31
#define DEMUXER_TYPE_MPEG_PES 41

// DEMUXER control commands/answers
#define DEMUXER_CTRL_NOTIMPL -1
#define DEMUXER_CTRL_DONTKNOW 0
#define DEMUXER_CTRL_OK 1
#define DEMUXER_CTRL_GUESS 2
#define DEMUXER_CTRL_GET_TIME_LENGTH 10
#define DEMUXER_CTRL_GET_PERCENT_POS 11
#define DEMUXER_CTRL_SWITCH_AUDIO 12
#define DEMUXER_CTRL_RESYNC 13
#define DEMUXER_CTRL_SWITCH_VIDEO 14
#define DEMUXER_CTRL_IDENTIFY_PROGRAM 15

#define SEEK_ABSOLUTE (1 << 0)
#define SEEK_FACTOR   (1 << 1)

#define MP_NOPTS_VALUE (-1LL<<63) //both int64_t and double should be able to represent this exactly
// Holds one packet/frame/whatever
typedef struct demux_packet_st {
  int len;
  double pts;
  double endpts;
  double stream_pts;
  off_t pos;  // position in index (AVI) or file (MPG)
  unsigned char* buffer;
  int flags; // keyframe, etc
  int refcount;   //refcounter for the master packet, if 0, buffer can be free()d
  struct demux_packet_st* master; //pointer to the master packet if this one is a cloned one
  struct demux_packet_st* next;
} demux_packet_t;

typedef struct {
  int buffer_pos;          // current buffer position
  int buffer_size;         // current buffer size
  unsigned char* buffer;   // current buffer, never free() it, always use free_demux_packet(buffer_ref);
  double pts;              // current buffer's pts
  int pts_bytes;           // number of bytes read after last pts stamp
  int eof;                 // end of demuxed stream? (true if all buffer empty)
  off_t pos;                 // position in the input stream (file)
  off_t dpos;                // position in the demuxed stream
  int pack_no;		   // serial number of packet
  int flags;               // flags of current packet (keyframe etc)
//---------------
  int packs;              // number of packets in buffer
  int bytes;              // total bytes of packets in buffer
  demux_packet_t *first;  // read to current buffer from here
  demux_packet_t *last;   // append new packets from input stream to here
  demux_packet_t *current;// needed for refcounting of the buffer
  int id;                 // stream ID  (for multiple audio/video streams)
  struct demuxer_st *demuxer; // parent demuxer structure (stream handler)
// ---- asf -----
  demux_packet_t *asf_packet;  // read asf fragments here
  int asf_seq;
// ---- mov -----
  unsigned int ss_mul,ss_div;
// ---- stream header ----
  void* sh;
} demux_stream_t;
#define MAX_A_STREAMS 256
#define MAX_V_STREAMS 256
#define MAX_S_STREAMS 32

struct demuxer_st;

extern int correct_pts;

/**
 * Demuxer description structure
 */
typedef struct demuxers_desc_st {
  const char *info; ///< What is it (long name and/or description)
  const char *name; ///< Demuxer name, used with -demuxer switch
  const char *shortdesc; ///< Description printed at demuxer detection
  const char *author; ///< Demuxer author(s)
  const char *comment; ///< Comment, printed with -demuxer help

  int type; ///< DEMUXER_TYPE_xxx
  int safe_check; ///< If 1 detection is safe and fast, do it before file extension check

  /// Check if can demux the file, return DEMUXER_TYPE_xxx on success
  int (*check_file)(struct demuxer_st *demuxer); ///< Mandatory if safe_check == 1, else optional 
  /// Get packets from file, return 0 on eof
  int (*fill_buffer)(struct demuxer_st *demuxer, demux_stream_t *ds); ///< Mandatory
  /// Open the demuxer, return demuxer on success, NULL on failure
  struct demuxer_st* (*open)(struct demuxer_st *demuxer); ///< Optional
  /// Close the demuxer
  void (*close)(struct demuxer_st *demuxer); ///< Optional
  // Seek
  void (*seek)(struct demuxer_st *demuxer, float rel_seek_secs, float audio_delay, int flags); ///< Optional
  // Control
  int (*control)(struct demuxer_st *demuxer, int cmd, void *arg); ///< Optional
} demuxer_desc_t;

typedef struct demux_chapter_s
{
  uint64_t start, end;
  char* name;
} demux_chapter_t;

typedef struct demuxer_st {
  demuxer_desc_t *desc;  ///< Demuxer description structure
  off_t filepos; // input stream current pos.
  off_t movi_start;
  off_t movi_end;
  stream_t *stream;
  double stream_pts;       // current stream pts, if applicable (e.g. dvd)
  char *filename; ///< Needed by avs_check_file
  int synced;  // stream synced (used by mpeg)
  int type;    // demuxer type: mpeg PS, mpeg ES, avi, avi-ni, avi-nini, asf
  int file_format;  // file format: mpeg/avi/asf
  int seekable;  // flag
  //
  demux_stream_t *audio; // audio buffer/demuxer
  demux_stream_t *video; // video buffer/demuxer
  demux_stream_t *sub;   // dvd subtitle buffer/demuxer

  // stream headers:
  void* a_streams[MAX_A_STREAMS]; // audio streams (sh_audio_t)
  void* v_streams[MAX_V_STREAMS]; // video sterams (sh_video_t)
  void *s_streams[MAX_S_STREAMS];   // dvd subtitles (flag)

  demux_chapter_t* chapters;
  int num_chapters;
  
  void* priv;  // fileformat-dependent data
  char** info;

} demuxer_t;

typedef struct {
	int progid;        //program id
	int aid, vid, sid; //audio, video and subtitle id
} demux_program_t;

static inline demux_packet_t* new_demux_packet(int len){
  demux_packet_t* dp=(demux_packet_t*)malloc(sizeof(demux_packet_t));
  dp->len=len;
  dp->next=NULL;
  // still using 0 by default in case there is some code that uses 0 for both
  // unknown and a valid pts value
  dp->pts=correct_pts ? MP_NOPTS_VALUE : 0;
  dp->endpts=MP_NOPTS_VALUE;
  dp->stream_pts = MP_NOPTS_VALUE;
  dp->pos=0;
  dp->flags=0;
  dp->refcount=1;
  dp->master=NULL;
  dp->buffer=NULL;
  if (len > 0 && (dp->buffer = (unsigned char *)malloc(len + 8)))
    memset(dp->buffer + len, 0, 8);
  else
    dp->len = 0;
  return dp;
}

static inline void resize_demux_packet(demux_packet_t* dp, int len)
{
  if(len > 0)
  {
     dp->buffer=(unsigned char *)realloc(dp->buffer,len+8);
  }
  else
  {
     free(dp->buffer);
     dp->buffer = NULL;
  }
  dp->len=len;
  if (dp->buffer)
     memset(dp->buffer + len, 0, 8);
  else
     dp->len = 0;
}

static inline void free_demux_packet(demux_packet_t* dp){
	if (dp != NULL) {
		if (dp->master==NULL) {  //dp is a master packet
		      dp->refcount--;
		      if (dp->refcount==0) {
				free(dp->buffer);
				dp->buffer = NULL;
				free(dp);
				dp = NULL;
		      }
		} else { // dp is a clone
			free_demux_packet(dp->master);
			free(dp);
			dp = NULL;
		}
	}
}

static inline int avi_stream_id(unsigned int id){
  unsigned char *p=(unsigned char *)&id;
  unsigned char a,b;
#if WORDS_BIGENDIAN
  a=p[3]-'0'; b=p[2]-'0';
#else
  a=p[0]-'0'; b=p[1]-'0';
#endif
  if(a>9 || b>9) return 100; // invalid ID
  return a*10+b;
}
#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#endif

static inline void *realloc_struct(void *ptr, size_t nmemb, size_t size) {
  if (nmemb > SIZE_MAX / size) {
    free(ptr);
    return NULL;
  }
  return realloc(ptr, nmemb * size);
}

extern char *dvdsub_lang;
extern char *audio_lang;
extern int dvdsub_id;

static inline int ds_tell_pts(demux_stream_t *ds){
  return (ds->pts_bytes-ds->buffer_size)+ds->buffer_pos;
}

#if 1
#define demux_getc(ds) (\
     (likely(ds->buffer_pos<ds->buffer_size)) ? ds->buffer[ds->buffer_pos++] \
     :((unlikely(!ds_fill_buffer(ds)))? (-1) : ds->buffer[ds->buffer_pos++] ) )
#else
static inline int demux_getc(demux_stream_t *ds){
  if(ds->buffer_pos>=ds->buffer_size){
    if(!ds_fill_buffer(ds)){
//      printf("DEMUX_GETC: EOF reached!\n");
      return -1; // EOF
    }
  }
//  printf("[%02X]",ds->buffer[ds->buffer_pos]);
  return ds->buffer[ds->buffer_pos++];
}
#endif

void free_sh_audio(demuxer_t *demuxer, int id);
void ds_add_packet(demux_stream_t *ds,demux_packet_t* dp);
int demux_fill_buffer(demuxer_t *demux,demux_stream_t *ds);
int ds_fill_buffer(demux_stream_t *ds);
int ds_fill_buffer2(demux_stream_t *ds,demux_stream_t *demuxer);//for avi and mpg
int demux_read_data(demux_stream_t *ds,unsigned char* mem,int len);
int demuxer_add_chapter(demuxer_t* demuxer, const char* name, uint64_t start, uint64_t end);
int demuxer_sub_track_by_lang(demuxer_t *d, char *lang);
void ds_read_packet(demux_stream_t *ds, stream_t *stream, int len, double pts, off_t pos, int flags);
void ds_free_packs(demux_stream_t *ds);
int demux_info_add(demuxer_t *demuxer, const char *opt, const char *param);
void demux_flush(demuxer_t *demuxer);

#endif //DEMUXER_H
