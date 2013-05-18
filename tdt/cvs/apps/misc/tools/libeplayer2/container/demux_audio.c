
//#include "../config.h"

#include <stdlib.h>
#include <stdio.h>
#include "stream.h"
#include "demuxer.h"
#include "stheader.h"
#include "genres.h"
#include "mp3_hdr.h"
#include "intreadwrite.h"

#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>

int audio_stop = 0;

#ifdef DEBUG
int demux_audio_debug = 0;
#define demux_audio_printf(x...) do { if (demux_audio_debug)printf(x); } while (0)
#endif

#define MP3 1
#define WAV 2
#define fLaC 3


#define HDR_SIZE 4
static const char FILENAME[] = "demux_audio.c";

typedef struct da_priv {
  int frmt;
  double next_pts;
} da_priv_t;

//! rather arbitrary value for maximum length of wav-format headers
#define MAX_WAVHDR_LEN (1 * 1024 * 1024)

//! how many valid frames in a row we need before accepting as valid MP3
#define MIN_MP3_HDRS 12

//! Used to describe a potential (chain of) MP3 headers we found
typedef struct mp3_hdr {
  off_t frame_pos; // start of first frame in this "chain" of headers
  off_t next_frame_pos; // here we expect the next header with same parameters
  int mp3_chans;
  int mp3_freq;
  int mpa_spf;
  int mpa_layer;
  int mpa_br;
  int cons_hdrs; // if this reaches MIN_MP3_HDRS we accept as MP3 file
  struct mp3_hdr *next;
} mp3_hdr_t;

void print_wave_header(WAVEFORMATEX *h, int verbose_level);

int hr_mp3_seek = 0;

pthread_mutex_t Audiomutex;

void getAudioMutex(const char *filename, const char *function, int line) {	// FIXME: Use one central getMutex and pass the mutex into the function
	#ifdef DEBUG
	demux_audio_printf("%s::%s::%d requesting mutex\n",filename, function, line);
	#endif
	
	pthread_mutex_lock(&Audiomutex);
	
	#ifdef DEBUG
	demux_audio_printf("%s::%s::%d received mutex\n",filename, function, line);
	#endif  
}

void releaseAudioMutex(const char *filename, const char *function, int line) {	// FIXME: Use one central getMutex and pass the mutex into the function
	pthread_mutex_unlock(&Audiomutex);
	
	#ifdef DEBUG
	demux_audio_printf("%s::%s::%d released mutex\n",filename, function, line);
	#endif  
}

/**
 * \brief free a list of MP3 header descriptions
 * \param list pointer to the head-of-list pointer
 */
static void free_mp3_hdrs(mp3_hdr_t **list) {
  mp3_hdr_t *tmp;
  while (*list) {
    tmp = (*list)->next;
    free(*list);
    *list=NULL;
    *list = tmp;
  }
}

/**
 * \brief add another potential MP3 header to our list
 * If it fits into an existing chain this one is expanded otherwise
 * a new one is created.
 * All entries that expected a MP3 header before the current position
 * are discarded.
 * The list is expected to be and will be kept sorted by next_frame_pos
 * and when those are equal by frame_pos.
 * \param list pointer to the head-of-list pointer
 * \param st_pos stream position where the described header starts
 * \param mp3_chans number of channels as specified by the header (*)
 * \param mp3_freq sampling frequency as specified by the header (*)
 * \param mpa_spf frame size as specified by the header
 * \param mpa_layer layer type ("version") as specified by the header (*)
 * \param mpa_br bitrate as specified by the header
 * \param mp3_flen length of the frame as specified by the header
 * \return If non-null the current file is accepted as MP3 and the
 * mp3_hdr struct describing the valid chain is returned. Must be
 * freed independent of the list.
 *
 * parameters marked by (*) must be the same for all headers in the same chain
 */
