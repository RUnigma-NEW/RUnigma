//  AVI file parser for DEMUXER v2.9  by A'rpi/ESP-team

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <memory.h>
#undef memcpy
#include <string.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/poll.h>
#include <pthread.h>

#include "stream.h"
#include "demuxer.h"
#include "stheader.h"
#include "aviheader.h"

#ifdef DEBUG
int demux_avi_debug = 0;
#define demux_avi_printf(x...) do { if (demux_avi_debug)printf(x); } while (0)
#endif

static const char FILENAME[] = "avi.c";

pthread_mutex_t AVImutex;

void getAVIMutex(const char *filename, const char *function, int line) {	// FIXME: Use one central getMutex and pass the mutex into the function
	#ifdef DEBUG
//	printf("%s::%s::%d requesting mutex\n",filename, function, line);
	#endif

	pthread_mutex_lock(&AVImutex);

	#ifdef DEBUG
//	printf("%s::%s::%d received mutex\n",filename, function, line);
	#endif
}

void releaseAVIMutex(const char *filename, const char *function, int line) {	// FIXME: Use one central getMutex and pass the mutex into the function
	pthread_mutex_unlock(&AVImutex);

	#ifdef DEBUG
//	printf("%s::%s::%d released mutex\n",filename, function, line);
	#endif
}

demuxer_t* init_avi_with_ogg(demuxer_t* demuxer);
int demux_ogg_open(demuxer_t* demuxer);

// PTS:  0=interleaved  1=BPS-based
int pts_from_bps=1;

// Select ds from ID
demux_stream_t* demux_avi_select_stream(demuxer_t *demux,unsigned int id){

  int stream_id=avi_stream_id(id);
#ifdef DEBUG
demux_avi_printf("%s::%d id=%u stream_id=%d\n",__FUNCTION__, __LINE__, id, stream_id);
#endif

  if(demux->video->id==-1)
    if(demux->v_streams[stream_id])
        demux->video->id=stream_id;

  if(demux->audio->id==-1)
    if(demux->a_streams[stream_id])
        demux->audio->id=stream_id;

  if(stream_id==demux->audio->id){
      if(!demux->audio->sh){
        sh_audio_t* sh;
	avi_priv_t *priv=demux->priv;
        sh=demux->audio->sh=demux->a_streams[stream_id];

	#ifdef DEBUG
        demux_avi_printf("Auto-selected AVI audio ID = %d\n",demux->audio->id);
	#endif

	if(sh->wf){
	  priv->audio_block_size=sh->wf->nBlockAlign;
	  if(!priv->audio_block_size){
	    // for PCM audio we can calculate the blocksize:
	    if(sh->format==1)
		priv->audio_block_size=sh->wf->nChannels*(sh->wf->wBitsPerSample/8);
	    else
		priv->audio_block_size=1; // hope the best...
	  } else {
	    // workaround old mencoder's bug:
	    if(sh->audio.dwSampleSize==1 && sh->audio.dwScale==1 &&
	       (sh->wf->nBlockAlign==1152 || sh->wf->nBlockAlign==576)){

		#ifdef DEBUG
		demux_avi_printf("WorkAroundBlockAlignHeaderBug\n");
		#endif

		priv->audio_block_size=1;
	    }
	  }
	} else {
	  priv->audio_block_size=sh->audio.dwSampleSize;
	}
      }
      return demux->audio;
  }
  if(stream_id==demux->video->id){
      if(!demux->video->sh){
        demux->video->sh=demux->v_streams[stream_id];

	#ifdef DEBUG
        demux_avi_printf("Auto-selected AVI video ID = %d\n",demux->video->id);
	#endif
      }
      return demux->video;
  }
  if(id!=mmioFOURCC('J','U','N','K')){
     	// unknown

	#ifdef DEBUG
     	demux_avi_printf("Unknown chunk: %.4s (%X)\n",(char *) &id,id);
	#endif

     	//abort();
  }
  return NULL;
}

static int valid_fourcc(unsigned int id){
  #ifdef DEBUG
  printf("log demux_avi 2\n");
  #endif
  
    static const char valid[] = "0123456789abcdefghijklmnopqrstuvwxyz"
                                "ABCDEFGHIJKLMNOPQRSTUVWXYZ_";
    unsigned char* fcc=(unsigned char*)(&id);
    return strchr(valid, fcc[0]) && strchr(valid, fcc[1]) &&
           strchr(valid, fcc[2]) && strchr(valid, fcc[3]);
}

static int choose_chunk_len(unsigned int len1,unsigned int len2){printf("log demux_avi 3\n");
    // len1 has a bit more priority than len2. len1!=len2
    // Note: this is a first-idea-logic, may be wrong. comments welcomed.

    // prefer small frames rather than 0
    if(!len1) return (len2>0x80000) ? len1 : len2;
    if(!len2) return (len1>0x100000) ? len2 : len1;

    // choose the smaller value:
    return (len1<len2)? len1 : len2;
}

static int demux_avi_read_packet(demuxer_t *demux,demux_stream_t *ds,unsigned int id,unsigned int len,int idxpos,int flags){
  #ifdef DEBUG
  demux_avi_printf("%s::%d\n",__FUNCTION__, __LINE__);
  #endif

  avi_priv_t *priv=demux->priv;
  int skip;
  float pts=0;
  
  #ifdef DEBUG
  demux_avi_printf("demux_avi.read_packet: %X\n",id);
  #endif

  if(ds==demux->audio){
      if(priv->pts_corrected==0){
          if(priv->pts_has_video){
	      // we have video pts now
	      float delay=0;
	      if(((sh_audio_t*)(ds->sh))->wf->nAvgBytesPerSec)
	          delay=(float)priv->pts_corr_bytes/((sh_audio_t*)(ds->sh))->wf->nAvgBytesPerSec;
	      
	      #ifdef DEBUG
	      printf("XXX initial  v_pts=%5.3f  a_pos=%d (%5.3f) \n",priv->avi_audio_pts,priv->pts_corr_bytes,delay);
	      #endif
	      
	      //priv->pts_correction=-priv->avi_audio_pts+delay;
	      priv->pts_correction=delay-priv->avi_audio_pts;
	      //priv->avi_audio_pts+=priv->pts_correction;
	      priv->pts_corrected=1;
	  } else
	      priv->pts_corr_bytes+=len;
      }
      if(pts_from_bps){
	  pts = priv->audio_block_no *
	    (float)((sh_audio_t*)demux->audio->sh)->audio.dwScale /
	    (float)((sh_audio_t*)demux->audio->sh)->audio.dwRate;
      } else
          pts=priv->avi_audio_pts; //+priv->pts_correction;
      priv->avi_audio_pts=0;
      // update blockcount:
      priv->audio_block_no+=priv->audio_block_size ?
	((len+priv->audio_block_size-1)/priv->audio_block_size) : 1;
  } else 
  if(ds==demux->video){
     // video
     if(priv->skip_video_frames>0){
       // drop frame (seeking)
       --priv->skip_video_frames;
       ds=NULL;
     }

     pts = priv->avi_video_pts = priv->video_pack_no *
         (float)((sh_video_t*)demux->video->sh)->video.dwScale /
	 (float)((sh_video_t*)demux->video->sh)->video.dwRate;

     priv->avi_audio_pts=priv->avi_video_pts+priv->pts_correction;
     priv->pts_has_video=1;

     if(ds) ++priv->video_pack_no;

  }
  
  skip=(len+1)&(~1); // total bytes in this chunk
  
  #ifdef DEBUG
  demux_avi_printf("%s::%d\n",__FUNCTION__, __LINE__);
  #endif

  if(ds){
    #ifdef DEBUG
    demux_avi_printf("DEMUX_AVI: Read %d data bytes from packet %04X\n",len,id);
    #endif

    ds_read_packet(ds,demux->stream,len,pts,idxpos,flags);
    skip-=len;
  }
  skip = FFMAX(skip, 0);
  if (avi_stream_id(id) > 99 && id != mmioFOURCC('J','U','N','K'))
    skip = FFMIN(skip, 65536);
  if(skip){
    #ifdef DEBUG
    demux_avi_printf("DEMUX_AVI: Skipping %d bytes from packet %04X\n",skip,id);
    #endif

    stream_skip(demux->stream,skip);
  }
  return ds?1:0;
}

