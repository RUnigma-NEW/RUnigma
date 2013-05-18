#include <stdio.h> 
#include <string.h>
#include <stdlib.h> 
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <memory.h>
#include <asm/types.h>
#include <pthread.h>
#include <errno.h>

#include "common.h"
#include "output.h"

//#ifndef DEBUG
//#define DEBUG	// FIXME: until this is set properly by Makefile
//#endif

static const char FILENAME[] = "enigma2.c";

/*
Number, Style, Name,, MarginL, MarginR, MarginV, Effect,, Text

1038,0,tdk,,0000,0000,0000,,That's not good.
1037,0,tdk,,0000,0000,0000,,{\i1}Rack them up, rack them up,{\i0}\N{\i1}rack them up.{\i0} [90]
1036,0,tdk,,0000,0000,0000,,Okay, rack them up.
*/

void replace_all(char ** string, char * search, char * replace) {

	int len = 0;
    char * ptr = NULL;
    char tempString[512];
    char newString[512];
    newString[0] = '\0';
    strncpy(tempString, *string, 511);
    tempString[511] = '\0';

    free(*string);

    

    while ((ptr = strstr(tempString, search)) != NULL) {
        len  = ptr - tempString;
        strncpy(newString, tempString, len);
        newString[len] = '\0';
        strcat(newString, replace);

        len += strlen(search);
	    strcat(newString, tempString+len);

        strcpy(tempString, newString);
    }
#ifdef DEBUG
		printf("strdup in %s::%s:%d\n", FILENAME, __FUNCTION__,__LINE__);
#endif
    if(newString[0] != '\0')
        *string = strdup(newString);
    else
        *string = strdup(tempString);

}

int Enigma2ParseASS (char **Line) {
#ifdef DEBUG
    printf("%s::%s -> Text=%s\n",  FILENAME, __FUNCTION__, *Line);
		printf("strdup in %s::%s:%d\n", FILENAME, __FUNCTION__,__LINE__);
#endif
    char * Text = strdup(*Line);
    int i;
    char *ptr1;
    
    ptr1 = Text;
    for (i=0; i < 8 && *ptr1 != '\0'; ptr1++) {
#ifdef DEBUG
        printf("%s",ptr1);
#endif
        if (*ptr1 == ',')
            i++;
    }

    free(*Line);
#ifdef DEBUG
		printf("strdup in %s::%s:%d\n", FILENAME, __FUNCTION__,__LINE__);
#endif
    *Line = strdup(ptr1);
    free(Text);

    replace_all(Line, "\\N", "\n");

    replace_all(Line, "{\\i1}", "<i>");
    replace_all(Line, "{\\i0}", "</i>");

#ifdef DEBUG
    printf("%s::%s <- Text=%s\n",  FILENAME, __FUNCTION__, *Line);
#endif
    return 0;
}

int Enigma2ParseSRT (char **Line) {
#ifdef DEBUG
    printf("%s::%s -> Text=%s\n",  FILENAME, __FUNCTION__, *Line);
#endif

    replace_all(Line, "\x0d", "");
    replace_all(Line, "\n\n", "\\N");
    replace_all(Line, "\n", "");
    replace_all(Line, "\\N", "\n");
    replace_all(Line, "ö", "oe");
    replace_all(Line, "ä", "ae");
    replace_all(Line, "ü", "ue");
    replace_all(Line, "Ö", "Oe");
    replace_all(Line, "Ä", "Ae");
    replace_all(Line, "Ü", "Ue");
    replace_all(Line, "ß", "ss");

#ifdef DEBUG
    printf("%s::%s <- Text=%s\n",  FILENAME, __FUNCTION__, *Line);
#endif
    return 0;
}

int Enigma2ParseSSA (char **Line) {
#ifdef DEBUG
    printf("%s::%s -> Text=%s\n",  FILENAME, __FUNCTION__, *Line);
#endif

    replace_all(Line, "\x0d", "");
    replace_all(Line, "\n\n", "\\N");
    replace_all(Line, "\n", "");
    replace_all(Line, "\\N", "\n");
    replace_all(Line, "ö", "oe");
    replace_all(Line, "ä", "ae");
    replace_all(Line, "ü", "ue");
    replace_all(Line, "Ö", "Oe");
    replace_all(Line, "Ä", "Ae");
    replace_all(Line, "Ü", "Ue");
    replace_all(Line, "ß", "ss");

#ifdef DEBUG
    printf("%s::%s <- Text=%s\n",  FILENAME, __FUNCTION__, *Line);
#endif
    return 0;
}

static pthread_t thread_sub;
void * _smp3 = NULL;
void (*_fkt) (long int, size_t, char *, void *);

int eplayerCreateSubtitleSink()
{
#ifdef DEBUG
    printf("eplayerCreateSubtitleSink->\n");
#endif
  
    return 0;
}

struct sub_t{
    char * text;
    unsigned long long int pts;
    unsigned long int milliDuration;
};