static mp3_hdr_t *add_mp3_hdr(mp3_hdr_t **list, off_t st_pos,
                               int mp3_chans, int mp3_freq, int mpa_spf,
                               int mpa_layer, int mpa_br, int mp3_flen) {
  mp3_hdr_t *tmp;
  int in_list = 0;
  while (*list && (*list)->next_frame_pos <= st_pos) {
    
    #ifdef DEBUG  
    printf("%s::%d\n", __FUNCTION__, __LINE__);
    #endif
  
    if (((*list)->next_frame_pos < st_pos) || ((*list)->mp3_chans != mp3_chans)
         || ((*list)->mp3_freq != mp3_freq) || ((*list)->mpa_layer != mpa_layer) ) {
      // wasn't valid!
      tmp = (*list)->next;
      free(*list);
      *list = NULL;
      *list = tmp;
    } else {
      (*list)->cons_hdrs++;
      (*list)->next_frame_pos = st_pos + mp3_flen;
      (*list)->mpa_spf = mpa_spf;
      (*list)->mpa_br = mpa_br;
      if ((*list)->cons_hdrs >= MIN_MP3_HDRS) {
        // copy the valid entry, so that the list can be easily freed
        tmp = malloc(sizeof(mp3_hdr_t));
        memcpy(tmp, *list, sizeof(mp3_hdr_t));
        tmp->next = NULL;
        return tmp;
      }
      in_list = 1;
      list = &((*list)->next);
    }
  }

  #ifdef DEBUG  
  printf("%s::%d\n", __FUNCTION__, __LINE__);
  #endif
  
  if (!in_list) { // does not belong into an existing chain, insert
    // find right position to insert to keep sorting
    while (*list && (*list)->next_frame_pos <= st_pos + mp3_flen)
      list = &((*list)->next);
    tmp = malloc(sizeof(mp3_hdr_t));
    tmp->frame_pos = st_pos;
    tmp->next_frame_pos = st_pos + mp3_flen;
    tmp->mp3_chans = mp3_chans;
    tmp->mp3_freq = mp3_freq;
    tmp->mpa_spf = mpa_spf;
    tmp->mpa_layer = mpa_layer;
    tmp->mpa_br = mpa_br;
    tmp->cons_hdrs = 1;
    tmp->next = *list;
    *list = tmp;
    
    tmp = malloc(sizeof(mp3_hdr_t));
    memcpy(tmp, *list, sizeof(mp3_hdr_t));
    tmp->next = NULL;
    return tmp;
  }
  return NULL;
}

#if 0 /* this code is a mess, clean it up before reenabling */
#define FLAC_SIGNATURE_SIZE 4
#define FLAC_STREAMINFO_SIZE 34
#define FLAC_SEEKPOINT_SIZE 18

enum {
  FLAC_STREAMINFO = 0,
  FLAC_PADDING,
  FLAC_APPLICATION,
  FLAC_SEEKTABLE,
  FLAC_VORBIS_COMMENT,
  FLAC_CUESHEET
} flac_preamble_t;

static void
get_flac_metadata (demuxer_t* demuxer)
{
  uint8_t preamble[4];
  unsigned int blk_len;
  stream_t *s = demuxer->stream;
  
  /* file is qualified; skip over the signature bytes in the stream */
  stream_seek (s, 4);

  /* loop through the metadata blocks; use a do-while construct since there
   * will always be 1 metadata block */
  do {
    int r;
    
    r = stream_read (s, (char *) preamble, FLAC_SIGNATURE_SIZE);
    if (r != FLAC_SIGNATURE_SIZE)
      return;

    blk_len = AV_RB24(preamble + 1);

    switch (preamble[0] & 0x7F)
    {
    case FLAC_VORBIS_COMMENT:
    {
      /* For a description of the format please have a look at */
      /* http://www.xiph.org/vorbis/doc/v-comment.html */

      uint32_t length, comment_list_len;
      char comments[blk_len];
      uint8_t *ptr = comments;
      char *comment;
      int cn;
      char c;

      if (stream_read (s, comments, blk_len) == blk_len)
      {
        length = AV_RL32(ptr);
        ptr += 4 + length;

        comment_list_len = AV_RL32(ptr);
        ptr += 4;

        cn = 0;
        for (; cn < comment_list_len; cn++)
        {
          length = AV_RL32(ptr);
          ptr += 4;

          comment = ptr;
          if (&comment[length] < comments || &comment[length] >= &comments[blk_len])
            return;
          c = comment[length];
          comment[length] = 0;

          if (!strncasecmp ("TITLE=", comment, 6) && (length - 6 > 0))
            demux_info_add (demuxer, "Title", comment + 6);
          else if (!strncasecmp ("ARTIST=", comment, 7) && (length - 7 > 0))
            demux_info_add (demuxer, "Artist", comment + 7);
          else if (!strncasecmp ("ALBUM=", comment, 6) && (length - 6 > 0))
            demux_info_add (demuxer, "Album", comment + 6);
          else if (!strncasecmp ("DATE=", comment, 5) && (length - 5 > 0))
            demux_info_add (demuxer, "Year", comment + 5);
          else if (!strncasecmp ("GENRE=", comment, 6) && (length - 6 > 0))
            demux_info_add (demuxer, "Genre", comment + 6);
          else if (!strncasecmp ("Comment=", comment, 8) && (length - 8 > 0))
            demux_info_add (demuxer, "Comment", comment + 8);
          else if (!strncasecmp ("TRACKNUMBER=", comment, 12)
                   && (length - 12 > 0))
          {
            char buf[31];
            buf[30] = '\0';
            sprintf (buf, "%d", atoi (comment + 12));
            demux_info_add(demuxer, "Track", buf);
          }
          comment[length] = c;

          ptr += length;
        }
      }
      break;
    }

    case FLAC_STREAMINFO:
    case FLAC_PADDING:
    case FLAC_APPLICATION:
    case FLAC_SEEKTABLE:
    case FLAC_CUESHEET:
    default: 
      /* 6-127 are presently reserved */
      stream_skip (s, blk_len);
      break;
    }
  } while ((preamble[0] & 0x80) == 0);
}
#endif

