
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>


// for avi_stream_id():
#include "stream.h"
#include "demuxer.h"
#include "stheader.h"

#include "aviheader.h"


int aviprint_debug = 1;
#define aviprint_printf(x...) do { if (aviprint_debug)printf(x); } while (0)

void print_avih_flags(MainAVIHeader *h, int verbose_level){
  aviprint_printf("MainAVIHeader.dwFlags: (%"PRId32")%s%s%s%s%s%s\n",h->dwFlags,
    (h->dwFlags&AVIF_HASINDEX)?" HAS_INDEX":"",
    (h->dwFlags&AVIF_MUSTUSEINDEX)?" MUST_USE_INDEX":"",
    (h->dwFlags&AVIF_ISINTERLEAVED)?" IS_INTERLEAVED":"",
    (h->dwFlags&AVIF_TRUSTCKTYPE)?" TRUST_CKTYPE":"",
    (h->dwFlags&AVIF_WASCAPTUREFILE)?" WAS_CAPTUREFILE":"",
    (h->dwFlags&AVIF_COPYRIGHTED)?" COPYRIGHTED":""
  );
}

void print_avih(MainAVIHeader *h, int verbose_level){
  aviprint_printf("======= AVI Header =======\n");
  aviprint_printf("us/frame: %"PRId32"  (fps=%5.3f)\n",h->dwMicroSecPerFrame,1000000.0f/(float)h->dwMicroSecPerFrame);
  aviprint_printf("max bytes/sec: %"PRId32"\n",h->dwMaxBytesPerSec);
  aviprint_printf("padding: %"PRId32"\n",h->dwPaddingGranularity);
  print_avih_flags(h, verbose_level);
  aviprint_printf("frames  total: %"PRId32"   initial: %"PRId32"\n",h->dwTotalFrames,h->dwInitialFrames);
  aviprint_printf("streams: %"PRId32"\n",h->dwStreams);
  aviprint_printf("Suggested BufferSize: %"PRId32"\n",h->dwSuggestedBufferSize);
  aviprint_printf("Size:  %"PRId32" x %"PRId32"\n",h->dwWidth,h->dwHeight);
  aviprint_printf("==========================\n");
}

void print_strh(AVIStreamHeader *h, int verbose_level){
  aviprint_printf("====== STREAM Header =====\n");
  aviprint_printf("Type: %.4s   FCC: %.4s (%X)\n",(char *)&h->fccType,(char *)&h->fccHandler,(unsigned int)h->fccHandler);
  aviprint_printf("Flags: %"PRId32"\n",h->dwFlags);
  aviprint_printf("Priority: %d   Language: %d\n",h->wPriority,h->wLanguage);
  aviprint_printf("InitialFrames: %"PRId32"\n",h->dwInitialFrames);
  aviprint_printf("Rate: %"PRId32"/%"PRId32" = %5.3f\n",h->dwRate,h->dwScale,(float)h->dwRate/(float)h->dwScale);
  aviprint_printf("Start: %"PRId32"   Len: %"PRId32"\n",h->dwStart,h->dwLength);
  aviprint_printf("Suggested BufferSize: %"PRId32"\n",h->dwSuggestedBufferSize);
  aviprint_printf("Quality %"PRId32"\n",h->dwQuality);
  aviprint_printf("Sample size: %"PRId32"\n",h->dwSampleSize);
  aviprint_printf("==========================\n");
}

void print_wave_header(WAVEFORMATEX *h, int verbose_level){
  aviprint_printf("======= WAVE Format =======\n");
  aviprint_printf("Format Tag: %d (0x%X)\n",h->wFormatTag,h->wFormatTag);
  aviprint_printf("Channels: %d\n",h->nChannels);
  aviprint_printf("Samplerate: %"PRId32"\n",h->nSamplesPerSec);
  aviprint_printf("avg byte/sec: %"PRId32"\n",h->nAvgBytesPerSec);
  aviprint_printf("Block align: %d\n",h->nBlockAlign);
  aviprint_printf("bits/sample: %d\n",h->wBitsPerSample);
  aviprint_printf("cbSize: %d\n",h->cbSize);
  if(h->wFormatTag==0x55 && h->cbSize>=12){
	printf("FIXME\n");
/*      MPEGLAYER3WAVEFORMAT* h2=(MPEGLAYER3WAVEFORMAT *)h;
      aviprint_printf("mp3.wID=%d\n",h2->wID);
      aviprint_printf("mp3.fdwFlags=0x%"PRIX32"\n",h2->fdwFlags);
      aviprint_printf("mp3.nBlockSize=%d\n",h2->nBlockSize);
      aviprint_printf("mp3.nFramesPerBlock=%d\n",h2->nFramesPerBlock);
      aviprint_printf("mp3.nCodecDelay=%d\n",h2->nCodecDelay);*/
  }
  else if (h->cbSize > 0)
  {
    int i;
    uint8_t* p = (uint8_t*)(h + 1);
    aviprint_printf("Unknown extra header dump: ");
    for (i = 0; i < h->cbSize; i++)
	aviprint_printf("[%x] ", p[i]);
    aviprint_printf("\n");
  }
  aviprint_printf("==========================================================================\n");
}


