#ifndef STREAM_H
#define STREAM_H

#include <string.h>
#include <inttypes.h>
#include <sys/types.h>

#define STREAMTYPE_FILE 0
#define STREAMTYPE_VCD  1      // raw mode-2 CDROM reading, 2324 bytes/sector
#define STREAMTYPE_STREAM 2    // same as FILE but no seeking (for net/stdin)
#define STREAMTYPE_DS   8      // read from a demuxer stream

#define STREAM_READ  0
#define STREAM_WRITE 1

#define STREAM_SEEK_BW  2
#define STREAM_SEEK_FW  4
#define STREAM_SEEK  (STREAM_SEEK_BW|STREAM_SEEK_FW)

#define MAX_STREAM_PROTOCOLS 10

#define STREAM_BUFFER_SIZE 2048
#define VCD_SECTOR_SIZE 2352

#define STREAM_UNSUPPORTED -1
#define STREAM_CTRL_RESET 0
#define STREAM_CTRL_GET_TIME_LENGTH 1
#define STREAM_CTRL_GET_CURRENT_TIME 5

#ifdef DEBUG
#define dprintf(x...) do { printf(x); } while (0)
#endif

#define mmioFOURCC( ch0, ch1, ch2, ch3 )				\
		( (uint32_t)(uint8_t)(ch0) | ( (uint32_t)(uint8_t)(ch1) << 8 ) |	\
		( (uint32_t)(uint8_t)(ch2) << 16 ) | ( (uint32_t)(uint8_t)(ch3) << 24 ) )

struct stream_st;
typedef struct stream_info_st {
  const char *info;
  const char *name;
  const char *author;
  const char *comment;
  /// mode isn't used atm (ie always READ) but it shouldn't be ignored
  /// opts is at least in it's defaults settings and may have been
  /// altered by url parsing if enabled and the options string parsing.
  //int (*open)(struct stream_st* st, int mode, void* opts, int* file_format);
  char* protocols[MAX_STREAM_PROTOCOLS];
  void* opts;
  int opts_url; /* If this is 1 we will parse the url as an option string
		 * too. Otherwise options are only parsed from the
		 * options string given to open_stream_plugin */
} stream_info_t;

typedef struct stream_st {
  /*// Read
  int (*fill_buffer)(struct stream_st *s, char* buffer, int max_len);
  // Write
  int (*write_buffer)(struct stream_st *s, char* buffer, int len);
  // Seek
  int (*seek)(struct stream_st *s,off_t pos);
  // Control
  // Will be later used to let streams like dvd and cdda report
  // their structure (ie tracks, chapters, etc)
  int (*control)(struct stream_st *s,int cmd,void* arg);
  // Close
  void (*close)(struct stream_st *s);*/
  int (*control)(struct stream_st *s,int cmd,void* arg);

  int fd;   // file descriptor, see man open(2)
  FILE* file;
  int type; // see STREAMTYPE_*
  int flags;
  int sector_size; // sector size (seek will be aligned on this size if non 0)
  unsigned int buf_pos,buf_len;
  off_t pos,start_pos,end_pos;
  int eof;
  int mode; //STREAM_READ or STREAM_WRITE
  unsigned int cache_pid;
  void* cache_data;
  void* priv; // used for DVD, TV, RTSP etc
  char* url;  // strdup() of filename/url

  unsigned char buffer[STREAM_BUFFER_SIZE];
} stream_t;

int stream_fill_buffer(stream_t *s);
int stream_seek_long(stream_t *s,off_t pos);
#define cache_stream_fill_buffer(x) stream_fill_buffer(x)
#define cache_stream_seek_long(x,y) stream_seek_long(x,y)
#define stream_enable_cache(x,y,z,w) 1