static int demux_audio_open(demuxer_t* demuxer) {
  stream_t *s;
  sh_audio_t* sh_audio;
  uint8_t hdr[HDR_SIZE];
  int frmt = 0, n = 0, step;
  off_t st_pos = 0, next_frame_pos = 0;
  // mp3_hdrs list is sorted first by next_frame_pos and then by frame_pos
  mp3_hdr_t *mp3_hdrs = NULL, *mp3_found = NULL;
  da_priv_t* priv;
  
  s = demuxer->stream;

  stream_read(s, hdr, HDR_SIZE);

  while(n < 30000 && !s->eof) {
  	//printf("%s::%d n=%d eof=%d %4s\n", __FUNCTION__, __LINE__, n,s->eof, hdr);
    int mp3_freq, mp3_chans, mp3_flen, mpa_layer, mpa_spf, mpa_br;
    st_pos = stream_tell(s) - HDR_SIZE;
    
    #ifdef DEBUG  
    printf("%s::%d st_pos=%u\n", __FUNCTION__, __LINE__, st_pos);
    #endif
    
    step = 1;

    if( hdr[0] == 'R' && hdr[1] == 'I' && hdr[2] == 'F' && hdr[3] == 'F' ) {
      stream_skip(s,4);
      if(s->eof)
	break;
      stream_read(s,hdr,4);
      if(s->eof)
	break;
      if(hdr[0] != 'W' || hdr[1] != 'A' || hdr[2] != 'V'  || hdr[3] != 'E' )
	stream_skip(s,-8);
      else
      // We found wav header. Now we can have 'fmt ' or a mp3 header
      // empty the buffer
	step = 4;
    } else if( hdr[0] == 'I' && hdr[1] == 'D' && hdr[2] == '3' && (hdr[3] >= 2)) {
      int len;
      stream_skip(s,2);
      stream_read(s,hdr,4);
      len = (hdr[0]<<21) | (hdr[1]<<14) | (hdr[2]<<7) | hdr[3];

      stream_skip(s,len);
      step = 4;
    } else if( hdr[0] == 'f' && hdr[1] == 'm' && hdr[2] == 't' && hdr[3] == ' ' ) {
      frmt = WAV;
      break;      
    } else if((mp3_flen = mp_get_mp3_header(hdr, &mp3_chans, &mp3_freq,
                                &mpa_spf, &mpa_layer, &mpa_br)) > 0) {
      mp3_found = add_mp3_hdr(&mp3_hdrs, st_pos, mp3_chans, mp3_freq,
                              mpa_spf, mpa_layer, mpa_br, mp3_flen);
      if (mp3_found) {
        frmt = MP3;
        break;
      }
    } else if( hdr[0] == 'f' && hdr[1] == 'L' && hdr[2] == 'a' && hdr[3] == 'C' ) {
      frmt = fLaC;
      if (!mp3_hdrs || mp3_hdrs->cons_hdrs < 3)
        break;
    }
    // Add here some other audio format detection
    if(step < HDR_SIZE)
      memmove(hdr,&hdr[step],HDR_SIZE-step);
    stream_read(s, &hdr[HDR_SIZE - step], step);
    n++;
  }

free_mp3_hdrs(&mp3_hdrs);

  if(!frmt)
    return 0;

  sh_audio = new_sh_audio(demuxer,0);

  switch(frmt) {
  case MP3:
    sh_audio->format = (mp3_found->mpa_layer < 3 ? 0x50 : 0x55);
    demuxer->movi_start = mp3_found->frame_pos;
    next_frame_pos = mp3_found->next_frame_pos;
    sh_audio->audio.dwSampleSize= 0;
    sh_audio->audio.dwScale = mp3_found->mpa_spf;
    sh_audio->audio.dwRate = mp3_found->mp3_freq;
    sh_audio->wf = malloc(sizeof(WAVEFORMATEX));
    sh_audio->wf->wFormatTag = sh_audio->format;
    sh_audio->wf->nChannels = mp3_found->mp3_chans;
    sh_audio->wf->nSamplesPerSec = mp3_found->mp3_freq;
    sh_audio->wf->nAvgBytesPerSec = mp3_found->mpa_br * (1000 / 8);
    sh_audio->wf->nBlockAlign = mp3_found->mpa_spf;
    sh_audio->wf->wBitsPerSample = 16;
    sh_audio->wf->cbSize = 0;    
    sh_audio->i_bps = sh_audio->wf->nAvgBytesPerSec;
    free(mp3_found);
    mp3_found = NULL;
    
    #ifdef DEBUG  
    printf("%s::%d s->end_pos = %d\n", __FUNCTION__, __LINE__, s->end_pos);
    #endif
    
    if(s->end_pos && (s->flags & STREAM_SEEK) == STREAM_SEEK) {
      char tag[4];
      stream_seek(s,s->end_pos-128);
      stream_read(s,(unsigned char*) tag,3);
      tag[3] = '\0';
      if(strcmp(tag,"TAG")) {
		demuxer->movi_end = s->end_pos;
		
		#ifdef DEBUG  
		printf("demuxer->movi_end = s->end_pos = %d\n", demuxer->movi_end);
		#endif
      } else {
		char buf[31];
		uint8_t g;
		demuxer->movi_end = stream_tell(s)-3;
		
		#ifdef DEBUG  
		printf("demuxer->movi_end = stream_tell(s)-3; = %d\n", demuxer->movi_end);
		#endif
		
		stream_read(s,(unsigned char*) buf,30);
		buf[30] = '\0';
		demux_info_add(demuxer,"Title",buf);
		stream_read(s,(unsigned char*) buf,30);
		buf[30] = '\0';
		demux_info_add(demuxer,"Artist",buf);
		stream_read(s,(unsigned char*) buf,30);
		buf[30] = '\0';
		demux_info_add(demuxer,"Album",buf);
		stream_read(s,(unsigned char*) buf,4);
		buf[4] = '\0';
		demux_info_add(demuxer,"Year",buf);
		stream_read(s,(unsigned char*) buf,30);
		buf[30] = '\0';
		demux_info_add(demuxer,"Comment",buf);
		
		if(buf[28] == 0 && buf[29] != 0) {
			uint8_t trk = (uint8_t)buf[29];
			sprintf(buf,"%d",trk);
			demux_info_add(demuxer,"Track",buf);
		}
		
		g = stream_read_char(s);
		demux_info_add(demuxer,"Genre",genres[g]);
      }
    }

    break;
  case WAV: {
    unsigned int chunk_type;
    unsigned int chunk_size;
    WAVEFORMATEX* w;
    int l;
    l = stream_read_dword_le(s);
    if(l < 16) {
      
	#ifdef DEBUG  
	demux_audio_printf("[demux_audio] Bad wav header length: too short (%d)!!!\n",l);
	#endif
	
	l = 16;
    }
    if(l > MAX_WAVHDR_LEN) {
      
	#ifdef DEBUG  
	demux_audio_printf("[demux_audio] Bad wav header length: too long (%d)!!!\n",l);
	#endif
	
	l = 16;
    }
    sh_audio->wf = w = malloc(l > sizeof(WAVEFORMATEX) ? l : sizeof(WAVEFORMATEX));
    w->wFormatTag = sh_audio->format = stream_read_word_le(s);
    w->nChannels = sh_audio->channels = stream_read_word_le(s);
    w->nSamplesPerSec = sh_audio->samplerate = stream_read_dword_le(s);
    w->nAvgBytesPerSec = stream_read_dword_le(s);
    w->nBlockAlign = stream_read_word_le(s);
    w->wBitsPerSample = stream_read_word_le(s);
    sh_audio->samplesize = (w->wBitsPerSample + 7) / 8;
    w->cbSize = 0;
    sh_audio->i_bps = sh_audio->wf->nAvgBytesPerSec;
    l -= 16;
    if (l >= 2) {
      w->cbSize = stream_read_word_le(s);
      l -= 2;
      if (l < w->cbSize) {
	#ifdef DEBUG  
        demux_audio_printf("[demux_audio] truncated extradata (%d < %d)\n",l,w->cbSize);
	#endif
	
        w->cbSize = l;
      }
      stream_read(s,(unsigned char*)((char*)(w)+sizeof(WAVEFORMATEX)),w->cbSize);
      l -= w->cbSize;
    }

    //if( mp_msg_test(MSGT_DEMUX,MSGL_V) ) print_wave_header(w, MSGL_V);
    if(l)
      stream_skip(s,l);
    do
    {
      chunk_type = stream_read_fourcc(demuxer->stream);
      chunk_size = stream_read_dword_le(demuxer->stream);
      if (chunk_type != mmioFOURCC('d', 'a', 't', 'a'))
        stream_skip(demuxer->stream, chunk_size);
    } while (!s->eof && chunk_type != mmioFOURCC('d', 'a', 't', 'a'));
    demuxer->movi_start = stream_tell(s);
    demuxer->movi_end = chunk_size ? demuxer->movi_start + chunk_size : s->end_pos;
//    printf("wav: %X .. %X\n",(int)demuxer->movi_start,(int)demuxer->movi_end);
    // Check if it contains dts audio
    if((w->wFormatTag == 0x01) && (w->nChannels == 2) && (w->nSamplesPerSec == 44100)) {
	unsigned char buf[16384]; // vlc uses 16384*4 (4 dts frames)
	unsigned int i;
	memset(buf, 0, sizeof(buf));
	stream_read(s, buf, sizeof(buf));
	for (i = 0; i < sizeof(buf) - 5; i += 2) {
	    // DTS, 14 bit, LE
	    if((buf[i] == 0xff) && (buf[i+1] == 0x1f) && (buf[i+2] == 0x00) &&
	       (buf[i+3] == 0xe8) && ((buf[i+4] & 0xfe) == 0xf0) && (buf[i+5] == 0x07)) {
		sh_audio->format = 0x2001;
		
		#ifdef DEBUG  
		demux_audio_printf("[demux_audio] DTS audio in wav, 14 bit, LE\n");
		#endif
		
		break;
	    }
	    // DTS, 14 bit, BE
	    if((buf[i] == 0x1f) && (buf[i+1] == 0xff) && (buf[i+2] == 0xe8) &&
	       (buf[i+3] == 0x00) && (buf[i+4] == 0x07) && ((buf[i+5] & 0xfe) == 0xf0)) {
		sh_audio->format = 0x2001;
		
		#ifdef DEBUG  
		demux_audio_printf("[demux_audio] DTS audio in wav, 14 bit, BE\n");
		#endif
		
		break;
	    }
	    // DTS, 16 bit, BE
	    if((buf[i] == 0x7f) && (buf[i+1] == 0xfe) && (buf[i+2] == 0x80) &&
	       (buf[i+3] == 0x01)) {
		sh_audio->format = 0x2001;
		
		#ifdef DEBUG  
		demux_audio_printf("[demux_audio] DTS audio in wav, 16 bit, BE\n");
		#endif
		
		break;
	    }
	    // DTS, 16 bit, LE
	    if((buf[i] == 0xfe) && (buf[i+1] == 0x7f) && (buf[i+2] == 0x01) &&
	       (buf[i+3] == 0x80)) {
		sh_audio->format = 0x2001;
		
		#ifdef DEBUG  
		demux_audio_printf("[demux_audio] DTS audio in wav, 16 bit, LE\n");
		#endif
		
		break;
	    }
	}
	
	#ifdef DEBUG  
	if (sh_audio->format == 0x2001)
	    demux_audio_printf("[demux_audio] DTS sync offset = %u\n", i);
	#endif
    }
    stream_seek(s,demuxer->movi_start);  
  } break;
  case fLaC:
	    sh_audio->format = mmioFOURCC('f', 'L', 'a', 'C');
	    demuxer->movi_start = stream_tell(s) - 4;
	    demuxer->movi_end = s->end_pos;
	    if (demuxer->movi_end > demuxer->movi_start) {
	      // try to find out approx. bitrate
	      int64_t size = demuxer->movi_end - demuxer->movi_start;
	      int64_t num_samples = 0;
	      int32_t srate = 0;
	      stream_skip(s, 14);
	      stream_read(s, (unsigned char *)&srate, 3);
	      srate = be2me_32(srate) >> 12;
	      stream_read(s, (unsigned char *)&num_samples, 5);
	      num_samples = (be2me_64(num_samples) >> 24) & 0xfffffffffULL;
	      if (num_samples && srate)
	        sh_audio->i_bps = size * srate / num_samples;
	    }
	    if (sh_audio->i_bps < 1) // guess value to prevent crash
	      sh_audio->i_bps = 64 * 1024;
//	    get_flac_metadata (demuxer);
	    break;
  }

  priv = malloc(sizeof(da_priv_t));
  priv->frmt = frmt;
  priv->next_pts = 0;
  demuxer->priv = priv;
  demuxer->audio->id = 0;
  demuxer->audio->sh = sh_audio;
  sh_audio->ds = demuxer->audio;
  sh_audio->samplerate = sh_audio->audio.dwRate;

  if(stream_tell(s) != demuxer->movi_start)
  {
    #ifdef DEBUG  
    demux_audio_printf("demux_audio: seeking from 0x%X to start pos 0x%X\n", (int)stream_tell(s), (int)demuxer->movi_start);
    #endif
    
    stream_seek(s,demuxer->movi_start);
    if (stream_tell(s) != demuxer->movi_start) {
      #ifdef DEBUG  
      demux_audio_printf("demux_audio: seeking failed, now at 0x%X!\n", (int)stream_tell(s));
      #endif
      
      if (next_frame_pos) {
	#ifdef DEBUG  
        demux_audio_printf("demux_audio: seeking to 0x%X instead\n", (int)next_frame_pos);
	#endif
        stream_seek(s, next_frame_pos);
      }
    }
  }

  #ifdef DEBUG  
  printf("demux_audio: audio data 0x%X - 0x%X  \n",(int)demuxer->movi_start,(int)demuxer->movi_end);
  #endif

  return DEMUXER_TYPE_AUDIO;
}