void print_video_header(BITMAPINFOHEADER *h, int verbose_level){
  aviprint_printf("======= VIDEO Format ======\n");
	aviprint_printf("  biSize %d\n", h->biSize);
	aviprint_printf("  biWidth %d\n", h->biWidth);
	aviprint_printf("  biHeight %d\n", h->biHeight);
	aviprint_printf("  biPlanes %d\n", h->biPlanes);
	aviprint_printf("  biBitCount %d\n", h->biBitCount);
	aviprint_printf("  biCompression %d='%.4s'\n", h->biCompression, (char *)&h->biCompression);
	aviprint_printf("  biSizeImage %d\n", h->biSizeImage);
  if (h->biSize > sizeof(BITMAPINFOHEADER))
  {
    int i;
    uint8_t* p = (uint8_t*)(h + 1);
    aviprint_printf("Unknown extra header dump: ");
    for (i = 0; i < h->biSize-sizeof(BITMAPINFOHEADER); i++)
	aviprint_printf("[%x] ", *(p+i));
    aviprint_printf("\n");
  }
  aviprint_printf("===========================\n");
}

void print_vprp(VideoPropHeader *vprp, int verbose_level){
  int i;
  aviprint_printf("======= Video Properties Header =======\n");
  aviprint_printf("Format: %d  VideoStandard: %d\n",
         vprp->VideoFormatToken,vprp->VideoStandard);
  aviprint_printf("VRefresh: %d  HTotal: %d  VTotal: %d\n",
         vprp->dwVerticalRefreshRate, vprp->dwHTotalInT, vprp->dwVTotalInLines);
  aviprint_printf("FrameAspect: %d:%d  Framewidth: %d  Frameheight: %d\n",
         vprp->dwFrameAspectRatio >> 16, vprp->dwFrameAspectRatio & 0xffff,
         vprp->dwFrameWidthInPixels, vprp->dwFrameHeightInLines);
  aviprint_printf("Fields: %d\n", vprp->nbFieldPerFrame);
  for (i=0; i<vprp->nbFieldPerFrame; i++) {
    VIDEO_FIELD_DESC *vfd = &vprp->FieldInfo[i];
    aviprint_printf("  == Field %d description ==\n", i);
    aviprint_printf("  CompressedBMHeight: %d  CompressedBMWidth: %d\n",
           vfd->CompressedBMHeight, vfd->CompressedBMWidth);
    aviprint_printf("  ValidBMHeight: %d  ValidBMWidth: %d\n",
           vfd->ValidBMHeight, vfd->ValidBMWidth);
    aviprint_printf("  ValidBMXOffset: %d  ValidBMYOffset: %d\n",
           vfd->ValidBMXOffset, vfd->ValidBMYOffset);
    aviprint_printf("  VideoXOffsetInT: %d  VideoYValidStartLine: %d\n",
           vfd->VideoXOffsetInT, vfd->VideoYValidStartLine);
  }
  aviprint_printf("=======================================\n");
}

void print_index(AVIINDEXENTRY *idx, int idx_size, int verbose_level){
  int i;
  unsigned int pos[256];
  unsigned int num[256];
  memset(pos, 0, sizeof(pos));
  memset(num, 0, sizeof(num));
  for(i=0;i<idx_size;i++){
    int id=avi_stream_id(idx[i].ckid);
    if(id<0 || id>255) id=255;
    aviprint_printf("%5d:  %.4s  %4X  %016"PRIX64"  len:%6"PRId32"  pos:%7d->%7.3f %7d->%7.3f\n",i,
      (char *)&idx[i].ckid,
      (unsigned int)idx[i].dwFlags&0xffff,
      (uint64_t)AVI_IDX_OFFSET(&idx[i]),
//      idx[i].dwChunkOffset+demuxer->movi_start,
      idx[i].dwChunkLength,
      pos[id],(float)pos[id]/18747.0f,
      num[id],(float)num[id]/23.976f
    );
    pos[id]+=idx[i].dwChunkLength;
    ++num[id];
  }
}

void print_avistdindex_chunk(avistdindex_chunk *h, int verbose_level){
    aviprint_printf("====== AVI Standard Index Header ========\n");
    aviprint_printf("  FCC (%.4s) dwSize (%d) wLongsPerEntry(%d)\n", h->fcc, h->dwSize, h->wLongsPerEntry);
    aviprint_printf("  bIndexSubType (%d) bIndexType (%d)\n", h->bIndexSubType, h->bIndexType);
    aviprint_printf("  nEntriesInUse (%d) dwChunkId (%.4s)\n", h->nEntriesInUse, h->dwChunkId);
    aviprint_printf("  qwBaseOffset (0x%"PRIX64") dwReserved3 (%d)\n", h->qwBaseOffset, h->dwReserved3);
    aviprint_printf("===========================\n");
}
void print_avisuperindex_chunk(avisuperindex_chunk *h, int verbose_level){
    aviprint_printf("====== AVI Super Index Header ========\n");
    aviprint_printf("  FCC (%.4s) dwSize (%d) wLongsPerEntry(%d)\n", h->fcc, h->dwSize, h->wLongsPerEntry);
    aviprint_printf("  bIndexSubType (%d) bIndexType (%d)\n", h->bIndexSubType, h->bIndexType);
    aviprint_printf("  nEntriesInUse (%d) dwChunkId (%.4s)\n", h->nEntriesInUse, h->dwChunkId);
    aviprint_printf("  dwReserved[0] (%d) dwReserved[1] (%d) dwReserved[2] (%d)\n", 
	    h->dwReserved[0], h->dwReserved[1], h->dwReserved[2]);
    aviprint_printf("===========================\n");
}