inline static int stream_read_char(stream_t *s){
  return (s->buf_pos<s->buf_len)?s->buffer[s->buf_pos++]:
    (cache_stream_fill_buffer(s)?s->buffer[s->buf_pos++]:-256);
}
inline static unsigned int stream_read_word(stream_t *s){
  int x,y;
  x=stream_read_char(s);
  y=stream_read_char(s);
  return (x<<8)|y;
}

inline static int stream_read(stream_t *s,unsigned char* mem,int total){
  int len=total;
  while(len>0){
    int x;
    x=s->buf_len-s->buf_pos;
    if(x==0){
      if(!cache_stream_fill_buffer(s)) return total-len; // EOF
      x=s->buf_len-s->buf_pos;
    }
#ifdef DEBUG
    if(s->buf_pos>s->buf_len) dprintf("stream_read: WARNING! s->buf_pos>s->buf_len\n");
#endif
    if(x>len) x=len;
    memcpy(mem,&s->buffer[s->buf_pos],x);
    s->buf_pos+=x; mem+=x; len-=x;
  }
  return total;
}


inline static unsigned int stream_read_dword(stream_t *s){
  unsigned int y;
  y=stream_read_char(s);
  y=(y<<8)|stream_read_char(s);
  y=(y<<8)|stream_read_char(s);
  y=(y<<8)|stream_read_char(s);
  return y;
}

inline static uint64_t stream_read_qword(stream_t *s){
  uint64_t y;
  y = stream_read_char(s);
  y=(y<<8)|stream_read_char(s);
  y=(y<<8)|stream_read_char(s);
  y=(y<<8)|stream_read_char(s);
  y=(y<<8)|stream_read_char(s);
  y=(y<<8)|stream_read_char(s);
  y=(y<<8)|stream_read_char(s);
  y=(y<<8)|stream_read_char(s);
  return y;
}

#define stream_read_fourcc stream_read_dword_le

inline static unsigned int stream_read_word_le(stream_t *s){
  int x,y;
  x=stream_read_char(s);
  y=stream_read_char(s);
  return (y<<8)|x;
}

inline static unsigned int stream_read_dword_le(stream_t *s){
  unsigned int y;
  y=stream_read_char(s);
  y|=stream_read_char(s)<<8;
  y|=stream_read_char(s)<<16;
  y|=stream_read_char(s)<<24;
  return y;
}

inline static uint64_t stream_read_qword_le(stream_t *s){
  uint64_t y;
  y = stream_read_dword_le(s);
  y|=(uint64_t)stream_read_dword_le(s)<<32;
  return y;
}

inline static int stream_eof(stream_t *s){
  return s->eof;
}

inline static off_t stream_tell(stream_t *s){
  return s->pos+s->buf_pos-s->buf_len;
}

inline static int stream_seek(stream_t *s,off_t pos){

#ifdef DEBUG
  dprintf("seek to 0x%qX\n",(long long)pos);
#endif

  if(pos<s->pos){
    off_t x=pos-(s->pos-s->buf_len);
    if(x>=0){
      s->buf_pos=x;
//      putchar('*');fflush(stdout);
      return 1;
    }
  }
  
  return cache_stream_seek_long(s,pos);
}

inline static int stream_skip(stream_t *s,off_t len){
  //if( (len<0 && (s->flags & STREAM_SEEK_BW)) || (len>2*STREAM_BUFFER_SIZE && (s->flags & STREAM_SEEK_FW)) ) {
    // negative or big skip!
    return stream_seek(s,stream_tell(s)+len);
  //}
  while(len>0){
    int x=s->buf_len-s->buf_pos;
    if(x==0){
      if(!cache_stream_fill_buffer(s)) return 0; // EOF
      x=s->buf_len-s->buf_pos;
    }
    if(x>len) x=len;
    //memcpy(mem,&s->buf[s->buf_pos],x);
    s->buf_pos+=x; len-=x;
  }
  return 1;
}
int stream_control(stream_t *s, int cmd, void *arg);
void stream_reset(stream_t *s);

stream_t* open_stream(char* filename,char** options,int* file_format);

#endif // STREAM_H