static int demux_audio_fill_buffer(demuxer_t *demuxer, demux_stream_t *ds) {
//printf("%s::%d\n", __FUNCTION__, __LINE__);
  int l;
  demux_packet_t* dp;
  
  demuxer_t* demux = ds->demuxer;
  sh_audio_t* sh_audio = demuxer->audio->sh;
  da_priv_t* priv = demux->priv;
  double this_pts = priv->next_pts;
  stream_t* s = demux->stream;

  if(s->eof)
    return 0;
  switch(priv->frmt) {
  case MP3 :
    while(1) {
      uint8_t hdr[4];
      if(audio_stop) return 0;
      stream_read(s,hdr,4);
      if (s->eof)
        return 0;
      l = mp_decode_mp3_header(hdr);
      if(l < 0) {
	if (demux->movi_end && stream_tell(s) >= demux->movi_end)
	  return 0; // might be ID3 tag, i.e. EOF
	stream_skip(s,-3);
      } else {
	dp = new_demux_packet(l);
	memcpy(dp->buffer,hdr,4);
	if (stream_read(s,dp->buffer + 4,l-4) != l-4)
	{
	  free_demux_packet(dp);
	  return 0;
	}
	priv->next_pts += sh_audio->audio.dwScale/(double)sh_audio->samplerate;
	break;
      }
    } break;
  case WAV : {
    unsigned align = sh_audio->wf->nBlockAlign;
    l = sh_audio->wf->nAvgBytesPerSec;
    if (l <= 0) l = 65536;
    if (demux->movi_end && l > demux->movi_end - stream_tell(s)) {
      // do not read beyond end, there might be junk after data chunk
      l = demux->movi_end - stream_tell(s);
      if (l <= 0) return 0;
    }
    if (align)
      l = (l + align - 1) / align * align;
    dp = new_demux_packet(l);
    l = stream_read(s,dp->buffer,l);
    priv->next_pts += l/(double)sh_audio->i_bps;
    break;
  }
  case fLaC: {
    l = 65535;
    dp = new_demux_packet(l);
    l = stream_read(s,dp->buffer,l);
    /* FLAC is not a constant-bitrate codec. These values will be wrong. */
    priv->next_pts += l/(double)sh_audio->i_bps;
    break;
  }
  default:
    #ifdef DEBUG  
    demux_audio_printf("DEMUX_AUDIO_UnknownFormat %p\n",priv->frmt);
    #endif
    return 0;
  }
  resize_demux_packet(dp, l);
  dp->pts = this_pts;
  ds_add_packet(ds, dp);
  return 1;
}