static uint32_t avi_find_id(stream_t *stream) {
  uint32_t id = stream_read_dword_le(stream);
  if (!id) {
    #ifdef DEBUG
    demux_avi_printf("Incomplete stream? Trying resync.\n");
    #endif
    
    do {
      id = stream_read_dword_le(stream);
      if (stream_eof(stream)) return 0;
    } while (avi_stream_id(id) > 99);
  }
  return id;
}

// return value:
//     0 = EOF or no stream found
//     1 = successfully read a packet
static int demux_avi_fill_buffer(demuxer_t *demux, demux_stream_t *dsds){
//printf("%s::%d\n", __FUNCTION__, __LINE__);
avi_priv_t *priv=demux->priv;
unsigned int id=0;
unsigned int len;
int ret=0;
demux_stream_t *ds;

do{
  int flags=1;
  AVIINDEXENTRY *idx=NULL;
  if(priv->idx_size>0 && priv->idx_pos<priv->idx_size){
    off_t pos;
    
    idx=&((AVIINDEXENTRY *)priv->idx)[priv->idx_pos++];
    
    if(idx->dwFlags&AVIIF_LIST){
      // LIST
      continue;
    }
//printf("%s::%d\n", __FUNCTION__, __LINE__);
    if(!demux_avi_select_stream(demux,idx->ckid)){
      #ifdef DEBUG
      demux_avi_printf("Skip chunk %.4s (0x%X)  \n",(char *)&idx->ckid,(unsigned int)idx->ckid);
      #endif
      
      continue; // skip this chunk
    }
//printf("%s::%d\n", __FUNCTION__, __LINE__);
    pos = (off_t)priv->idx_offset+AVI_IDX_OFFSET(idx);
    if((pos<demux->movi_start || pos>=demux->movi_end) && (demux->movi_end>demux->movi_start) && (demux->stream->flags & STREAM_SEEK)){
      #ifdef DEBUG
      demux_avi_printf("ChunkOffset out of range!   idx=0x%"PRIX64"  \n",(int64_t)pos);
      #endif
      
      continue;
    }
    stream_seek(demux->stream,pos);
    demux->filepos=stream_tell(demux->stream);

//printf("%s::%d %u %u\n", __FUNCTION__, __LINE__, pos, demux->filepos);

    id=stream_read_dword_le(demux->stream);
//printf("%s::%d\n", __FUNCTION__, __LINE__);
    if(stream_eof(demux->stream)) return 0; // EOF!
//printf("%s::%d\n", __FUNCTION__, __LINE__);
    
    if(id!=idx->ckid){
      #ifdef DEBUG
      demux_avi_printf("ChunkID mismatch! raw=%.4s idx=%.4s  \n",(char *)&id,(char *)&idx->ckid);
      #endif
      
      if(valid_fourcc(idx->ckid))
          id=idx->ckid;	// use index if valid
      else
          if(!valid_fourcc(id)) continue; // drop chunk if both id and idx bad
    }
    len=stream_read_dword_le(demux->stream);
    if((len!=idx->dwChunkLength)&&((len+1)!=idx->dwChunkLength)){
      #ifdef DEBUG
      demux_avi_printf("ChunkSize mismatch! raw=%d idx=%d  \n",len,idx->dwChunkLength);
      #endif

      if(len>0x200000 && idx->dwChunkLength>0x200000) continue; // both values bad :(
      len=choose_chunk_len(idx->dwChunkLength,len);
    }
    if(!(idx->dwFlags&AVIIF_KEYFRAME)) flags=0;
  } else {
    demux->filepos=stream_tell(demux->stream);
    if(demux->filepos>=demux->movi_end && demux->movi_end>demux->movi_start && (demux->stream->flags & STREAM_SEEK)){
          demux->stream->eof=1;
          return 0;
    }
    id=avi_find_id(demux->stream);
    len=stream_read_dword_le(demux->stream);
    if(stream_eof(demux->stream)) return 0; // EOF!
    
    if(id==mmioFOURCC('L','I','S','T') || id==mmioFOURCC('R', 'I', 'F', 'F')){
      id=stream_read_dword_le(demux->stream); // list or RIFF type
      continue;
    }
  }
//printf("%s::%d\n", __FUNCTION__, __LINE__);
  ds=demux_avi_select_stream(demux,id);
//printf("%s::%d\n", __FUNCTION__, __LINE__);
  if(ds)
    if(ds->packs+1>=MAX_PACKS || ds->bytes+len>=MAX_PACK_BYTES){
	// this packet will cause a buffer overflow, switch to -ni mode!!!
	#ifdef DEBUG
	demux_avi_printf("SwitchToNi\n");
	#endif
	
	if(priv->idx_size>0){
	    // has index
	    //demux->type=DEMUXER_TYPE_AVI_NI;
	    //demux->desc=&demuxer_desc_avi_ni;
	    --priv->idx_pos; // hack
	} else {
	    // no index
	    //demux->type=DEMUXER_TYPE_AVI_NINI;
	    //demux->desc=&demuxer_desc_avi_nini;
	    priv->idx_pos=demux->filepos; // hack
	}
	priv->idx_pos_v=priv->idx_pos_a=priv->idx_pos;
	// quit now, we can't even (no enough buffer memory) read this packet :(
	return -1;
    }

//printf("%s::%d\n", __FUNCTION__, __LINE__);
  ret=demux_avi_read_packet(demux,ds,id,len,priv->idx_pos-1,flags);
} while(ret!=1);
  return 1;
}


// return value:
//     0 = EOF or no stream found
//     1 = successfully read a packet
int demux_avi_fill_buffer_ni(demuxer_t *demux,demux_stream_t* ds){printf("log demux_avi 7\n");
avi_priv_t *priv=demux->priv;
unsigned int id=0;
unsigned int len;
int ret=0;

do{
  int flags=1;
  AVIINDEXENTRY *idx=NULL;
  int idx_pos=0;
  demux->filepos=stream_tell(demux->stream);
  
  if(ds==demux->video) idx_pos=priv->idx_pos_v++; else
  if(ds==demux->audio) idx_pos=priv->idx_pos_a++; else
                       idx_pos=priv->idx_pos++;
  
  #ifdef DEBUG
  printf("%s::%d\n", __FUNCTION__, __LINE__);
  #endif
  
  if(priv->idx_size>0 && idx_pos<priv->idx_size){
    off_t pos;
    idx=&((AVIINDEXENTRY *)priv->idx)[idx_pos];
    
    if(idx->dwFlags&AVIIF_LIST){
      // LIST
      continue;
    }
    if(ds && demux_avi_select_stream(demux,idx->ckid)!=ds){
      #ifdef DEBUG
      demux_avi_printf("Skip chunk %.4s (0x%X)  \n",(char *)&idx->ckid,(unsigned int)idx->ckid);
      #endif

      continue; // skip this chunk
    }

    pos = priv->idx_offset+AVI_IDX_OFFSET(idx);
    if((pos<demux->movi_start || pos>=demux->movi_end) && (demux->movi_end>demux->movi_start)){
      #ifdef DEBUG
      demux_avi_printf("ChunkOffset out of range!  current=0x%"PRIX64"  idx=0x%"PRIX64"  \n",(int64_t)demux->filepos,(int64_t)pos);
      #endif
      
      continue;
    }
    stream_seek(demux->stream,pos);

    id=stream_read_dword_le(demux->stream);

    if(stream_eof(demux->stream)) return 0;

    if(id!=idx->ckid){
      #ifdef DEBUG
      demux_avi_printf("ChunkID mismatch! raw=%.4s idx=%.4s  \n",(char *)&id,(char *)&idx->ckid);
      #endif
      
      if(valid_fourcc(idx->ckid))
          id=idx->ckid;	// use index if valid
      else
          if(!valid_fourcc(id)) continue; // drop chunk if both id and idx bad
    }
    len=stream_read_dword_le(demux->stream);
    if((len!=idx->dwChunkLength)&&((len+1)!=idx->dwChunkLength)){
      #ifdef DEBUG
      demux_avi_printf("ChunkSize mismatch! raw=%d idx=%d  \n",len,idx->dwChunkLength);
      #endif

      if(len>0x200000 && idx->dwChunkLength>0x200000) continue; // both values bad :(
      len=choose_chunk_len(idx->dwChunkLength,len);
    }
    if(!(idx->dwFlags&AVIIF_KEYFRAME)) flags=0;
  } else return 0;
  
  #ifdef DEBUG
  printf("%s::%d\n", __FUNCTION__, __LINE__);
  #endif
  
  ret=demux_avi_read_packet(demux,demux_avi_select_stream(demux,id),id,len,idx_pos,flags);
} while(ret!=1);
  return 1;
}


