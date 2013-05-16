#include <stdio.h>
#include <string.h>
#include "common.h"
#include "output.h"

//#ifndef DEBUG
//#define DEBUG	// FIXME: until this is set properly by Makefile
//#endif

static const char FILENAME[] = "output.c";

static void printOutputCapabilities() {
	int i, j;
#ifdef DEBUG
	printf("%s::%s\n", FILENAME, __FUNCTION__);
#endif
	printf("Capabilities:\n");
	for (i = 0; AvailableOutput[i] != NULL; i++)	{
		printf("\t%s : ", AvailableOutput[i]->Name);
		for (j = 0; AvailableOutput[i]->Capabilities[j] != NULL; j++)
		printf("%s ", AvailableOutput[i]->Capabilities[j]);
		printf("\n");
	}
}

static void OutputAdd(Context_t  *context, char * port) {
	int i, j;
#ifdef DEBUG
	printf("%s::%s\n", FILENAME, __FUNCTION__);
#endif
	for (i = 0; AvailableOutput[i] != NULL; i++)	
		for (j = 0; AvailableOutput[i]->Capabilities[j] != NULL; j++)
			if (!strcmp(AvailableOutput[i]->Capabilities[j], port)) {
				if (!strcmp("audio", port))
					context->output->audio = AvailableOutput[i];
				else if (!strcmp("video", port))
					context->output->video = AvailableOutput[i];
				else if (!strcmp("subtitle", port))
					context->output->subtitle = AvailableOutput[i];
				break;
			}
}

static void OutputDel(Context_t  *context, char * port) {
#ifdef DEBUG
	printf("%s::%s\n", FILENAME, __FUNCTION__);
#endif

    if (!strcmp("audio", port))
        context->output->audio = NULL;
    else if (!strcmp("video", port))
        context->output->video = NULL;
    else if (!strcmp("subtitle", port))
        context->output->subtitle = NULL;

}