static void high_res_mp3_seek(demuxer_t *demuxer,float time) {
  uint8_t hdr[4];
  int len,nf;
  da_priv_t* priv = demuxer->priv;
  sh_audio_t* sh = (sh_audio_t*)demuxer->audio->sh;

  nf = time*sh->samplerate/sh->audio.dwScale;
  while(nf > 0) {
    stream_read(demuxer->stream,hdr,4);
    len = mp_decode_mp3_header(hdr);
    if(len < 0) {
      stream_skip(demuxer->stream,-3);
      continue;
    }
    stream_skip(demuxer->stream,len-4);
    priv->next_pts += sh->audio.dwScale/(double)sh->samplerate;
    nf--;
  }
}

static int whileSeeking = 0;

static void demux_audio_seek(demuxer_t *demuxer,float rel_seek_secs,float audio_delay,int flags){
	sh_audio_t* sh_audio;
	stream_t* s;
	int64_t base,pos;
	float len;
	da_priv_t* priv;

	if(!(sh_audio = demuxer->audio->sh))
		return;

	#ifdef DEBUG
	printf("%s::%s\n", FILENAME, __FUNCTION__);
	#endif

	whileSeeking = 1;
	getAudioMutex(FILENAME, __FUNCTION__,__LINE__);
	
	s = demuxer->stream;
	priv = demuxer->priv;

	if(priv->frmt == MP3 && hr_mp3_seek && !(flags & SEEK_FACTOR)) {
		len = (flags & SEEK_ABSOLUTE) ? rel_seek_secs - priv->next_pts : rel_seek_secs;
		if(len < 0) {
			stream_seek(s,demuxer->movi_start);
			len = priv->next_pts + len;
			priv->next_pts = 0;
		}
		if(len > 0)
			high_res_mp3_seek(demuxer,len);
		
		releaseAudioMutex(FILENAME, __FUNCTION__,__LINE__);
		whileSeeking = 0;
		return;
	}

	base = flags&SEEK_ABSOLUTE ? demuxer->movi_start : stream_tell(s);
	if(flags&SEEK_FACTOR)
		pos = base + ((demuxer->movi_end - demuxer->movi_start)*rel_seek_secs);
	else
		pos = base + (rel_seek_secs*sh_audio->i_bps);

	if(demuxer->movi_end && pos >= demuxer->movi_end) {
		pos = demuxer->movi_end;
	} else if(pos < demuxer->movi_start)
		pos = demuxer->movi_start;

	priv->next_pts = (pos-demuxer->movi_start)/(double)sh_audio->i_bps;

	switch(priv->frmt) {
		case WAV:
			pos -= (pos - demuxer->movi_start) %
			(sh_audio->wf->nBlockAlign ? sh_audio->wf->nBlockAlign :
			(sh_audio->channels * sh_audio->samplesize));
		break;
	}

	stream_seek(s,pos);

	releaseAudioMutex(FILENAME, __FUNCTION__,__LINE__);
	whileSeeking = 0;
}