// return value:
//     0 = EOF or no stream found
//     1 = successfully read a packet
int demux_avi_fill_buffer_nini(demuxer_t *demux,demux_stream_t* ds){
  #ifdef DEBUG
  printf("log demux_avi 8\n");
  #endif

  avi_priv_t *priv=demux->priv;
  unsigned int id=0;
  unsigned int len;
  int ret=0;
  off_t *fpos=NULL;

  if(ds==demux->video) fpos=&priv->idx_pos_v; else
  if(ds==demux->audio) fpos=&priv->idx_pos_a; else
  return 0;

  stream_seek(demux->stream,fpos[0]);

do{

  demux->filepos=stream_tell(demux->stream);
  if(demux->filepos>=demux->movi_end && (demux->movi_end>demux->movi_start)){
	  ds->eof=1;
          return 0;
  }

  id=avi_find_id(demux->stream);
  len=stream_read_dword_le(demux->stream);

  if(stream_eof(demux->stream)) return 0;
  
  if(id==mmioFOURCC('L','I','S','T')){
      id=stream_read_dword_le(demux->stream);      // list type
      continue;
  }
  
  if(id==mmioFOURCC('R','I','F','F')){
      #ifdef DEBUG
      demux_avi_printf("additional RIFF header...\n");
      #endif

      id=stream_read_dword_le(demux->stream);      // "AVIX"
      continue;
  }
  
  #ifdef DEBUG
  printf("%s::%d\n", __FUNCTION__, __LINE__);
  #endif

  if(ds==demux_avi_select_stream(demux,id)){
    // read it!
    ret=demux_avi_read_packet(demux,ds,id,len,priv->idx_pos-1,0);
  } else {
    // skip it!
    int skip=(len+1)&(~1); // total bytes in this chunk
    stream_skip(demux->stream,skip);
  }
  
  #ifdef DEBUG
  printf("%s::%d\n", __FUNCTION__, __LINE__);
  #endif
} while(ret!=1);
  fpos[0]=stream_tell(demux->stream);
  return 1;
}

// AVI demuxer parameters:
int index_mode=-1;  // -1=untouched  0=don't use index  1=use (generate) index
char *index_file_save = NULL, *index_file_load = NULL;
int force_ni=0;     // force non-interleaved AVI parsing

void read_avi_header(demuxer_t *demuxer,int index_mode);
/* konfetti: next removed from signature because the descr structure
 * has only one parameter.
 * ->see demuxer.h
 */