#define PUFFERSIZE 2
static struct sub_t subPuffer[PUFFERSIZE];
static int nextSubPointer = 0;
static int freeSubPointer = 0;


void addSub(Context_t  *context, char * text, unsigned long long int pts, unsigned long int milliDuration) {
#ifdef DEBUG
    printf("%s::%s\n",  FILENAME, __FUNCTION__);
#endif

    if(context && context->playback && !context->playback->isPlaying)
        return;

    while (subPuffer[freeSubPointer].text != NULL) {
        //List is full, wait till we got some free space

        if(context && context->playback && !context->playback->isPlaying)
            return;

        sleep(1);
    }
#ifdef DEBUG
    printf("from mkv: %s pts:%lld milliDuration:%lld\n",text,pts,milliDuration);
		printf("strdup in %s::%s:%d\n", FILENAME, __FUNCTION__,__LINE__);
#endif
    subPuffer[freeSubPointer].text = strdup(text);
    subPuffer[freeSubPointer].pts = pts;
    subPuffer[freeSubPointer].milliDuration = milliDuration;

    freeSubPointer++;
    freeSubPointer%=PUFFERSIZE;

}

int getNextSub(char ** text, unsigned long long int * pts, long int * milliDuration) {
#ifdef DEBUG
    printf("%s::%s\n",  FILENAME, __FUNCTION__);
#endif
    if (subPuffer[nextSubPointer].text == NULL || 
        subPuffer[nextSubPointer].pts == 0 || 
        subPuffer[nextSubPointer].milliDuration == 0) {

        return -1;
    }
#ifdef DEBUG
		printf("strdup in %s::%s:%d\n", FILENAME, __FUNCTION__,__LINE__);
#endif
    *text = strdup(subPuffer[nextSubPointer].text);
    free(subPuffer[nextSubPointer].text);
    subPuffer[nextSubPointer].text = NULL;

    *pts = subPuffer[nextSubPointer].pts;
    subPuffer[nextSubPointer].pts = 0;

    *milliDuration = subPuffer[nextSubPointer].milliDuration;
    subPuffer[nextSubPointer].milliDuration = 0;

    nextSubPointer++;
    nextSubPointer%=PUFFERSIZE;
#ifdef DEBUG
    printf("nextSubPointer %d\n",nextSubPointer);
#endif

    return 0;
} 

static void* Enigma2SubtitleThread(void* data) {
Context_t *context = (Context_t*) data;
#ifdef DEBUG
    printf("%s::%s\n",  FILENAME, __FUNCTION__);
#endif
    char *                  subText             = NULL;
    long int                subMilliDuration    = 0;
    unsigned long long int  subPts              = 0;
    unsigned long long int  Pts                 = 0;
    
    //wait a little, so isPlaying is set
    //sleep(1);
    while ( context->playback->isCreationPhase ) {
#ifdef DEBUG
		printf("%s::%s Thread waiting for end of init phase...\n", FILENAME, __FUNCTION__);
#endif
		}

    while ( context && 
            context->playback && 
            context->playback->isPlaying) {

        int curtrackid = -1;
        if (context && context->manager && context->manager->subtitle)
            context->manager->subtitle->Command(context, MANAGER_GET, &curtrackid);

        if (curtrackid >= 0) {
            if (getNextSub(&subText, &subPts, &subMilliDuration) != 0) {
                usleep(500000);
                continue;
            }

            if (context && context->playback)
                context->playback->Command(context, PLAYBACK_PTS, &Pts);
            else return NULL;

           if(Pts > subPts) {
                if(subText != NULL)
                    free(subText);
                continue;
            }
                
#ifdef DEBUG
            printf("%s::%s %llu < %llu\n", FILENAME, __FUNCTION__, Pts, subPts);
#endif

            while ( context && 
                    context->playback && 
                    context->playback->isPlaying && 
                    Pts < subPts) {

                unsigned long int diff = subPts - Pts;
                diff = (diff*1000)/90.0;
#ifdef DEBUG
                printf("DIFF: %d\n", diff);
#endif
                if(diff > 100)
                    usleep(diff);

                if (context && context->playback)
                    context->playback->Command(context, PLAYBACK_PTS, &Pts);
                else return NULL;

#ifdef DEBUG
                printf("%s::%s cur: %llu wanted: %llu\n", FILENAME, __FUNCTION__, Pts, subPts);
#endif
            }

            if (    context && 
                    context->playback && 
                    context->playback->isPlaying &&
                    subText != NULL ) {

                if(_fkt != NULL)
                  _fkt(subMilliDuration, strlen(subText), subText, _smp3);
#ifdef DEBUG
                else
                  printf("%s::%s \{%ld\} \[%d\] \"%s\"\n", FILENAME,__FUNCTION__, subMilliDuration, strlen(subText), subText);
#endif

                free(subText);
            }
                
        } else //Wait
            usleep(500000);

    }
    return NULL;
} 




