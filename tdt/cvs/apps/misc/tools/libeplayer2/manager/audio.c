#include <stdlib.h>
#include <string.h>

#include "manager.h"
#include "common.h"

static const char FILENAME[] = "audio.c";

#define TRACKWRAP 20
static Track_t * Tracks;
static int TrackCount = 0;
static int CurrentTrack = 0; //TRACK[0] as default.

static void ManagerAdd(Context_t  *context, Track_t track) {
	printf("%s::%s:%d\n", FILENAME, __FUNCTION__,__LINE__);
	if (Tracks == NULL) {
		Tracks = malloc(sizeof(Track_t) * TRACKWRAP);
	}
	
	if (TrackCount < TRACKWRAP) {
		Tracks[TrackCount].Name       = strdup(track.Name);
        	Tracks[TrackCount].Encoding   = strdup(track.Encoding);
        	Tracks[TrackCount].Id         = track.Id;
        	TrackCount++;
	} else {
		//Track_t * tmp_Tracks = realoc(sizeof(Tracks));
	}

	if (TrackCount > 0)
		context->playback->isAudio = 1;
}

static char ** ManagerList(Context_t  *context) {
	char ** tracklist = NULL;
	printf("%s::%s\n", FILENAME, __FUNCTION__);
	if (Tracks != NULL) {
		int i = 0, j = 0;
		tracklist = malloc(sizeof(char *) * ((TrackCount*2) + 1));
		for (i = 0, j = 0; i < TrackCount; i++, j+=2) {
			tracklist[j]    = strdup(Tracks[i].Name);
            		tracklist[j+1]  = strdup(Tracks[i].Encoding);
		}
        tracklist[j] = NULL;
	}

	return tracklist;
}

static void ManagerDel(Context_t * context) {

    int i = 0;
    if(Tracks != NULL) {
        for (i = 0; i < TrackCount; i++) {
	    free(Tracks[i].Name);
            free(Tracks[i].Encoding);
            Tracks[i].Name = NULL;
            Tracks[i].Encoding = NULL;
	}
	free(Tracks);
        Tracks = NULL;
    }

    TrackCount = 0;
    CurrentTrack = 0;
    context->playback->isAudio = 0;
}


static int Command(Context_t  *context, ManagerCmd_t command, void * argument) {
	//printf("%s::%s\n", FILENAME, __FUNCTION__);

	switch(command) {
		case MANAGER_ADD: {
			Track_t * track = argument;
			ManagerAdd(context, *track);
			break;
		}
		case MANAGER_LIST: {
			*((char***)argument) = (char **)ManagerList(context);
			break;
		}
		case MANAGER_GET: {
			printf("%s::%s MANAGER_GET\n", FILENAME, __FUNCTION__);
			if (TrackCount > 0)
				*((int*)argument) = (int)Tracks[CurrentTrack].Id;
			else 
				*((int*)argument) = (int)-1;
			break;
		}
		case MANAGER_GETENCODING: {
			if (TrackCount > 0)
				*((char**)argument) = (char *)strdup(Tracks[CurrentTrack].Encoding);
			else 
				*((char**)argument) = (char *)strdup("");
			break;
		}
		case MANAGER_GETNAME: {
			if (TrackCount > 0)
				*((char**)argument) = (char *)strdup(Tracks[CurrentTrack].Name);
			else 
				*((char**)argument) = (char *)strdup("");
			break;
		}
		case MANAGER_SET: {
			int id = *((int*)argument);
			printf("%s::%s MANAGER_SET id=%d\n", FILENAME, __FUNCTION__, id);
			if (id < TrackCount)
				CurrentTrack = id;
			break;
		}
		case MANAGER_DEL: {
			ManagerDel(context);
			break;
		}
		default:
			printf("%s::%s ContainerCmd %d not supported!\n", FILENAME, __FUNCTION__, command);
			break;
	}

	return 0;
}


struct Manager_s AudioManager = {
	"Audio",
	&Command,
	NULL,

};