static demuxer_t* demux_open_avi(demuxer_t* demuxer){
  #ifdef DEBUG
  printf("%s::%d\n", __FUNCTION__, __LINE__);
  #endif
  
    demux_stream_t *d_audio=demuxer->audio;
    demux_stream_t *d_video=demuxer->video;
    sh_audio_t *sh_audio=NULL;
    sh_video_t *sh_video=NULL;
    avi_priv_t* priv=malloc(sizeof(avi_priv_t));

  // priv struct:
  priv->avi_audio_pts=priv->avi_video_pts=0.0f;
  priv->pts_correction=0.0f;
  priv->skip_video_frames=0;
  priv->pts_corr_bytes=0;
  priv->pts_has_video=priv->pts_corrected=0;
  priv->video_pack_no=0;
  priv->audio_block_no=0;
  priv->audio_block_size=0;
  priv->isodml = 0;
  priv->suidx_size = 0;
  priv->suidx = NULL;
  priv->duration = 0;

  demuxer->priv=(void*)priv;

  //---- AVI header:
  read_avi_header(demuxer,(demuxer->stream->flags & STREAM_SEEK_BW)?index_mode:-2);

  if(demuxer->audio->id>=0 && !demuxer->a_streams[demuxer->audio->id]){
      #ifdef DEBUG
      demux_avi_printf("InvalidAudioStreamNosound %p\n",demuxer->audio->id);
      #endif

      demuxer->audio->id=-2; // disabled
  }
  if(demuxer->video->id>=0 && !demuxer->v_streams[demuxer->video->id]){
      #ifdef DEBUG
      demux_avi_printf("InvalidAudioStreamUsingDefault %p\n",demuxer->video->id);
      #endif

      demuxer->video->id=-1; // autodetect
  }
  
  #ifdef DEBUG
  printf("%s::%d\n", __FUNCTION__, __LINE__);
  #endif

  stream_reset(demuxer->stream);
  
  #ifdef DEBUG
  printf("%s::%d\n", __FUNCTION__, __LINE__);
  #endif

  stream_seek(demuxer->stream,demuxer->movi_start);
  
  #ifdef DEBUG
  printf("%s::%d\n", __FUNCTION__, __LINE__);
  #endif

  priv->idx_pos=0;
  priv->idx_pos_a=0;
  priv->idx_pos_v=0;
  if(priv->idx_size>1){
    // decide index format:
#if 1
    if((AVI_IDX_OFFSET(&((AVIINDEXENTRY *)priv->idx)[0])<demuxer->movi_start ||
        AVI_IDX_OFFSET(&((AVIINDEXENTRY *)priv->idx)[1])<demuxer->movi_start )&& !priv->isodml)
      priv->idx_offset=demuxer->movi_start-4;
    else
      priv->idx_offset=0;
#else
    if(AVI_IDX_OFFSET(&((AVIINDEXENTRY *)priv->idx)[0])<demuxer->movi_start)
      priv->idx_offset=demuxer->movi_start-4;
    else
      priv->idx_offset=0;
#endif

    #ifdef DEBUG
    demux_avi_printf("AVI index offset: 0x%X (movi=0x%X idx0=0x%X idx1=0x%X)\n",		     
	    (int)priv->idx_offset,(int)demuxer->movi_start,
	    (int)((AVIINDEXENTRY *)priv->idx)[0].dwChunkOffset,
	    (int)((AVIINDEXENTRY *)priv->idx)[1].dwChunkOffset);
    #endif
  }
  
  if(priv->idx_size>0){
      // check that file is non-interleaved:
      int i;
      off_t a_pos=-1;
      off_t v_pos=-1;
      for(i=0;i<priv->idx_size;i++){
        AVIINDEXENTRY* idx=&((AVIINDEXENTRY *)priv->idx)[i];
        demux_stream_t* ds=demux_avi_select_stream(demuxer,idx->ckid);
        off_t pos = priv->idx_offset + AVI_IDX_OFFSET(idx);
        if(a_pos==-1 && ds==demuxer->audio){
          a_pos=pos;
          if(v_pos!=-1) break;
        }
        if(v_pos==-1 && ds==demuxer->video){
          v_pos=pos;
          if(a_pos!=-1) break;
        }
      }
      if(v_pos==-1){
	#ifdef DEBUG
        demux_avi_printf("AVI_NI: MissingVideoStream\n");
	#endif

	return NULL;
      }
      if(a_pos==-1){
        d_audio->sh=sh_audio=NULL;
      } else {
        if(force_ni || abs(a_pos-v_pos)>0x100000){  // distance > 1MB
          //mp_msg(MSGT_DEMUX,MSGL_INFO,MSGTR_NI_Message,force_ni?MSGTR_NI_Forced:MSGTR_NI_Detected);
          //demuxer->type=DEMUXER_TYPE_AVI_NI; // HACK!!!!
          //demuxer->desc=&demuxer_desc_avi_ni; // HACK!!!!
	  pts_from_bps=1; // force BPS sync!
        }
      }
  } else {
      // no index
      if(force_ni){
	  #ifdef DEBUG
          demux_avi_printf("UsingNINI\n");
	  #endif
	  
          //demuxer->type=DEMUXER_TYPE_AVI_NINI; // HACK!!!!
          //demuxer->desc=&demuxer_desc_avi_nini; // HACK!!!!
	  priv->idx_pos_a=
	  priv->idx_pos_v=demuxer->movi_start;
	  pts_from_bps=1; // force BPS sync!
      }
      demuxer->seekable=0;
  }

    #ifdef DEBUG
    demux_avi_printf("%s::%d packs=%u\n", __FUNCTION__, __LINE__, d_video->packs);
    #endif

#ifdef buggy
  if(next == 0){
    if(!ds_fill_buffer(d_video)){
	#ifdef DEBUG
        demux_avi_printf("AVI: MissingVideoStreamBug\n");
	#endif
	
       return NULL;
    }
  }else if(next == 1){
    if(!ds_fill_buffer2(d_video,demuxer)){
       #ifdef DEBUG
       demux_avi_printf("AVI: MissingVideoStreamBug\n");
       #endif
       
       return NULL;
    }
  }
#else
/* konfetti: dont know the meaning of ds_fill_buffer2 but
 * the second parameter is wrong in the call above. mplayer
 * does not need this 2nd function so I remove it here since
 * someone can explain the what it is for!
 */
    if(!ds_fill_buffer(d_video)){
	#ifdef DEBUG
        demux_avi_printf("AVI: MissingVideoStreamBug\n");
	#endif
	
       return NULL;
    }
#endif  
    #ifdef DEBUG
    demux_avi_printf("%s::%d\n", __FUNCTION__, __LINE__);
    #endif

  sh_video=d_video->sh;sh_video->ds=d_video;
  if(d_audio->id!=-2){
    #ifdef DEBUG
    demux_avi_printf("AVI: Searching for audio stream (id:%d)\n",d_audio->id);
    #endif
    
#ifdef buggy
    if(next == 0){
      if(!priv->audio_streams || !ds_fill_buffer(d_audio)){
	#ifdef DEBUG
        demux_avi_printf("AVI: MissingAudioStream\n");
	#endif

        d_audio->sh=sh_audio=NULL;
      } else {
        sh_audio=d_audio->sh;sh_audio->ds=d_audio;
      }
    }else if(next == 1){
      if(!priv->audio_streams || !ds_fill_buffer2(d_audio,demuxer)){
	#ifdef DEBUG
        demux_avi_printf("AVI: MissingAudioStream\n");
	#endif

	d_audio->sh=sh_audio=NULL;
      } else {
        sh_audio=d_audio->sh;sh_audio->ds=d_audio;
      }
    }
#else
/* konfetti: dont know the meaning of ds_fill_buffer2 but
 * the second parameter is wrong in the call above. mplayer
 * does not need this 2nd function so I remove it here since
 * someone can explain the what it is for!
 */
      if(!priv->audio_streams || !ds_fill_buffer(d_audio)){
	#ifdef DEBUG
        demux_avi_printf("AVI: MissingAudioStream\n");
	#endif

        d_audio->sh=sh_audio=NULL;
      } else {
        sh_audio=d_audio->sh;sh_audio->ds=d_audio;
      }
#endif
  }

    #ifdef DEBUG
    demux_avi_printf("%s::%d\n", __FUNCTION__, __LINE__);
    #endif
    
  // calculating audio/video bitrate:
  if(priv->idx_size>0){
    // we have index, let's count 'em!
    int64_t vsize=0;
    int64_t asize=0;
    size_t vsamples=0;
    size_t asamples=0;
    int i;
    for(i=0;i<priv->idx_size;i++){ 
      int id=avi_stream_id(((AVIINDEXENTRY *)priv->idx)[i].ckid);
      int len=((AVIINDEXENTRY *)priv->idx)[i].dwChunkLength;
      if(sh_video->ds->id == id) {
        vsize+=len;
        ++vsamples;
      }
      else if(sh_audio && sh_audio->ds->id == id) {
        asize+=len;
	asamples+=(len+priv->audio_block_size-1)/priv->audio_block_size;
      }
    }
    
    #ifdef DEBUG
    demux_avi_printf("AVI video size=%"PRId64" (%u) audio size=%"PRId64" (%u)\n",vsize,vsamples,asize,asamples);
    #endif

    priv->numberofframes=vsamples;
    sh_video->i_bps=((float)vsize/(float)vsamples)*(float)sh_video->video.dwRate/(float)sh_video->video.dwScale;
    if(sh_audio) sh_audio->i_bps=((float)asize/(float)asamples)*(float)sh_audio->audio.dwRate/(float)sh_audio->audio.dwScale;
  } else {
    // guessing, results may be inaccurate:
    int64_t vsize;
    int64_t asize=0;

    if((priv->numberofframes=sh_video->video.dwLength)<=1)
      // bad video header, try to get number of frames from audio
      if(sh_audio && sh_audio->wf->nAvgBytesPerSec) priv->numberofframes=sh_video->fps*sh_audio->audio.dwLength/sh_audio->audio.dwRate*sh_audio->audio.dwScale;
    if(priv->numberofframes<=1){
      #ifdef DEBUG
      demux_avi_printf("CouldntDetFNo\n");
      #endif
      
      priv->numberofframes=0;
    }          

    if(sh_audio){
      if(sh_audio->wf->nAvgBytesPerSec && sh_audio->audio.dwSampleSize!=1){
        asize=(float)sh_audio->wf->nAvgBytesPerSec*sh_audio->audio.dwLength*sh_audio->audio.dwScale/sh_audio->audio.dwRate;
      } else {
        asize=sh_audio->audio.dwLength;
        sh_audio->i_bps=(float)asize/(sh_video->frametime*priv->numberofframes);
      }
    }
    vsize=demuxer->movi_end-demuxer->movi_start-asize-8*priv->numberofframes;
    
    #ifdef DEBUG
    demux_avi_printf("AVI video size=%"PRId64" (%u)  audio size=%"PRId64"\n",vsize,priv->numberofframes,asize);
    #endif

    sh_video->i_bps=(float)vsize/(sh_video->frametime*priv->numberofframes);
  }

  return demuxer;
  
}

static int whileSeeking = 0;