static int Write(void* _context, unsigned char *PLAYERData, int DataLength, unsigned long long int Pts, unsigned char *Private, const int PrivateLength, float Duration, char * type) {
Context_t  * context = (Context_t  *) _context;
#ifdef DEBUG
	printf("%s::%s\n", FILENAME, __FUNCTION__);
#endif

	char * Encoding = NULL;
	char * Text = strdup((const char*) PLAYERData);

	context->manager->subtitle->Command(context, MANAGER_GETENCODING, &Encoding);

#ifdef DEBUG
	printf("%s::%s %s\n",       FILENAME, __FUNCTION__, Encoding);
	printf("%s::%s %s [%d]\n",  FILENAME, __FUNCTION__, Text, DataLength);
#endif

    if(     !strncmp("S_TEXT/SSA",  Encoding, 10) || 
            !strncmp("S_SSA",       Encoding, 5))
        Enigma2ParseSSA(&Text);
    else if(!strncmp("S_TEXT/ASS",  Encoding, 10) || 
            !strncmp("S_AAS",       Encoding, 5))
        Enigma2ParseASS(&Text);
    else if(!strncmp("S_TEXT/SRT",  Encoding, 10) || 
            !strncmp("S_SRT",       Encoding, 5))
        Enigma2ParseSRT(&Text);
    else
        ;
        
#ifdef DEBUG
    printf("%s::%s %s\n",  FILENAME, __FUNCTION__, Text);
#endif

    addSub(context, Text, Pts, Duration * 1000);
    free(Text);
    free(Encoding);

	return 0;
}

static int Enigma2Open(context) {
#ifdef DEBUG
    printf("%s::%s\n",  FILENAME, __FUNCTION__);
#endif
    //Reset all
    nextSubPointer = 0;
    freeSubPointer = 0;
    int i;
	for (i = 0; i < PUFFERSIZE; i++) {
        subPuffer[i].text          = NULL;
        subPuffer[i].pts           = 0;
        subPuffer[i].milliDuration = 0;
    }

    return 0;
}

static int Enigma2Close(Context_t* context) {
#ifdef DEBUG
    printf("%s::%s\n",  FILENAME, __FUNCTION__);
#endif
    //Reset all
    nextSubPointer = 0;
    freeSubPointer = 0;
    int i;
	for (i = 0; i < PUFFERSIZE; i++) {
        subPuffer[i].text          = NULL;
        subPuffer[i].pts           = 0;
        subPuffer[i].milliDuration = 0;
    }

    return 0;
}

static int Enigma2Play(Context_t* context) {
#ifdef DEBUG
    printf("%s::%s\n",  FILENAME, __FUNCTION__);
#endif
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create (&thread_sub, &attr, &Enigma2SubtitleThread, (void*) context);   

    return 0;
}

static int Enigma2Stop(context) {
#ifdef DEBUG
    printf("%s::%s\n",  FILENAME, __FUNCTION__);
#endif
    //pthread_join (&thread_sub, NULL, &sub_loop, NULL);

    return 0;
}

void Enigma2SignalConnect(void (*fkt) (long int, size_t, char *, void *))
{
#ifdef DEBUG
    printf("%s::%s\n",  FILENAME, __FUNCTION__);
#endif

    _fkt = fkt;
}

void Enigma2SignalConnectBuffer(void* smp3)
{
#ifdef DEBUG
    printf("%s::%s\n",  FILENAME, __FUNCTION__);
#endif

    _smp3 = smp3;   
}

static int Command(void  *_context, OutputCmd_t command, void * argument) {
Context_t  *context = (Context_t*) _context;
#ifdef DEBUG
	printf("%s::%s\n", FILENAME, __FUNCTION__);
#endif
	
	int ret = 0;

	switch(command) {
		case OUTPUT_OPEN: {
			Enigma2Open(context);
			break;
		}
		case OUTPUT_CLOSE: {
			Enigma2Close(context);
			break;
		}
		case OUTPUT_PLAY: {
			Enigma2Play(context);
			break;
		}
		case OUTPUT_STOP: {
			Enigma2Stop(context);
			break;
		}
		case OUTPUT_SWITCH: {
			//Enigma2Switch(context, (char*)argument);
			break;
		}
		case 222: { //Subtitle function register hack
		    Enigma2SignalConnect(argument);
		    break;
		}
		case 223: { //Subtitle buffer register hack
		    Enigma2SignalConnectBuffer(argument);
		    break;
		}
		/*case OUTPUT_FLUSH: {
			Enigma2Flush(context, (char*)argument);
			break;
		}
		case OUTPUT_PAUSE: {
			Enigma2Pause(context, (char*)argument);
			break;
		}
		case OUTPUT_CONTINUE: {
			Enigma2Continue(context, (char*)argument);
			break;
		}*/

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


static char *Enigma2Capabilitis[] = { "subtitle", NULL };

struct Output_s Enigma2Output = {
	"Enigma2",
	&Command,
	&Write,
	Enigma2Capabilitis,

};

