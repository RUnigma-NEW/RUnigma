#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "utils.h"
#include "stream.h"
#include "demuxer.h"
#include "mkv.h"

#include "stheader.h"

#ifdef __cplusplus
extern "C" {
#endif

int demuxer_debug = 1;
static const char FILENAME[] = "demuxer.c";

#define demuxer_printf(x...) do { if (demuxer_debug)printf(x); } while (0)

#define fast_memcpy(a,b,c) memcpy(a,b,c)

char *dvdsub_lang;
char *audio_lang;
int dvdsub_id=0;

sh_sub_t *new_sh_sub_sid(demuxer_t *demuxer, int id, int sid) {
  if (id > MAX_S_STREAMS - 1 || id < 0) {
    demuxer_printf("Requested sub stream id overflow (%d > %d)\n",id, MAX_S_STREAMS);
    return NULL;
  }
  if (demuxer->s_streams[id])
    demuxer_printf("Sub stream %i redefined\n", id);
  else {
    sh_sub_t *sh = calloc(1, sizeof(sh_sub_t));
    demuxer->s_streams[id] = sh;
    sh->sid = sid;
    demuxer_printf("ID_SUBTITLE_ID=%d\n", sid);
    if (dvdsub_id == id) {
      demuxer->sub->id = id;
      demuxer->sub->sh = sh;
    }
  }
  demuxer_printf("id =%d\n",id);
  return demuxer->s_streams[id];
}

void free_sh_sub(sh_sub_t *sh) {
    demuxer_printf("DEMUXER: freeing sh_sub at %p\n", sh);
    free(sh);
    sh = NULL;
}

sh_audio_t* new_sh_audio_aid(demuxer_t *demuxer,int id,int aid){
    if(id > MAX_A_STREAMS-1 || id < 0)
    {
    demuxer_printf("Requested audio stream id overflow (%d > %d)\n",id, MAX_A_STREAMS);
    return NULL;
    }
    if(demuxer->a_streams[id]){
        demuxer_printf("AUDIO %d\n",id);
    } else {
        sh_audio_t *sh;
        //mp_msg(MSGT_DEMUXER,MSGL_V,MSGTR_FoundAudioStream,id);
        demuxer->a_streams[id]=calloc(1, sizeof(sh_audio_t));
        sh = demuxer->a_streams[id];
        // set some defaults
        sh->samplesize=2;
        sh->sample_format=AF_FORMAT_S16_NE;
        sh->audio_out_minsize=8192;/* default size, maybe not enough for Win32/ACM*/
        sh->pts=MP_NOPTS_VALUE;
          demuxer_printf("ID_AUDIO_ID=%d\n", aid);
    }
    ((sh_audio_t *)demuxer->a_streams[id])->aid = aid;
    demuxer_printf("audio_id =%d\n",id);
    audio_id = id;
    return demuxer->a_streams[id];
}

void free_sh_audio(demuxer_t *demuxer, int id) {
    sh_audio_t *sh = demuxer->a_streams[id];
    demuxer->a_streams[id] = NULL;
    demuxer_printf("DEMUXER: freeing sh_audio at %p\n",sh);
    free(sh->wf);
    sh->wf = NULL;
    free(sh);
    sh = NULL;
}

sh_video_t* new_sh_video_vid(demuxer_t *demuxer,int id,int vid){demuxer_printf("new_sh_video_vid->\n");
    if(id > MAX_V_STREAMS-1 || id < 0)
    {
    demuxer_printf("Requested video stream id overflow (%d > %d)\n",
        id, MAX_V_STREAMS);
demuxer_printf("new_sh_video_vid-< nULL\n");
    return NULL;
    }
    if(demuxer->v_streams[id] != NULL){
    demuxer_printf("Warnung VideoStreamRedefined %d\n",id);
    } else {
        demuxer_printf("VideoStreamRedefined %d\n",id);
        demuxer->v_streams[id]=calloc(1, sizeof(sh_video_t));
          demuxer_printf("ID_VIDEO_ID=%d\n", vid);
    }
demuxer_printf("new_sh_video_vid 2\n");

    ((sh_video_t *)demuxer->v_streams[id])->vid = vid;demuxer_printf("new_sh_video_vid-<\n");
    return demuxer->v_streams[id];
}

void free_sh_video(sh_video_t* sh){
    demuxer_printf("DEMUXER: freeing sh_video at %p\n",sh);
    free(sh->bih);
    sh->bih = NULL;
    free(sh);
    sh = NULL;
}
void ds_add_packet(demux_stream_t *ds,demux_packet_t* dp){
//printf("ds_add_packet\n");
//    demux_packet_t* dp=new_demux_packet(len);
//    stream_read(stream,dp->buffer,len);
//    dp->pts=pts; //(float)pts/90000.0f;
//    dp->pos=pos;
    // append packet to DS stream:
    ++ds->packs;
    ds->bytes+=dp->len;
    if(ds->last){
      // next packet in stream
      ds->last->next=dp;
      ds->last=dp;
    } else {
      // first packet in stream
      ds->first=ds->last=dp;
    }
//    demuxer_printf("DEMUX: Append packet to %s, len=%d  pts=%5.3f  pos=%u  [packs: A=%d V=%d]\n",
//        (ds==ds->audio)?"d_audio":"d_video",
//        dp->len,dp->pts,(unsigned int)dp->pos,ds->audio->packs,ds->video->packs);
}

int demux_fill_buffer(demuxer_t *demux,demux_stream_t *ds){
demuxer_printf("%s::%d\n",__FUNCTION__, __LINE__);
  // Note: parameter 'ds' can be NULL!
//  demuxer_printf("demux->type=%d\n",demux->type);

if(demux->desc == NULL)
demuxer_printf("%s::%d demux->desc=NULL\n",__FUNCTION__, __LINE__);

  return demux->desc->fill_buffer(demux, ds);
}
// return value:
//     0 = EOF
//     1 = succesfull
int ds_fill_buffer(demux_stream_t *ds){
demuxer_printf("ds_fill_buffer->\n");
  demuxer_t *demux=ds->demuxer;
  if(ds->current) free_demux_packet(ds->current);

  while(1){
    if(ds->packs){
      demux_packet_t *p=ds->first;
      // copy useful data:
      ds->buffer=p->buffer;
      ds->buffer_pos=0;
      ds->buffer_size=p->len;
      ds->pos=p->pos;
      ds->dpos+=p->len; // !!!
      ++ds->pack_no;

      if (p->pts != (correct_pts ? MP_NOPTS_VALUE : 0)) {
        ds->pts=p->pts;
        ds->pts_bytes=0;
      }
      ds->pts_bytes+=p->len; // !!!
      if(p->stream_pts != MP_NOPTS_VALUE) demux->stream_pts=p->stream_pts;
      ds->flags=p->flags;
      // unlink packet:
      ds->bytes-=p->len;
      ds->current=p;
      ds->first=p->next;
      if(!ds->first) ds->last=NULL;
      --ds->packs;
      return 1; //ds->buffer_size;
    }
/*    if(demux->audio->packs>=MAX_PACKS || demux->audio->bytes>=MAX_PACK_BYTES){
//      mp_msg(MSGT_DEMUXER,MSGL_ERR,MSGTR_TooManyAudioInBuffer,demux->audio->packs,demux->audio->bytes);
//      mp_msg(MSGT_DEMUXER,MSGL_HINT,MSGTR_MaybeNI);
      break;
    }
    if(demux->video->packs>=MAX_PACKS || demux->video->bytes>=MAX_PACK_BYTES){
//      mp_msg(MSGT_DEMUXER,MSGL_ERR,MSGTR_TooManyVideoInBuffer,demux->video->packs,demux->video->bytes);
//      mp_msg(MSGT_DEMUXER,MSGL_HINT,MSGTR_MaybeNI);
      break;
    }*/
demuxer_printf("%s::%d\n",__FUNCTION__, __LINE__);

return 0;

if( demux == NULL)
	demuxer_printf("ds_fill_buffer demux==NULL\n");
if( ds == NULL)
	demuxer_printf("ds_fill_buffer ds==NULL\n");

demuxer_printf("%s::%d\n",__FUNCTION__, __LINE__);
    if(!demux_fill_buffer(demux,ds)){
       demuxer_printf("ds_fill_buffer()->demux_fill_buffer() failed\n");
       break; // EOF
    }
demuxer_printf("%s::%d\n",__FUNCTION__, __LINE__);
  }
  ds->buffer_pos=ds->buffer_size=0;
  ds->buffer=NULL;
  ds->current=NULL;
  demuxer_printf("ds_fill_buffer: EOF reached (stream: %s)  \n",ds==demux->audio?"audio":"video");
  ds->eof=1;
demuxer_printf("ds_fill_buffer-< 0\n");
  return 0;
}
int ds_fill_buffer2(demux_stream_t *ds,demux_stream_t *demuxer){
demuxer_printf("ds_fill_buffer2->\n");
if( demuxer == NULL)
	demuxer_printf("ds_fill_buffer2 demuxer==NULL\n");
  ds->demuxer = demuxer;
  demuxer_t *demux=ds->demuxer;
  if(ds->current) free_demux_packet(ds->current);
  while(1){
    if(ds->packs){
      demux_packet_t *p=ds->first;
      ds->buffer=p->buffer;
      ds->buffer_pos=0;
      ds->buffer_size=p->len;
      ds->pos=p->pos;
      ds->dpos+=p->len; // !!!
      ++ds->pack_no;
      if (p->pts != (correct_pts ? MP_NOPTS_VALUE : 0)) {
        ds->pts=p->pts;
        ds->pts_bytes=0;
      }
      ds->pts_bytes+=p->len; // !!!
      if(p->stream_pts != MP_NOPTS_VALUE) demux->stream_pts=p->stream_pts;
      ds->flags=p->flags;
      ds->bytes-=p->len;
      ds->current=p;
      ds->first=p->next;
      if(!ds->first) ds->last=NULL;
      --ds->packs;
      return 1; //ds->buffer_size;
    }
demuxer_printf("%s::%d\n",__FUNCTION__, __LINE__);
    if(!demux_fill_buffer(demux,ds)){
       demuxer_printf("ds_fill_buffer2()->demux_fill_buffer2() failed\n");
       break; // EOF
    }
demuxer_printf("%s::%d\n",__FUNCTION__, __LINE__);
  }
  ds->buffer_pos=ds->buffer_size=0;
  ds->buffer=NULL;
  ds->current=NULL;
  demuxer_printf("ds_fill_buffer2: EOF reached (stream: %s)  \n",ds==demux->audio?"audio":"video");
  ds->eof=1;
demuxer_printf("ds_fill_buffer2-< 0\n");
  return 0;
}

int demux_read_data(demux_stream_t *ds,unsigned char* mem,int len){
int x;
int bytes=0;
while(len>0){
  x=ds->buffer_size-ds->buffer_pos;
  if(x==0){
    if(!ds_fill_buffer(ds)) return bytes;
  } else {
    if(x>len) x=len;
    if(mem) fast_memcpy(mem+bytes,&ds->buffer[ds->buffer_pos],x);
    bytes+=x;len-=x;ds->buffer_pos+=x;
  }
}
return bytes;
}

int demuxer_add_chapter(demuxer_t* demuxer, const char* name, uint64_t start, uint64_t end){
    if (demuxer->chapters == NULL)
        demuxer->chapters = malloc (32*sizeof(*demuxer->chapters));
    else if (!(demuxer->num_chapters % 32))
        demuxer->chapters = realloc (demuxer->chapters, (demuxer->num_chapters + 32) * sizeof(*demuxer->chapters));

    demuxer->chapters[demuxer->num_chapters].start = start;
    demuxer->chapters[demuxer->num_chapters].end = end;
	printf("strdup in %s::%s:%d\n", FILENAME, __FUNCTION__,__LINE__);
    demuxer->chapters[demuxer->num_chapters].name = strdup(name);

    return demuxer->num_chapters ++;
}

int demuxer_sub_track_by_lang(demuxer_t *d, char *lang)
{
    int i, len;
    lang += strspn(lang, ",");
    while ((len = strcspn(lang, ",")) > 0) {
        for (i = 0; i < MAX_S_STREAMS; ++i) {
            sh_sub_t *sh = d->s_streams[i];
            if (sh && sh->lang && strncmp(sh->lang, lang, len) == 0)
                return sh->sid;
        }
        lang += len;
        lang += strspn(lang, ",");
    }
    return -1;
}
int demux_info_add(demuxer_t *demuxer, const char *opt, const char *param)
{
    char **info = demuxer->info;
    int n = 0;


    for(n = 0; info && info[2*n] != NULL; n++) 
      {
	if(!strcasecmp(opt,info[2*n]))
	  {
	    demuxer_printf("DemuxerInfoChanged %p %p\n",opt,param);
	    free(info[2*n+1]);
		printf("strdup in %s::%s:%d\n", FILENAME, __FUNCTION__,__LINE__);
	    info[2*n+1] = strdup(param);
	    return 0;
	  }
      }
    
    info = demuxer->info = (char**)realloc(info,(2*(n+2))*sizeof(char*));
	printf("2strdup in %s::%s:%d\n", FILENAME, __FUNCTION__,__LINE__);
    info[2*n] = strdup(opt);
    info[2*n+1] = strdup(param);
    memset(&info[2*(n+1)],0,2*sizeof(char*));

    return 1;
}

void ds_read_packet(demux_stream_t *ds, stream_t *stream, int len, double pts, off_t pos, int flags) {
    demux_packet_t* dp=new_demux_packet(len);
    len = stream_read(stream,dp->buffer,len);
    resize_demux_packet(dp, len);
    dp->pts=pts; //(float)pts/90000.0f;
    dp->pos=pos;
    dp->flags=flags;
    // append packet to DS stream:
    ds_add_packet(ds,dp);
}

// Reset and set internal data but do not free the instance
void ds_free_packs(demux_stream_t *ds){
  
	if (ds != NULL) {
		demux_packet_t *dp=ds->first;
		
		while(dp){
			demux_packet_t *dn=dp->next;
			free_demux_packet(dp);
			dp=dn;
		}
		
		if(ds->asf_packet){
			// free unfinished .asf fragments:
			free(ds->asf_packet->buffer);
			ds->asf_packet->buffer = NULL;
			free(ds->asf_packet);
			ds->asf_packet=NULL;
		}
		
		ds->first=ds->last=NULL;
		ds->packs=0; // !!!!!
		ds->bytes=0;
		
		free_demux_packet(ds->current);
		
		ds->buffer=NULL;
		ds->buffer_pos=ds->buffer_size;
		ds->pts=0; ds->pts_bytes=0;		
	}
}

void demux_flush(demuxer_t *demuxer)
{
	ds_free_packs(demuxer->video);
	ds_free_packs(demuxer->audio);
	ds_free_packs(demuxer->sub);
}

/**
 * \brief read data until the given 3-byte pattern is encountered, up to maxlen
 * \param mem memory to read data into, may be NULL to discard data
 * \param maxlen maximum number of bytes to read
 * \param read number of bytes actually read
 * \param pattern pattern to search for (lowest 8 bits are ignored)
 * \return whether pattern was found
 */
int demux_pattern_3(demux_stream_t *ds, unsigned char *mem, int maxlen,
                    int *read, uint32_t pattern)
{
    register uint32_t head = 0xffffff00;
    register uint32_t pat = pattern & 0xffffff00;
    int total_len = 0;
    do {
        register unsigned char *ds_buf = &ds->buffer[ds->buffer_size];
        int len = ds->buffer_size - ds->buffer_pos;
        register long pos = -len;
        if (unlikely(pos >= 0)) { // buffer is empty
            ds_fill_buffer(ds);
            continue;
        }
        do {
            head |= ds_buf[pos];
            head <<= 8;
        } while (++pos && head != pat);
        len += pos;
        if (total_len + len > maxlen)
            len = maxlen - total_len;
        len = demux_read_data(ds, mem ? &mem[total_len] : NULL, len);
        total_len += len;
    } while ((head != pat || total_len < 3) && total_len < maxlen && !ds->eof);
    if (read)
        *read = total_len;
    return total_len >= 3 && head == pat;
}