void demux_seek_avi(demuxer_t *demuxer,float rel_seek_secs,float audio_delay,int flags){
    avi_priv_t *priv=demuxer->priv;
    demux_stream_t *d_audio=demuxer->audio;
    demux_stream_t *d_video=demuxer->video;
    sh_audio_t *sh_audio=d_audio->sh;
    sh_video_t *sh_video=d_video->sh;
#ifdef DEBUG
    float skip_audio_secs=0;
#endif
    audio_delay=priv->pts_correction;
    
    #ifdef DEBUG
    printf("\n\n seek:%f %f\n\n",rel_seek_secs,audio_delay);
    #endif

  //FIXME: OFF_T - Didn't check AVI case yet (avi files can't be >2G anyway?)
  //================= seek in AVI ==========================
    int rel_seek_frames=rel_seek_secs*sh_video->fps;
    int video_chunk_pos=priv->idx_pos;
    int i;

	whileSeeking = 1;
	getAVIMutex(FILENAME, __FUNCTION__,__LINE__);

     if(flags&SEEK_ABSOLUTE){
	// seek absolute
	video_chunk_pos=0;
      }
      
      if(flags&SEEK_FACTOR){
	rel_seek_frames=rel_seek_secs*priv->numberofframes;
      }
    
      priv->skip_video_frames=0;
      priv->avi_audio_pts=0;

// ------------ STEP 1: find nearest video keyframe chunk ------------
      // find nearest video keyframe chunk pos:
      if(rel_seek_frames>0){
        // seek forward
        while(video_chunk_pos<priv->idx_size-1){
          int id=((AVIINDEXENTRY *)priv->idx)[video_chunk_pos].ckid;
          if(avi_stream_id(id)==d_video->id){  // video frame
            if((--rel_seek_frames)<0 && ((AVIINDEXENTRY *)priv->idx)[video_chunk_pos].dwFlags&AVIIF_KEYFRAME) break;
          }
          ++video_chunk_pos;
        }
      } else {
        // seek backward
        while(video_chunk_pos>0){
          int id=((AVIINDEXENTRY *)priv->idx)[video_chunk_pos].ckid;
          if(avi_stream_id(id)==d_video->id){  // video frame
            if((++rel_seek_frames)>0 && ((AVIINDEXENTRY *)priv->idx)[video_chunk_pos].dwFlags&AVIIF_KEYFRAME) break;
          }
          --video_chunk_pos;
        }
      }
      priv->idx_pos_a=priv->idx_pos_v=priv->idx_pos=video_chunk_pos;
      // re-calc video pts:
      d_video->pack_no=0;
      for(i=0;i<video_chunk_pos;i++){
          int id=((AVIINDEXENTRY *)priv->idx)[i].ckid;
          if(avi_stream_id(id)==d_video->id) ++d_video->pack_no;
      }
      priv->video_pack_no=
      sh_video->num_frames=sh_video->num_frames_decoded=d_video->pack_no;
      priv->avi_video_pts=d_video->pack_no*(float)sh_video->video.dwScale/(float)sh_video->video.dwRate;
      d_video->pos=video_chunk_pos;
      
      #ifdef DEBUG
      demux_avi_printf("V_SEEK:  pack=%d  pts=%5.3f  chunk=%d  \n",d_video->pack_no,priv->avi_video_pts,video_chunk_pos);
      #endif

// ------------ STEP 2: seek audio, find the right chunk & pos ------------

      d_audio->pack_no=0;
      priv->audio_block_no=0;
      d_audio->dpos=0;

      if(sh_audio){
        int i;
        int len=0;
	int skip_audio_bytes=0;
	int curr_audio_pos=-1;
	int audio_chunk_pos=-1;
	int chunk_max=(demuxer->type==DEMUXER_TYPE_AVI)?video_chunk_pos:priv->idx_size;
	
	if(sh_audio->audio.dwSampleSize){
	    // constant rate audio stream
	    /* immediate seeking to audio position, including when streams are delayed */
	    curr_audio_pos=(priv->avi_video_pts + audio_delay)*(float)sh_audio->audio.dwRate/(float)sh_audio->audio.dwScale;
	    curr_audio_pos*=sh_audio->audio.dwSampleSize;

        // find audio chunk pos:
          for(i=0;i<chunk_max;i++){
            int id=((AVIINDEXENTRY *)priv->idx)[i].ckid;
            if(avi_stream_id(id)==d_audio->id){
                len=((AVIINDEXENTRY *)priv->idx)[i].dwChunkLength;
                if(d_audio->dpos<=curr_audio_pos && curr_audio_pos<(d_audio->dpos+len)){
                  break;
                }
                ++d_audio->pack_no;
                priv->audio_block_no+=priv->audio_block_size ?
		    ((len+priv->audio_block_size-1)/priv->audio_block_size) : 1;
                d_audio->dpos+=len;
            }
          }
	  audio_chunk_pos=i;
	  skip_audio_bytes=curr_audio_pos-d_audio->dpos;

	  #ifdef DEBUG
          demux_avi_printf("SEEK: i=%d (max:%d) dpos=%d (wanted:%d)  \n",
	      i,chunk_max,(int)d_audio->dpos,curr_audio_pos);
	  #endif
	      
	} else {
	    // VBR audio
	    /* immediate seeking to audio position, including when streams are delayed */
	    int chunks=(priv->avi_video_pts + audio_delay)*(float)sh_audio->audio.dwRate/(float)sh_audio->audio.dwScale;
	    audio_chunk_pos=0;
	    
        // find audio chunk pos:
          for(i=0;i<priv->idx_size && chunks>0;i++){
            int id=((AVIINDEXENTRY *)priv->idx)[i].ckid;
            if(avi_stream_id(id)==d_audio->id){
                len=((AVIINDEXENTRY *)priv->idx)[i].dwChunkLength;
		if(i>chunk_max){
		  skip_audio_bytes+=len;
		} else {
		  ++d_audio->pack_no;
                  priv->audio_block_no+=priv->audio_block_size ?
		    ((len+priv->audio_block_size-1)/priv->audio_block_size) : 1;
                  d_audio->dpos+=len;
		  audio_chunk_pos=i;
		}
		if(priv->audio_block_size)
		    chunks-=(len+priv->audio_block_size-1)/priv->audio_block_size;
            }
          }
	}
	
	// Now we have:
	//      audio_chunk_pos = chunk no in index table (it's <=chunk_max)
	//      skip_audio_bytes = bytes to be skipped after chunk seek
	//      d-audio->pack_no = chunk_no in stream at audio_chunk_pos
	//      d_audio->dpos = bytepos in stream at audio_chunk_pos
	// let's seek!
	
          // update stream position:
          d_audio->pos=audio_chunk_pos;
	
	if(demuxer->type==DEMUXER_TYPE_AVI){
	  // interleaved stream:
	  if(audio_chunk_pos<video_chunk_pos){
            // calc priv->skip_video_frames & adjust video pts counter:
	    for(i=audio_chunk_pos;i<video_chunk_pos;i++){
              int id=((AVIINDEXENTRY *)priv->idx)[i].ckid;
              if(avi_stream_id(id)==d_video->id) ++priv->skip_video_frames;
            }
            // requires for correct audio pts calculation (demuxer):
            priv->avi_video_pts-=priv->skip_video_frames*(float)sh_video->video.dwScale/(float)sh_video->video.dwRate;
	    priv->avi_audio_pts=priv->avi_video_pts;
	    // set index position:
	    priv->idx_pos_a=priv->idx_pos_v=priv->idx_pos=audio_chunk_pos;
	  }
	} else {
	    // non-interleaved stream:
	    priv->idx_pos_a=audio_chunk_pos;
	    priv->idx_pos_v=video_chunk_pos;
	    priv->idx_pos=(audio_chunk_pos<video_chunk_pos)?audio_chunk_pos:video_chunk_pos;
	}

	  #ifdef DEBUG
          demux_avi_printf("SEEK: idx=%d  (a:%d v:%d)  v.skip=%d  a.skip=%d/%4.3f  \n",
            (int)priv->idx_pos,audio_chunk_pos,video_chunk_pos,
            (int)priv->skip_video_frames,skip_audio_bytes,skip_audio_secs);
	  #endif

          if(skip_audio_bytes){
            demux_read_data(d_audio,NULL,skip_audio_bytes);
          }

      }
	d_video->pts=priv->avi_video_pts; // OSD

	releaseAVIMutex(FILENAME, __FUNCTION__,__LINE__);
	whileSeeking = 0;
}


void demux_close_avi(demuxer_t *demuxer) {
  avi_priv_t* priv=demuxer->priv;

  if(!priv)
    return;

  if(priv->idx_size > 0)
    free(priv->idx);
  free(priv);
}