static void demux_close_audio(demuxer_t* demuxer) {
	da_priv_t* priv = demuxer->priv;

	free(priv);
	demuxer->priv = NULL;
}

////////////////////////////////////////////////////////////////7
////////////////////////////////////////////////////////////////7

#include "common.h"
#include "container.h"
#include "manager.h"
//#include "utils.h"

//static demuxer_t *demuxer = NULL;
static demux_stream_t *ds = NULL;   // dvd subtitle buffer/demuxer
//static sh_audio_t *sh_audio = NULL;
//static sh_video_t *sh_video = NULL;
static pthread_t PlayThread;
static int hasPlayThreadStarted = 0;
//static int isFirstAudioFrame = 1;

int AUDIOInit(Context_t *context, char * filename) {
	#ifdef DEBUG
	printf("%s::%s\n", FILENAME, __FUNCTION__);
	#endif

	getAudioMutex(FILENAME, __FUNCTION__,__LINE__);
	
	ds = (demux_stream_t*)malloc ( sizeof(demux_stream_t));
	memset (ds,0,sizeof(demux_stream_t));

	ds->demuxer = (demuxer_t*)malloc ( sizeof(demuxer_t));
	memset (ds->demuxer,0,sizeof(demuxer_t));

	ds->demuxer->audio = (demux_stream_t*)malloc ( sizeof(demux_stream_t));
	memset (ds->demuxer->audio,0,sizeof(demux_stream_t));
	
	ds->demuxer->stream = (stream_t*)malloc ( sizeof(stream_t));
	memset (ds->demuxer->stream,0,sizeof(stream_t));

	ds->demuxer->stream->fd = context->playback->fd;

	read(ds->demuxer->stream->fd,ds->demuxer->stream->buffer,2048);//soviel ??

	ds->demuxer->stream->start_pos	 = 0;
	ds->demuxer->stream->flags	 = 6;
	ds->demuxer->stream->sector_size = 0;
	ds->demuxer->stream->buf_pos	 = 0;
	ds->demuxer->stream->buf_len	 = 2048;
	ds->demuxer->stream->pos	 = 2048;
	ds->demuxer->stream->start_pos	 = 0;
      
	if(context->playback->isFile) {
	    ds->demuxer->stream->type = STREAMTYPE_FILE;
	    long pos = lseek(ds->demuxer->stream->fd, 0L, SEEK_CUR);
	    ds->demuxer->stream->end_pos = lseek(ds->demuxer->stream->fd, 0L, SEEK_END);
	    lseek(ds->demuxer->stream->fd, pos, SEEK_SET);
	} else {
		ds->demuxer->stream->type = STREAMTYPE_STREAM;
		ds->demuxer->stream->end_pos = 0;
	}
    
	ds->demuxer->stream->eof	= 0;
	ds->demuxer->stream->cache_pid	= 0;
	
	demux_audio_open(ds->demuxer);
	    
	da_priv_t* priv = ds->demuxer->priv;

  	switch(priv->frmt) {
  	case MP3 : {
  	    Track_t Audio = {
			"und",
			"A_MPEG/L3",
			0,
		};
		context->manager->audio->Command(context, MANAGER_ADD, &Audio);
		break;}
  	case WAV :{
  		Track_t Audio = {
			"und",
			"A_MP3",
			0,
		};
		context->manager->audio->Command(context, MANAGER_ADD, &Audio);
		break;}
  	case fLaC :{
  		Track_t Audio = {
			"und",
			"A_MP3",
			0,
		};
		context->manager->audio->Command(context, MANAGER_ADD, &Audio);
		break;}
	}
	
	releaseAudioMutex(FILENAME, __FUNCTION__,__LINE__);
	
	return 0; // FIXME
}

