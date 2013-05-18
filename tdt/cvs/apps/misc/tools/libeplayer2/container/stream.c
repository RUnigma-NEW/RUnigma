#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#ifndef __MINGW32__
#include <sys/ioctl.h>
#include <sys/wait.h>
#endif
#include <fcntl.h>
#include <signal.h>
#include <strings.h>

#include "stream.h"
#include "demuxer.h"

#ifdef __cplusplus
extern "C" {
#endif

int stream_debug = 0;
#define stream_printf(x...) do { if (stream_debug)printf(x); } while (0)

/////////////////////////////////////7

static int file_fill_buffer(stream_t *s, char* buffer, int max_len){
  int r = read(s->fd,buffer,max_len);
  return (r <= 0) ? -1 : r;
}

static int file_seek(stream_t *s,off_t newpos) {
  s->pos = newpos;
  if(lseek(s->fd,s->pos,SEEK_SET)<0) {
    s->eof=1;
    return 0;
  }
  return 1;
}

void file_reset(stream_t *s){
  if(s->eof){
    s->pos=0; //ftell(f);
//    s->buf_pos=s->buf_len=0;
    s->eof=0;
  }
  //if(s->control) s->control(s,STREAM_CTRL_RESET,NULL);
  lseek(s->fd, 0L, SEEK_SET);
}

/////////////////////////////////////7

int stream_fill_buffer(stream_t *s){
  stream_printf("stream_fill_buffer->\n");
  
  int len;
  if (/*s->fd == NULL ||*/ s->eof) { 
      stream_printf("stream_fill_buffer s->eof !\n");
    s->buf_pos = s->buf_len = 0; return 0; 
  }
  switch(s->type){
  case STREAMTYPE_STREAM:
    len=read(s->fd,s->buffer,STREAM_BUFFER_SIZE);break;
  case STREAMTYPE_DS:
    len = demux_read_data((demux_stream_t*)s->priv,s->buffer,STREAM_BUFFER_SIZE);
    break;

  default: 
    len= file_fill_buffer(s,(char*) s->buffer,STREAM_BUFFER_SIZE);
    break;
  }
  stream_printf("stream_fill_buffer-< len=%d\n", len);
  if(len<=0){ s->eof=1; s->buf_pos=s->buf_len=0; return 0; }
  s->buf_pos=0;
  s->buf_len=len;
  s->pos+=len;
//  stream_printf("[%d]",len);fflush(stdout);

  return len;
}


int stream_seek_long(stream_t *s,off_t pos) {
stream_printf("stream_seek_long> 0x%lx ->\n", (long unsigned int) pos);

  off_t newpos=0;

  s->buf_pos=s->buf_len=0;

  if(s->mode == STREAM_WRITE) {
    if(!file_seek(s,pos))
      return 0;
    return 1;
  }

  if(s->sector_size)
      newpos = (pos/s->sector_size)*s->sector_size;
  else
      newpos = pos&(~((off_t)STREAM_BUFFER_SIZE-1));

  pos-=newpos;

  if(newpos==0 || newpos!=s->pos){
    switch(s->type){
      case STREAMTYPE_STREAM:
      //s->pos=newpos; // real seek
      // Some streaming protocol allow to seek backward and forward
      // A function call that return -1 can tell that the protocol
      // doesn't support seeking.

        if(newpos<s->pos){
          stream_printf("Cannot seek backward in linear streams!\n");
          return 1;
        }
        while(s->pos<newpos){
          stream_printf("\ts->pos=0x%lx < newpos=0x%lx\n", (long unsigned int) s->pos, (long unsigned int) newpos);
          if(stream_fill_buffer(s)<=0) break; // EOF
        }

	//lseek(s->fd, newpos, SEEK_SET);
	//s->pos=newpos;

        break;
    default:
      // Now seek
      if(!file_seek(s,newpos)) {
        stream_printf("Seek failed\n");
        return 0;
      }
    }
//   putchar('.');fflush(stdout);
//} else {
//   putchar('%');fflush(stdout);
  }

  while(stream_fill_buffer(s) > 0 && pos >= 0) {
    if(pos<=s->buf_len){
      s->buf_pos=pos; // byte position in sector
      return 1;
    }
    pos -= s->buf_len;
  }
  
//  if(pos==s->buf_len) stream_printf("XXX Seek to last byte of file -> EOF\n");
  
  stream_printf("stream_seek: WARNING! Can't seek to 0x%"PRIX64" !\n",(int64_t)(pos+newpos));
  return 0;
}

int stream_control(stream_t *s, int cmd, void *arg){
  if(!s->control) return STREAM_UNSUPPORTED;
  return s->control(s, cmd, arg);
}
void stream_reset(stream_t *s){
  if(s->type == STREAMTYPE_FILE)
  	return file_reset(s);
  if(s->eof){
    s->pos=0; //ftell(f);
//    s->buf_pos=s->buf_len=0;
    s->eof=0;
  }
  //if(s->control) s->control(s,STREAM_CTRL_RESET,NULL);
  //lseek(s->fd, 0L, SEEK_SET);
}