static int demux_avi_control(demuxer_t *demuxer,int cmd, void *arg){
    #ifdef DEBUG
    demux_avi_printf("%s::%d\n", __FUNCTION__, __LINE__);
    #endif
    
    avi_priv_t *priv=demuxer->priv;
    demux_stream_t *d_video=demuxer->video;
    sh_video_t *sh_video=d_video->sh;

    switch(cmd) {
	case DEMUXER_CTRL_GET_TIME_LENGTH:
    	    if (!priv->numberofframes || !sh_video) return DEMUXER_CTRL_DONTKNOW;
	    *((double *)arg)=(double)priv->numberofframes/sh_video->fps;
	    if (sh_video->video.dwLength<=1) return DEMUXER_CTRL_GUESS;
	    return DEMUXER_CTRL_OK;

	case DEMUXER_CTRL_GET_PERCENT_POS:
    	    if (!priv->numberofframes || !sh_video) {
              return DEMUXER_CTRL_DONTKNOW;
	    }
	    *((int *)arg)=(int)(priv->video_pack_no*100/priv->numberofframes);
	    if (sh_video->video.dwLength<=1) return DEMUXER_CTRL_GUESS;
	    return DEMUXER_CTRL_OK;

	case DEMUXER_CTRL_SWITCH_AUDIO:
	case DEMUXER_CTRL_SWITCH_VIDEO: {
	    int audio = (cmd == DEMUXER_CTRL_SWITCH_AUDIO);
	    demux_stream_t *ds = audio ? demuxer->audio : demuxer->video;
	    void **streams = audio ? demuxer->a_streams : demuxer->v_streams;
	    int maxid = FFMIN(100, audio ? MAX_A_STREAMS : MAX_V_STREAMS);
	    int chunkid;
	    if (ds->id < -1)
	      return DEMUXER_CTRL_NOTIMPL;

	    if (*(int *)arg >= 0)
	      ds->id = *(int *)arg;
	    else {
	      int i;
	      for (i = 0; i < maxid; i++) {
	        if (++ds->id >= maxid) ds->id = 0;
	        if (streams[ds->id]) break;
	      }
	    }

	    chunkid = (ds->id / 10 + '0') | (ds->id % 10 + '0') << 8;
	    ds->sh = NULL;
	    if (!streams[ds->id]) // stream not available
	      ds->id = -1;
	    else
	      demux_avi_select_stream(demuxer, chunkid);
	    *(int *)arg = ds->id;
	    return DEMUXER_CTRL_OK;
	}

	default:
	    return DEMUXER_CTRL_NOTIMPL;
    }
}


static int avi_check_file(demuxer_t *demuxer)
{
  #ifdef DEBUG
  printf("log demux_avi 13\n");
  #endif
  
  int id=stream_read_dword_le(demuxer->stream); // "RIFF"

  if((id==mmioFOURCC('R','I','F','F')) || (id==mmioFOURCC('O','N','2',' '))) {
    stream_read_dword_le(demuxer->stream); //filesize
    id=stream_read_dword_le(demuxer->stream); // "AVI "
    if(id==formtypeAVI)
      return DEMUXER_TYPE_AVI;
    // "Samsung Digimax i6 PMP" crap according to bug 742
    if(id==mmioFOURCC('A','V','I',0x19))
      return DEMUXER_TYPE_AVI;
    if(id==mmioFOURCC('O','N','2','f')){
      #ifdef DEBUG
      demux_avi_printf("ON2AviFormat\n");
      #endif
      
      return DEMUXER_TYPE_AVI;
    }
  }

  return 0;
}


static demuxer_t* demux_open_hack_avi(demuxer_t *demuxer)
{
  #ifdef DEBUG
  printf("log demux_avi 14\n");
  #endif
  
   sh_audio_t* sh_a;

   demuxer = demux_open_avi(demuxer);
   if(!demuxer) return NULL; // failed to open
   sh_a = demuxer->audio->sh;
   if(demuxer->audio->id != -2 && sh_a) {
#ifdef CONFIG_OGGVORBIS
    // support for Ogg-in-AVI:
    if(sh_a->format == 0xFFFE)
      demuxer = init_avi_with_ogg(demuxer);
    else if(sh_a->format == 0x674F) {
      demuxer_t  *od;
      demuxer->stream = new_ds_stream(demuxer->audio);
      od = new_demuxer(demuxer->stream,DEMUXER_TYPE_OGG,-1,-2,-2,NULL);
      if(!demux_ogg_open(od)) {
	#ifdef DEBUG
        demux_avi_printf("ErrorOpeningOGGDemuxer\n");
	#endif
	
        free_stream(demuxer->stream);
        demuxer->audio->id = -2;
      } else
        demuxer = new_demuxers_demuxer(demuxer,od,demuxer);
   }
#endif
   }

   return demuxer;
}
////////////////////////////////////////////////////////////////7
////////////////////////////////////////////////////////////////7

#include "common.h"
#include "container.h"
#include "manager.h"
#include "utils.h"

static demuxer_t *demuxer = NULL;
static demux_stream_t *ds = NULL;   // dvd subtitle buffer/demuxer
static sh_audio_t *sh_audio = NULL;
static sh_video_t *sh_video = NULL;
static pthread_t PlayThread;
static int hasPlayThreadStarted = 0;

static int isFirstAudioFrame = 1;

demuxer_desc_t demuxer_desc_avi = {
  "AVI demuxer",
  "avi",
  "AVI",
  "Arpi?",
  "AVI files, including non interleaved files",
  DEMUXER_TYPE_AVI,
  1, // safe autodetect
  avi_check_file,
  demux_avi_fill_buffer,
  demux_open_avi,
  demux_close_avi,
  demux_seek_avi,
  demux_avi_control
};