#define INVALID_PTS_VALUE                       0x200000000ull
void AUDIOGenerateParcel(Context_t *context, const demuxer_t *demuxer) {
	//printf("%s::%d\n", __FUNCTION__, __LINE__);

	unsigned long long int Pts = 0;

	 if (ds->first != NULL) {
		demux_packet_t * current = ds->first;
		
		Pts = (current->pts * 90000);
		//printf("a current->pts=%f Pts=%llu\n", current->pts, Pts);
		
		while (current != NULL) {		
			context->output->audio->Write(context, current->buffer, current->len, Pts, NULL, 0, 0, "audio");

			current = current->next;
			Pts = INVALID_PTS_VALUE;
		}
	}
}

static void AUDIOThread(Context_t *context) {
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

		getAudioMutex(FILENAME, __FUNCTION__,__LINE__);

		if ( !demux_audio_fill_buffer(ds->demuxer,ds) ) {
			#ifdef DEBUG
			printf("%s::%s demux_ts_fill_buffer failed!\n", FILENAME, __FUNCTION__);
			#endif
			
			releaseAudioMutex(FILENAME, __FUNCTION__,__LINE__);
			
			break;
		} else {		
			AUDIOGenerateParcel(context, ds->demuxer);

			if (ds != NULL && ds->first != NULL) {
				ds_free_packs(ds);
			}

			//printf("%s <--\n", __FUNCTION__);

			releaseAudioMutex(FILENAME, __FUNCTION__,__LINE__);
		}
	}

	usleep(500000);
	
	hasPlayThreadStarted = 0;
	
	if(context && context->playback)
	   context->playback->Command(context, PLAYBACK_TERM, NULL);
	   
	#ifdef DEBUG
	printf("%s::%s terminating\n",FILENAME, __FUNCTION__);
	#endif
}