static int Command(void  *_context, OutputCmd_t command, void * argument) {
Context_t  *context = (Context_t*) _context;
#ifdef DEBUG
	printf("%s::%s Command %d\n", FILENAME, __FUNCTION__, command);
#endif
	
	int ret = 0;
	
	switch(command) {
		case OUTPUT_OPEN: {
			if (context && context->playback ) {
				if (context->playback->isVideo)
					context->output->video->Command(context, OUTPUT_OPEN, "video");
				if (context->playback->isAudio)
					context->output->audio->Command(context, OUTPUT_OPEN, "audio");
				if (context->playback->isSubtitle)
					context->output->subtitle->Command(context, OUTPUT_OPEN, "subtitle");
			} else
				ret = -1;
			break;
		}
		case OUTPUT_CLOSE: {
			if (context && context->playback ) {
				if (context->playback->isVideo)
					context->output->video->Command(context, OUTPUT_CLOSE, "video");
				if (context->playback->isAudio)
					context->output->audio->Command(context, OUTPUT_CLOSE, "audio");
				if (context->playback->isSubtitle)
					context->output->subtitle->Command(context, OUTPUT_CLOSE, "subtitle");
			} else
				ret = -1;
			break;
		}
		case OUTPUT_ADD: {
			OutputAdd(context, (char*) argument);
			break;
		}
		case OUTPUT_DEL: {
			OutputDel(context, (char*) argument);
			break;
		}
		case OUTPUT_CAPABILITIES: {
			printOutputCapabilities();
			break;
		}
		case OUTPUT_PLAY: { // 4
			if (context && context->playback ) {
				if (context->playback->isVideo)
					ret = context->output->video->Command(context, OUTPUT_PLAY, "video");
				
				if (!ret) {	// success or not executed, dunn care
					if (context->playback->isAudio)
						ret = context->output->audio->Command(context, OUTPUT_PLAY, "audio");
					
					if (!ret) {	// success or not executed, dunn care
						if (context->playback->isSubtitle)
							ret = context->output->subtitle->Command(context, OUTPUT_PLAY, "subtitle");
					}
				}
			} else
				ret = -1;
			break;
		}
		case OUTPUT_STOP: {
			if (context && context->playback ) {
				if (context->playback->isVideo)
					context->output->video->Command(context, OUTPUT_STOP, "video");
				if (context->playback->isAudio)
					context->output->audio->Command(context, OUTPUT_STOP, "audio");
				if (context->playback->isSubtitle)
					context->output->subtitle->Command(context, OUTPUT_STOP, "subtitle");
			} else
				ret = -1;
			break;
		}
		case OUTPUT_FLUSH: {
			if (context && context->playback ) {
				if (context->playback->isVideo)
					context->output->video->Command(context, OUTPUT_FLUSH, "video");
				if (context->playback->isAudio)
					context->output->audio->Command(context, OUTPUT_FLUSH, "audio");
				//if (context->playback->isSubtitle)
				//	context->output->subtitle->Command(context, OUTPUT_FLUSH, "subtitle");
			} else
				ret = -1;
			break;
		}
		case OUTPUT_PAUSE: {	// 6
			if (context && context->playback ) {
				if (context->playback->isVideo)
					context->output->video->Command(context, OUTPUT_PAUSE, "video");
				if (context->playback->isAudio)
					context->output->audio->Command(context, OUTPUT_PAUSE, "audio");
				//if (context->playback->isSubtitle)
				//	context->output->subtitle->Command(context, OUTPUT_PAUSE, "subtitle");
			} else
				ret = -1;
			break;
		}
		case OUTPUT_FASTFORWARD: {
			if (context && context->playback ) {
				if (context->playback->isVideo)
					context->output->video->Command(context, OUTPUT_FASTFORWARD, "video");
				if (context->playback->isAudio)
					context->output->audio->Command(context, OUTPUT_FASTFORWARD, "audio");
				//if (context->playback->isSubtitle)
				//	context->output->subtitle->Command(context, OUTPUT_PAUSE, "subtitle");
			} else
				ret = -1;
			break;
		}
		case OUTPUT_CONTINUE: {
			if (context && context->playback ) {
				if (context->playback->isVideo)
					context->output->video->Command(context, OUTPUT_CONTINUE, "video");
				if (context->playback->isAudio)
					context->output->audio->Command(context, OUTPUT_CONTINUE, "audio");
				//if (context->playback->isSubtitle)
				//	context->output->subtitle->Command(context, OUTPUT_CONTINUE, "subtitle");
			} else
				ret = -1;
			break;
		}
		case OUTPUT_AVSYNC: {
			if (context && context->playback ) {
				if (context->playback->isVideo && context->playback->isAudio)
					context->output->audio->Command(context, OUTPUT_AVSYNC, "audio");
			} else
				ret = -1;
			break;
		}
		case OUTPUT_CLEAR: {
			if (context && context->playback ) {
				if (context->playback->isVideo && (argument == NULL || *(char *) argument == 'v'))
					context->output->video->Command(context, OUTPUT_CLEAR, "video");
				if (context->playback->isAudio && (argument == NULL || *(char *) argument == 'a'))
					context->output->audio->Command(context, OUTPUT_CLEAR, "audio");
				//if (context->playback->isSubtitle && (argument == NULL || *(char *) argument == 's'))
				//	context->output->subtitle->Command(context, OUTPUT_CLEAR, "subtitle");
			} else
				ret = -1;
			break;
		}
		case OUTPUT_PTS: {
			if (context && context->playback ) {
				if (context->playback->isVideo)
					return context->output->video->Command(context, OUTPUT_PTS, argument);
				if (context->playback->isAudio)
					return context->output->audio->Command(context, OUTPUT_PTS, argument);
				//if (context->playback->isSubtitle)
				//	return context->output->subtitle->Command(context, OUTPUT_PTS, "subtitle");
			} else
				ret = -1;
			break;
		}
		case OUTPUT_SWITCH: {
			if (context && context->playback ) {
				if (context->playback->isAudio)
					return context->output->audio->Command(context, OUTPUT_SWITCH, "audio");
				if (context->playback->isVideo)
					return context->output->video->Command(context, OUTPUT_SWITCH, "video");
			} else
				ret = -1;
			break;
		}
		case OUTPUT_SLOWMOTION: {
			if (context && context->playback ) {
				if (context->playback->isVideo)
					context->output->video->Command(context, OUTPUT_SLOWMOTION, "video");
				if (context->playback->isAudio)
					context->output->audio->Command(context, OUTPUT_SLOWMOTION, "audio");
				//if (context->playback->isSubtitle)
				//	context->output->subtitle->Command(context, OUTPUT_PAUSE, "subtitle");
			} else
				ret = -1;
			break;
		}
		case OUTPUT_AUDIOMUTE: {
			if (context && context->playback ) {
				if (context->playback->isAudio)
					context->output->audio->Command(context, OUTPUT_AUDIOMUTE, (char*) argument);
			} else
				ret = -1;
			break;
		}
		default:
#ifdef DEBUG
			printf("%s::%s OutputCmd %d not supported!\n", FILENAME, __FUNCTION__, command);
#endif
			break;
	}

#ifdef DEBUG
	printf("%s::%s exiting with value %d\n", FILENAME, __FUNCTION__, ret);
#endif

	return ret;
}

OutputHandler_t OutputHandler = {
	"Output",
	NULL,
	NULL,
	NULL,
	&Command,
};