//Trick: VID-3F.avi geht nicht!
//	 VID-3I.avi 720p stottert!
//	 VID-3J.avi 1080p not support!
int AviInit(Context_t *context, char * filename) {
	#ifdef DEBUG
	printf("%s::%s\n", FILENAME, __FUNCTION__);
	#endif

	getAVIMutex(FILENAME, __FUNCTION__,__LINE__);

	int ret = 0;
	int i = 0;
	int video_format = 0;
	demuxer = (demuxer_t*)malloc ( sizeof(demuxer_t));
	memset (demuxer,0,sizeof(demuxer_t));

	demuxer->audio = (demux_stream_t*)malloc ( sizeof(demux_stream_t));
	memset (demuxer->audio,0,sizeof(demux_stream_t));

	demuxer->video = (demux_stream_t*)malloc ( sizeof(demux_stream_t));
	memset (demuxer->video,0,sizeof(demux_stream_t));

	demuxer->sub = (demux_stream_t*)malloc ( sizeof(demux_stream_t));
	memset (demuxer->sub,0,sizeof(demux_stream_t));

	ds = (demux_stream_t*)malloc ( sizeof(demux_stream_t));
	memset (ds,0,sizeof(demux_stream_t));

	#ifdef DEBUG
	printf("%s::%d\n", __FUNCTION__, __LINE__);
	#endif

	demuxer->stream = (stream_t*)malloc ( sizeof(stream_t));
	memset (demuxer->stream,0,sizeof(stream_t));
/*    demuxer->stream = (demux_stream_t*)malloc ( sizeof(demux_stream_t));
    memset (demuxer->stream,0,sizeof(demux_stream_t));*/

	#ifdef DEBUG
	printf("%s::%d\n", __FUNCTION__, __LINE__);
	#endif

	demuxer->stream->fd = context->playback->fd;

	read(demuxer->stream->fd,demuxer->stream->buffer,2048);//soviel ??

	#ifdef DEBUG
	printf("%s::%d\n", __FUNCTION__, __LINE__);
	#endif

    demuxer->desc = &demuxer_desc_avi;
    demuxer->audio->id = -1;
    demuxer->video->id = -1;
    //demuxer->video_play = 0;
    demuxer->stream->start_pos	= 0;

    if(context->playback->isFile) {
    	demuxer->stream->type		= STREAMTYPE_FILE;
    	long pos = lseek(demuxer->stream->fd, 0L, SEEK_CUR);
    	demuxer->stream->end_pos = lseek(demuxer->stream->fd, 0L, SEEK_END);
    	lseek(demuxer->stream->fd, pos, SEEK_SET);
    } else {
	demuxer->stream->type		= STREAMTYPE_STREAM;
	demuxer->stream->end_pos = 0;
    }

    demuxer->stream->flags		= 6;
    demuxer->stream->sector_size	= 0;
    demuxer->stream->buf_pos		= 0;
    demuxer->stream->buf_len		= 2048;
    demuxer->stream->pos		= 2048;
    demuxer->stream->start_pos		= 0;
    demuxer->stream->eof		= 0;
    demuxer->stream->cache_pid		= 0;

    ret=avi_check_file(demuxer);
    if(ret == DEMUXER_TYPE_AVI)
    {
	#ifdef DEBUG
	printf("%s::%d\n", __FUNCTION__, __LINE__);
	#endif

	demux_open_hack_avi(demuxer);

	if(demuxer->audio->sh == NULL){
		#ifdef DEBUG
		printf("%s::%d\n", __FUNCTION__, __LINE__);
		#endif

		demux_flush(demuxer);
		demuxer->audio->id = -1;
		demuxer->video->id = -1;
		//demuxer->video_play = 0;
		demuxer->stream->start_pos	= 0;
                		
		if(context->playback->isFile)
                {
		     demuxer->stream->type		= STREAMTYPE_FILE;
		     lseek(demuxer->stream->fd, 0L, SEEK_SET);
                }
		else
		{
		     demuxer->stream->type		= STREAMTYPE_STREAM;
		}
		
		read(demuxer->stream->fd,demuxer->stream->buffer,2048);//soviel ??
		demuxer->stream->flags		= 6;
		demuxer->stream->sector_size	= 0;
		demuxer->stream->buf_pos	= 0;
		demuxer->stream->buf_len	= 2048;
		demuxer->stream->pos		= 2048;
		demuxer->stream->start_pos	= 0;
		demuxer->stream->eof		= 0;
		demuxer->stream->cache_pid	= 0;
		demux_open_hack_avi(demuxer);

		#ifdef DEBUG
		printf("%s::%d\n", __FUNCTION__, __LINE__);
		#endif
	}
	if(demuxer->audio->sh != NULL)
	{
		sh_audio=demuxer->audio->sh;

		#ifdef DEBUG
		printf("AUDIO 0x%02x\n",sh_audio->format);
		#endif

		for(i= 0;i<MAX_A_STREAMS;i++){
			if(demuxer->a_streams[i]==NULL)continue;
			if(((sh_audio_t *)demuxer->a_streams[i])->format == 0x55) //mp3
			{
					Track_t Audio = {
						"und",
						"A_MPEG/L3",
						((sh_audio_t *)demuxer->a_streams[i])->aid,
					};
					context->manager->audio->Command(context, MANAGER_ADD, &Audio);
			} else if(((sh_audio_t *)demuxer->a_streams[i])->format == 0x2000) //ac3
			{
					Track_t Audio = {
						"und",
						"A_AC3",
						((sh_audio_t *)demuxer->a_streams[i])->aid,
					};
					context->manager->audio->Command(context, MANAGER_ADD, &Audio);
			} else if(((sh_audio_t *)demuxer->a_streams[i])->format == 0x2001) //dts
			{
					Track_t Audio = {
						"und",
						"A_DTS",
						((sh_audio_t *)demuxer->a_streams[i])->aid,
					};
					context->manager->audio->Command(context, MANAGER_ADD, &Audio);
			}
		}
		
	}
	if(demuxer->video->sh != NULL){
		sh_video=demuxer->video->sh;

		#ifdef DEBUG
		printf("VIDEO 0x%02x\n",sh_video->format);
		#endif

		video_format=le2me_32(sh_video->bih->biCompression);

		#ifdef DEBUG
		printf("VIDEO:  [%.4s] 0x%02x\n",(char *)&video_format,video_format);
		#endif

		if(video_format==0x3167706d)
		{
			#ifdef DEBUG
			printf("VIDEOFORMAT = MPEG1 not support\n");
			#endif

			/*printf("VIDEOFORMAT = MPEG1\n");
			Track_t Video = {
				"und",
				"V_MPEG2",
				0,
			};

			context->manager->video->Command(context, MANAGER_ADD, &Video);*/
		}
		else if(video_format==0x33564944) // xvid: ob diese nummern alle in avi gibt ??
		{
			#ifdef DEBUG
			printf("VIDEOFORMAT = DIVX3 not support\n");
			#endif
		}
		else if(video_format==0x44495658 || video_format==0x30355844 || video_format==0x3334504D || video_format==0x3334706D || video_format==0x35766964
		     || video_format==0x36564944 || video_format==0x36766964 || video_format==0x31345041 || video_format==0x58564944 || video_format==0x78766964 || video_format==0x64697678)
		{
			#ifdef DEBUG
			printf("VIDEOFORMAT = DIVX4/5 or XVID\n");
			#endif

			Track_t Video = {
				"und",
				"V_MS/VFW/FOURCC",
				0,
			};

			context->manager->video->Command(context, MANAGER_ADD, &Video);
		}
		else if(video_format == 0x34363248 || video_format == 0x34363268) // h264 !!bei avi geht h264 über V_MPEG2!!
		{
			#ifdef DEBUG
			printf("VIDEOFORMAT = H264\n");
			#endif

			Track_t Video = {
				"und",
				"V_MPEG2/H264",
				0,
			};

			context->manager->video->Command(context, MANAGER_ADD, &Video);
		}
	}

		/*} else if (d->tracks[i] != NULL && d->tracks[i]->type == MATROSKA_TRACK_SUBTITLE) {
			Track_t Subtitle = {
				d->tracks[i]->language,
				d->tracks[i]->codec_id,
				i,
			};
			context->manager->subtitle->Command(context, MANAGER_ADD, &Subtitle);
		}
	}*/

	//select fist subtitle track as default:
	/*int SubTrackID = 0;
	context->manager->subtitle->Command(context, MANAGER_GET, &SubTrackID);
	if (SubTrackID > 0)
		demuxer->sub->id = mkv_d->tracks[SubTrackID]->tnum;*/

    }
	releaseAVIMutex(FILENAME, __FUNCTION__,__LINE__);
    
	return 0; // FIXME
}
void HexdumpAVI(unsigned char *Data, int length)
{
	#ifdef DEBUG
	int k;
	for (k = 0; k < length; k++)
	{
		printf("%02x ", Data[k]);    
		if (((k+1)&31)==0)
			printf("\n");
	}

	printf("\n");
	#endif
}
#define INVALID_PTS_VALUE                       0x200000000ull
void AviGenerateParcel(Context_t *context, const demuxer_t *demuxer) {
	const demux_stream_t * video = demuxer->video;
	const demux_stream_t * audio = demuxer->audio;
	unsigned long long int Pts = 0;

	 if (audio->first != NULL) {
		demux_packet_t * current = audio->first;
		if (!(current->flags&0x1)) {  //current frame isn't a keyframe
			//printf("\tNORMALFRAME,                 ");
			Pts = INVALID_PTS_VALUE;
		} else {
			//printf("\tKEYFRAME,                    ");
			Pts = (current->pts * 90000);
		}
		//printf("a current->pts=%f Pts=%llu\n", current->pts, Pts);
		while (current != NULL) {
			//printf("AUDIODATA\n");
			//HexdumpAVI(current->buffer, current->len);
			context->output->audio->Write(context, current->buffer, current->len, Pts, NULL, 0, 0, "audio");
			
			isFirstAudioFrame=0;
			current = current->next;
			Pts = INVALID_PTS_VALUE;
		}
	}

    if (video->first != NULL) {
		demux_packet_t * current = video->first;
		if (!(current->flags&0x1)) {  //current frame isn't a keyframe
			//printf("\tNORMALFRAME,                 ");
			Pts = INVALID_PTS_VALUE;
		} else {
			//printf("\tKEYFRAME,                    ");
			Pts = (current->pts * 90000);
		}

		//printf("v current->pts=%f Pts=%llu\n", current->pts, Pts);
		MainAVIHeader aviHeader = getAVIHeader();
		while (current != NULL) {
			context->output->video->Write(context, current->buffer, current->len, Pts, 0, 0, aviHeader.dwMicroSecPerFrame, "video");

			current = current->next;
			Pts = INVALID_PTS_VALUE;
		}
	}
}