static int AUDIOPlay(Context_t *context) {
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
	
	audio_stop = 0;
	if (hasPlayThreadStarted == 0) {
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

		if((error=pthread_create(&PlayThread, &attr, (void *)&AUDIOThread, context)) != 0) {
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

static int AUDIOStop(Context_t *context) {
	#ifdef DEBUG
	printf("%s::%s\n", FILENAME, __FUNCTION__);
	#endif

	int ret = 0;
	int wait_time = 20;
	
	audio_stop = 1;
	while ( (hasPlayThreadStarted != 0) && (--wait_time) > 0 ) {
		#ifdef DEBUG  
		printf("%s::%s Waiting for Audio thread to terminate itself, will try another %d times\n", FILENAME, __FUNCTION__, wait_time);
		#endif
		
		usleep(100000);
	}

	if (wait_time == 0) {
		#ifdef DEBUG  
		printf("%s::%s Timeout waiting for Audio thread!\n", FILENAME, __FUNCTION__);
		#endif
		
		ret = -1;
	} else {
		if(ds != NULL) {
		  
			getAudioMutex(FILENAME, __FUNCTION__,__LINE__);
			
			if(ds->demuxer != NULL) {
				demux_close_audio(ds->demuxer);

				free(ds->demuxer->stream);
				ds->demuxer->stream = NULL;
			
				free(ds->demuxer->audio);
				ds->demuxer->audio = NULL;
				
				free(ds->demuxer);  
				ds->demuxer = NULL;
			}

			free(ds);
			ds = NULL;

			releaseAudioMutex(FILENAME, __FUNCTION__,__LINE__);
		}
	}

	return ret;	
}

static int AUDIOGetLength(demuxer_t *demuxer,double * length) {
	//printf("%s::%s\n", FILENAME, __FUNCTION__);
    
	int audio_length = 0;

	if(demuxer->audio != NULL && demuxer->audio->sh != NULL) {
		sh_audio_t *sh_audio = demuxer->audio->sh;
		audio_length = sh_audio->i_bps ? demuxer->movi_end / sh_audio->i_bps : 0;
	}
          
	if (audio_length <= 0) return -1;
	
	*((double *)length) = (double)audio_length;
		
	return 0;

}

static int AUDIOGetInfo(demuxer_t *demuxer,char ** infoString) {
	#ifdef DEBUG  
	printf("%s::%s\n", FILENAME, __FUNCTION__);
	#endif
    
	if(demuxer && demuxer->info) {
		char **info = demuxer->info;
		int n = 0;

		for(n = 0; info && info[2*n] != NULL; n++) {
			if(!strcasecmp(*infoString, info[2*n])) {
				#ifdef DEBUG  
				printf("strdup in %s::%s:%d\n", FILENAME, __FUNCTION__,__LINE__);
				#endif
			
				*infoString = strdup(info[2*n+1]);
			}
		}
	}
		
	return 0;

}

static int Command(Context_t  *context, ContainerCmd_t command, void * argument) {
	#ifdef DEBUG  
	printf("%s::%s Command %d\n", FILENAME, __FUNCTION__, command);
	#endif

	int ret = 0;
	
	switch(command) {
		case CONTAINER_INIT: {
			char * FILENAME = (char *)argument;
			ret = AUDIOInit(context, FILENAME);
			break;
		}
		case CONTAINER_PLAY: {
			ret = AUDIOPlay(context);
			break;
		}
		case CONTAINER_STOP: {
			ret = AUDIOStop(context);
			break;
		}
		case CONTAINER_SEEK: {
			if (ds && ds->demuxer)
				demux_audio_seek (ds->demuxer, (float)*((float*)argument), 0.0, 0);
			break;
		}
		case CONTAINER_LENGTH: {
			double length = 0;
			if (ds != NULL && ds->demuxer  != NULL && AUDIOGetLength(ds->demuxer, &length)!=0)
				*((double*)argument) = (double)0;
			else
				*((double*)argument) = (double)length;
			break;
		}
		case CONTAINER_INFO: {
			if (ds && ds->demuxer)
				ret = AUDIOGetInfo(ds->demuxer, (char **)argument);
			break;
		}
		default:
			#ifdef DEBUG  
			printf("%s::%s ContainerCmd %d not supported!\n", FILENAME, __FUNCTION__, command);
			#endif
			break;
	}

	return ret;
}

static char *AUDIOCapabilities[] = { "mp3", "wav", "flac", NULL };

Container_t AUDIOContainer = {
	"AUDIO",
	&Command,
	AUDIOCapabilities,
};

/*const demuxer_desc_t demuxer_desc_audio = {
  "Audio demuxer",
  "audio",
  "Audio only",
  "?",
  "Audio only files",
  DEMUXER_TYPE_AUDIO,
  0, // unsafe autodetect
  demux_audio_open,
  demux_audio_fill_buffer,
  NULL,
  demux_close_audio,
  demux_audio_seek,
  demux_audio_control
};*/
