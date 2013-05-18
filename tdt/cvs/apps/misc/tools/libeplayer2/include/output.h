#ifndef OUTPUT_H_
#define OUTPUT_H_

#include <stdio.h>

typedef enum {OUTPUT_INIT, OUTPUT_ADD, OUTPUT_DEL, OUTPUT_CAPABILITIES, OUTPUT_PLAY, OUTPUT_STOP, OUTPUT_PAUSE, OUTPUT_OPEN, OUTPUT_CLOSE, OUTPUT_FLUSH, OUTPUT_CONTINUE, OUTPUT_FASTFORWARD, OUTPUT_AVSYNC, OUTPUT_CLEAR, OUTPUT_PTS, OUTPUT_SWITCH, OUTPUT_SLOWMOTION, OUTPUT_AUDIOMUTE} OutputCmd_t;


typedef struct Output_s {
	char * Name;
	int (* Command) (/*Context_t*/void  *, OutputCmd_t, void *);
	int (* Write) (/*Context_t*/void  *, unsigned char *, int, unsigned long long int, unsigned char *, const int, float, char *);
	char ** Capabilities;

} Output_t;

extern Output_t LinuxDvbOutput;
extern Output_t Enigma2Output;

static Output_t * AvailableOutput[] = {
	&LinuxDvbOutput,
	&Enigma2Output,
	NULL
};

typedef struct OutputHandler_s {
	char * Name;
	Output_t * audio;
	Output_t * video;
	Output_t * subtitle;
	int (* Command) (/*Context_t*/void  *, OutputCmd_t, void *);
} OutputHandler_t;

#endif