static void AviThread(Context_t *context) {
	#ifdef DEBUG
	printf("%s::%s\n", FILENAME, __FUNCTION__);
	#endif
	
	while ( context && context->playback && context->playback->isCreationPhase ) {
		#ifdef DEBUG
		printf("%s::%s Thread waiting for end of init phase...\n", FILENAME, __FUNCTION__);
		#endif
	}

	#ifdef DEBUG
	printf("%s::%s Running!\n", FILENAME, __FUNCTION__);
	#endif

	isFirstAudioFrame=1;
	
	while ( context && context->playback && context->playback->isPlaying ) {
	    //printf("%s -->\n", __FUNCTION__);

		//IF AUDIO IS PAUSED, WAIT
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

		getAVIMutex(FILENAME, __FUNCTION__,__LINE__);

		if ( !demux_avi_fill_buffer(demuxer,ds) ) {
			#ifdef DEBUG
			printf("%s::%s demux_avi_fill_buffer failed!\n", FILENAME, __FUNCTION__);
			#endif

			releaseAVIMutex(FILENAME, __FUNCTION__,__LINE__);
			
			break;
		} else {		
			AviGenerateParcel(context, demuxer);

			if (demuxer->sub != NULL && demuxer->sub->first != NULL) {
				ds_free_packs(demuxer->sub);
			}
			
			if (demuxer->audio != NULL && demuxer->audio->first != NULL) {
				ds_free_packs(demuxer->audio);
			}
			
			if (demuxer->video != NULL && demuxer->video->first != NULL) {
				ds_free_packs(demuxer->video);
			}
			//printf("%s <--\n", __FUNCTION__);

			releaseAVIMutex(FILENAME, __FUNCTION__,__LINE__);
		}
	}

        hasPlayThreadStarted = 0;

	context->playback->Command(context, PLAYBACK_TERM, NULL);

	#ifdef DEBUG
	printf("%s::%s terminating\n",FILENAME, __FUNCTION__);
	#endif
}


static int AviPlay(Context_t *context) {
	#ifdef DEBUG
	printf("%s::%s\n", FILENAME, __FUNCTION__);
	#endif

	int error;
	int ret = 0;
	pthread_attr_t attr;

	if ( context && context->playback && context->playback->isPlaying ) {
		#ifdef DEBUG
		printf("%s::%s is Playing\n", FILENAME, __FUNCTION__);
		#endif
	} else {
		#ifdef DEBUG
		printf("%s::%s is NOT Playing\n", FILENAME, __FUNCTION__);
		#endif
	}
	
	if (hasPlayThreadStarted == 0) {
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

		if((error=pthread_create(&PlayThread, &attr, (void *)&AviThread, context)) != 0) {
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

static int AviStop(Context_t *context) {
	#ifdef DEBUG
	printf("%s::%s\n", FILENAME, __FUNCTION__);
	#endif

	int i;
	int ret = 0;
	int wait_time = 20;

	while ( (hasPlayThreadStarted != 0) && (--wait_time) > 0 ) {
		#ifdef DEBUG
		printf("%s::%s Waiting for AVI thread to terminate itself, will try another %d times\n", FILENAME, __FUNCTION__, wait_time);
		#endif

		usleep(100000);
	}

	if (wait_time == 0) {
		#ifdef DEBUG
		printf("%s::%s Timeout waiting for AVI thread!\n", FILENAME, __FUNCTION__);
		#endif

		ret = -1;
	} else {
		getAVIMutex(FILENAME, __FUNCTION__,__LINE__);
		
		if (demuxer) {		  
			demux_close_avi(demuxer);

			free(demuxer->stream);
			demuxer->stream = NULL;

			free(demuxer->sub);
			demuxer->sub = NULL;

			free(demuxer->video);
			demuxer->video = NULL;
		
			free(demuxer->audio);
			demuxer->audio = NULL;
			
			for(i=0;i<MAX_A_STREAMS;i++){
				free(demuxer->a_streams[i]);
				demuxer->a_streams[i]=NULL;
			}
			
			for(i=0;i<MAX_V_STREAMS;i++){
				free(demuxer->v_streams[i]);
				demuxer->v_streams[i]=NULL;
			}
			
			for(i=0;i<MAX_S_STREAMS;i++){
				free(demuxer->s_streams[i]);
				demuxer->s_streams[i]=NULL;
			}
			
			free(demuxer);   
			demuxer = NULL;
		}

		free(ds);
		ds = NULL;

		releaseAVIMutex(FILENAME, __FUNCTION__,__LINE__);
	}

	#ifdef DEBUG
	printf("%s::%s exiting with %d\n", FILENAME, __FUNCTION__, ret);
	#endif

	return ret;    
}

static int AviGetLength(demuxer_t *demuxer,double * length) {

	if (demuxer->priv) {
	    avi_priv_t* priv=demuxer->priv;
	      
	    if (priv->duration == 0)
		    return -1;

	    *((double *)length) = (double)priv->duration;
	} else
        	return -1;

	return 0;
}

static int AviSwitchAudio(demuxer_t *demuxer, int* arg) {
	#ifdef DEBUG
	printf("%s::%s\n", FILENAME, __FUNCTION__);
	#endif

	getAVIMutex(FILENAME, __FUNCTION__,__LINE__);
	
	if (demuxer && demuxer->priv) {

		//mkv_demuxer_t *mkv_d = (mkv_demuxer_t *) demuxer->priv;

		sh_audio_t *sh = demuxer->a_streams[demuxer->audio->id];
		int aid = *(int*)arg;
		/*if (aid < 0)
		aid = (sh->aid + 1) % mkv_d->last_aid;
		if (aid != sh->aid) */{

			#ifdef DEBUG
		    	printf("%s::%s track %d\n", FILENAME, __FUNCTION__,aid);
			#endif

		    	demuxer->audio->id = aid;
		    	sh = demuxer->a_streams[demuxer->audio->id];
		    	ds_free_packs(demuxer->audio);
		}
	    // *(int*)arg = sh->aid;
	} //else
	    // *(int*)arg = -2;
	
	releaseAVIMutex(FILENAME, __FUNCTION__,__LINE__);
	
    return 0;
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
			ret = AviInit(context, FILENAME);
			break;
		}
		case CONTAINER_PLAY: {
			ret = AviPlay(context);
			break;
		}
		case CONTAINER_STOP: {
			ret = AviStop(context);
			break;
		}
		case CONTAINER_SEEK: {
			demux_seek_avi (demuxer, (float)*((float*)argument), 0.0, 0);
			break;
		}
		case CONTAINER_LENGTH: {
			double length = 0;
			if (demuxer != NULL && AviGetLength(demuxer, &length)!=0)
				*((double*)argument) = (double)0;
			else
				*((double*)argument) = (double)length;
			break;
		}
		case CONTAINER_SWITCH_AUDIO: {
           		if (demuxer)
			    ret = AviSwitchAudio(demuxer, (int*) argument);
			break;
		}
		default: {
			#ifdef DEBUG
			printf("%s::%s ContainerCmd %d not supported!\n", FILENAME, __FUNCTION__, command);
			#endif

			break;
		}
	}

	#ifdef DEBUG
	printf("%s::%s exiting with value %d\n", FILENAME, __FUNCTION__, ret);
	#endif

	return ret;
}

static char *AviCapabilities[] = { "avi", NULL };

Container_t AviContainer = {
	"AVI",
	&Command,
	AviCapabilities,

};
