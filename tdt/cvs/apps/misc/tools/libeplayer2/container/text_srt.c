#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <dirent.h>
#include <pthread.h>
#include <errno.h>

#include <limits.h>

//#ifndef DEBUG
//#define DEBUG	// FIXME: until this is set properly by Makefile
//#endif

static const char FILENAME[] = "text_srt.c";
#define TEXTSRTOFFSET 100

#include "common.h"
static pthread_t thread_sub;

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct {
    char * File;
    int Id;
} SrtTrack_t;

#define TRACKWRAP 20
static SrtTrack_t * Tracks;
static int TrackCount = 0;
static int CurrentTrack = -1; //no as default.

static void SrtManagerAdd(Context_t  *context, SrtTrack_t track) {
#ifdef DEBUG
	printf("%s::%s %s %d\n", FILENAME, __FUNCTION__, track.File, track.Id);
#endif

	if (Tracks == NULL) {
		Tracks = malloc(sizeof(SrtTrack_t) * TRACKWRAP);
	}
	
	if (TrackCount < TRACKWRAP) {
#ifdef DEBUG
		printf("2strdup in %s::%s:%d\n", FILENAME, __FUNCTION__,__LINE__);
#endif
		Tracks[TrackCount].File       = strdup(track.File);
                Tracks[TrackCount].Id         = track.Id;
                TrackCount++;
	} else {
		//Track_t * tmp_Tracks = realoc(sizeof(Tracks));
	}
}

static char ** SrtManagerList(Context_t  *context) {
	char ** tracklist = NULL;
#ifdef DEBUG
	printf("%s::%s\n", FILENAME, __FUNCTION__);
#endif
	if (Tracks != NULL) {
	        char help[256];
		int i = 0, j = 0;
		tracklist = malloc(sizeof(char *) * ((TrackCount*2) + 1));
		for (i = 0, j = 0; i < TrackCount; i++, j+=2) {
#ifdef DEBUG
			printf("2strdup in %s::%s:%d\n", FILENAME, __FUNCTION__,__LINE__);
#endif
                sprintf(help, "%d", Tracks[i].Id);
                tracklist[j]    = strdup(help);
		tracklist[j+1]  = strdup(Tracks[i].File);
	   }
           tracklist[j] = NULL;
	}

	return tracklist;
}

static void SrtManagerDel(Context_t * context) {

    int i = 0;
    if(Tracks != NULL) {
        for (i = 0; i < TrackCount; i++) {
            free(Tracks[i].File);
            Tracks[i].File = NULL;
	}
	free(Tracks);
        Tracks = NULL;
    }

    TrackCount = 0;
    CurrentTrack = -1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

static int getExtension(char fileName[PATH_MAX], char extension[PATH_MAX]) {
#ifdef DEBUG
	printf("%s::%s\n", FILENAME, __FUNCTION__);
#endif

	int i = 0;
	int extensionLength = -1;
	int stringLength = (int) strlen(fileName);

	for (i = 0; stringLength - i > 0; i++) {
		if (fileName[stringLength - i - 1] == '.') {
			extensionLength = i;
			strncpy(extension, &fileName[stringLength - i], extensionLength);
			extension[extensionLength] = '\0';
			break;
		}
	}
	return extensionLength;
}

static int getParentFolder(char fileName[PATH_MAX], char folder[PATH_MAX]) {
#ifdef DEBUG
	printf("%s::%s\n", FILENAME, __FUNCTION__);
#endif

	int i = 0;
	int folderLength = -1;
	int stringLength = (int) strlen(fileName);

	for (i = 0; stringLength - i > 0; i++) {
		if (fileName[stringLength - i - 1] == '/') {
			folderLength = stringLength - i - 1;
			strncpy(folder, fileName, folderLength);
			folder[folderLength] = '\0';
			break;
		}
	}
	return folderLength;
}

static int compare(char fileNameA[PATH_MAX], char fileNameB[PATH_MAX]) {
	int i = 0;
	int stringLengthA = strlen(fileNameA);
	int stringLengthB = strlen(fileNameB);

	if(stringLengthA > stringLengthB)
		return -1;

	for(i = 0; i < stringLengthA; i++)
		if(fileNameA[i] != fileNameB[i])
			break;

	if (i == stringLengthA)
		i = 0;

	return i;
}

static void SrtGetSubtitle(Context_t  *context, char * _fileName) {
#ifdef DEBUG
    printf("%s::%s\n", FILENAME, __FUNCTION__);
#endif

    struct dirent *dirzeiger;
    DIR  *  dir;
    int     i                    = TEXTSRTOFFSET;

    int     absFileNameLength          = 0;
    int     absFileNameFolderLength    = 0;
    int     relFileNameLength     = 0;
    int     relFileNameExtensionLength = 0;
    int     relFileNameShortLength = 0;

    char absFileName[PATH_MAX];
    char absFileNameFolder[PATH_MAX];
    char relFileName[PATH_MAX];
    char relFileNameExtension[PATH_MAX];
    char relFileNameShort[PATH_MAX];
    char relFileNameSubExtension[PATH_MAX];

    absFileName[0] = '\0';
    absFileNameFolder[0] = '\0';
    relFileName[0] = '\0';
    relFileNameExtension[0] = '\0';
    relFileNameShort[0] = '\0';
    relFileNameSubExtension[0] = '\0';

    // Absolut File Name
    absFileNameLength = strlen(_fileName);
    strncpy(absFileName, _fileName, absFileNameLength);
    absFileName[absFileNameLength] = '\0';

    // Absolut Folder Path
    absFileNameFolderLength = getParentFolder(absFileName, absFileNameFolder);

    // Relativ File Name
    relFileNameLength = absFileNameLength - absFileNameFolderLength - 1;
    strncpy(relFileName, &absFileName[absFileNameFolderLength + 1], relFileNameLength);
    relFileName[relFileNameLength] = '\0';

    // Extension of Relativ File Name
    relFileNameExtensionLength = getExtension(relFileName, relFileNameExtension);

    // Relativ File Name without Extension
    relFileNameShortLength = relFileNameLength - relFileNameExtensionLength - 1 /* . */;
    strncpy(relFileNameShort, relFileName, relFileNameShortLength);
    relFileNameShort[relFileNameShortLength] = '\0';

    if((dir = opendir(absFileNameFolder)) != NULL) {
        while((dirzeiger = readdir(dir)) != NULL) {
#ifdef DEBUG
            printf("%s\n",(*dirzeiger).d_name);
#endif

            int relSubtitleFileNameLength = 0;
            int relSubtitleFileNameExtensionLength = 0;

            char relSubtitleFileName[PATH_MAX];
            char relSubtitleFileNameExtension[PATH_MAX];

            // Relativ Subtitle File Name
            relSubtitleFileNameLength = strlen((*dirzeiger).d_name);
            strncpy(relSubtitleFileName, (*dirzeiger).d_name, relSubtitleFileNameLength);
            relSubtitleFileName[relSubtitleFileNameLength] = '\0';

            // Extension of Relativ Subtitle File Name
            relSubtitleFileNameExtensionLength = getExtension(relSubtitleFileName, relSubtitleFileNameExtension);

            if(relSubtitleFileNameExtensionLength <= 0)
            	continue;

            if(!strncmp(relSubtitleFileNameExtension, "srt", 3)) {

            	if(compare(relFileNameShort, relSubtitleFileName) == 0) {
#ifdef DEBUG
                    printf("%s\n", relSubtitleFileName);
#endif

                    int absSubtitleFileNameLength = 0;
                    int relSubtitleFileNameShortLength = 0;
                    int relSubtitleFileNameSubExtensionLength = 0;

                    char absSubtitleFileName[PATH_MAX];
            	    char relSubtitleFileNameShort[PATH_MAX];
            	    char relSubtitleFileNameSubExtension[PATH_MAX];

            	    relSubtitleFileNameShort[0] = '\0';
            	    relSubtitleFileNameSubExtension[0] = '\0';

                    // Relativ File Name without Extension
                    relSubtitleFileNameShortLength = relSubtitleFileNameLength - relSubtitleFileNameExtensionLength - 1 /* . */;
                    strncpy(relSubtitleFileNameShort, relSubtitleFileName, relSubtitleFileNameShortLength);
                    relSubtitleFileNameShort[relSubtitleFileNameShortLength] = '\0';

            	    // SubExtension of Relativ File Name
            	    relSubtitleFileNameSubExtensionLength = getExtension(relSubtitleFileNameShort, relSubtitleFileNameSubExtension);

            	    if(relSubtitleFileNameSubExtensionLength <= 0 || relSubtitleFileNameSubExtensionLength > 3) {
            	    	strncpy(relSubtitleFileNameSubExtension, "und", 3);
            	    	relSubtitleFileNameSubExtension[3] = '\0';
            	    }

            	    absSubtitleFileNameLength = absFileNameFolderLength;
            	    strncpy(absSubtitleFileName, absFileNameFolder, absFileNameFolderLength);
            	    absSubtitleFileName[absSubtitleFileNameLength] = '/';
            	    absSubtitleFileNameLength++;
            	    strncpy(&absSubtitleFileName[absSubtitleFileNameLength], relSubtitleFileName, relSubtitleFileNameLength);
            	    absSubtitleFileNameLength += relSubtitleFileNameLength;
            	    absSubtitleFileName[absSubtitleFileNameLength] = '\0';

            	    printf("SRT: %s [%s]\n", relSubtitleFileNameSubExtension, relSubtitleFileNameShort);
            	    printf("\t->%s\n", absSubtitleFileName);

            	    SrtTrack_t SrtSubtitle = {
            	        absSubtitleFileName,
            	        i,
            	    };
            	    SrtManagerAdd(context, SrtSubtitle);
            	    
            	    Track_t Subtitle = {
            	        relSubtitleFileNameSubExtension,
            	        "S_TEXT/SRT",
            	        i++,
            	    };
            	    context->manager->subtitle->Command(context, MANAGER_ADD, &Subtitle);

            	}
            }
        }
        closedir(dir);
    }

}




FILE * fsub = NULL;
#define MAXLINELENGTH 80

static int SrtOpenSubtitle(Context_t *context, int trackid) {

    if(trackid < TEXTSRTOFFSET) {
        return -1;
    }

    trackid %= TEXTSRTOFFSET;

#ifdef DEBUG
    printf("%s::%s %s\n", FILENAME, __FUNCTION__, Tracks[trackid].File);
#endif

    fsub = fopen(Tracks[trackid].File, "rb");
#ifdef DEBUG
    printf("%s::%s %s\n", FILENAME, __FUNCTION__, fsub?"fsub!=NULL":"fsub==NULL");
#endif
    if(!fsub)
        return -1;

    return 0;
}

static int SrtCloseSubtitle(Context_t *context) {

    if(fsub)
        fclose(fsub);

    fsub = NULL;

    return 0;
}


static void* SrtSubtitleThread(void *data) {
Context_t *context = (Context_t*) data;
#ifdef DEBUG
		printf("%s::%s\n", FILENAME, __FUNCTION__);
#endif
    int pos = 0;
    char  Data[MAXLINELENGTH];
    unsigned long long int Pts = 0;
    float Duration = 0;
    char * Text = NULL;

    while(context->playback->isPlaying && fsub && fgets(Data, MAXLINELENGTH, fsub)) {
#ifdef DEBUG
				printf("%s::%s pos=%d\n", FILENAME, __FUNCTION__, pos);
#endif
        if(pos == 0) {
            //int numScene = 0;
            //ret = sscanf(line, "%li", &numScene);
            pos++; 

        } else if(pos == 1) {
            int ret, horIni, minIni, secIni, milIni, horFim, minFim, secFim, milFim;

            ret = sscanf(Data, "%d:%d:%d,%d --> %d:%d:%d,%d", &horIni, &minIni, &secIni, &milIni, &horFim, &minFim, &secFim, &milFim);

            Pts = (horIni*3600 + minIni*60 + secIni)*1000 + milIni;
	        Duration = ((horFim*3600 + minFim*60 + secFim) * 1000  + milFim - Pts) / 1000.0;
	        Pts = Pts * 90;

            pos++;

        } else if(pos == 2) {
#ifdef DEBUG
            printf("Data[0] = %d \'%c\'\n", Data[0], Data[0]);
#endif
            if(Data[0] == '\n' || Data[0] == '\0' || Data[0] == 13 /* ^M */) {
#ifdef DEBUG
                printf("--> Text= \"%s\"\n", Text);
#endif
                context->output->subtitle->Write(context, (unsigned char*) Text, strlen(Text), Pts, NULL, 0, Duration, "subtitle");
#ifdef DEBUG
                printf("<-- Text= \"%s\"\n", Text);
#endif
                free(Text);
                Text = NULL;
                pos = 0;
                continue;
            }
            
            if(!Text) {
#ifdef DEBUG
								printf("strdup in %s::%s:%d\n", FILENAME, __FUNCTION__,__LINE__);
#endif
                Text = strdup(Data);
                
                Text[-1] = '\0';
            } else {
                int length = strlen(Text) /* \0 -> \n */ + strlen(Data) + 2 /* \0 */;
#ifdef DEBUG
                printf("Alloc: %d\n", length);
								printf("strdup in %s::%s:%d\n", FILENAME, __FUNCTION__,__LINE__);
#endif
                char * tmpText = strdup(Text);
                free(Text);
                Text = (char*)malloc(length);
                strcpy(Text, tmpText);
                strcat(Text, "\n");
                strcat(Text, Data);
                free(tmpText);
            }
        }
    }
    if(Text) {
        free(Text);
        Text = NULL;
    }
    return NULL;
}

static int SrtPlay(Context_t *context) {
#ifdef DEBUG
    printf("%s::%s\n",  FILENAME, __FUNCTION__);
#endif

    int curtrackid = -1;
    context->manager->subtitle->Command(context, MANAGER_GET, &curtrackid);

    if(SrtOpenSubtitle(context, curtrackid) == 0){
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        pthread_create (&thread_sub, &attr, &SrtSubtitleThread, context);   
    }
    return 0;
}

static int SrtSwitchSubtitle(Context_t *context, int* arg) {

    SrtCloseSubtitle(context);

    if(SrtOpenSubtitle(context, *arg)==0){
	 pthread_attr_t attr;
	 pthread_attr_init(&attr);
	 pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
         pthread_create (&thread_sub, &attr, &SrtSubtitleThread, context);   
    }
    return 0;
}

static int SrtDel(Context_t *context) {

    SrtCloseSubtitle(context);
    SrtManagerDel(context);
    return 0;
}

static int Command(void  *_context, ContainerCmd_t command, void * argument) {
Context_t  *context = (Context_t*) _context;
#ifdef DEBUG
	printf("%s::%s\n", FILENAME, __FUNCTION__);
#endif

	switch(command) {
		case CONTAINER_INIT: {
			char * filename = (char *)argument;
			SrtGetSubtitle(context, filename);
			break;
		}
		case CONTAINER_PLAY: {
			SrtPlay(context);
			break;
		}
		case CONTAINER_DEL: {
			SrtDel(context);
			break;
		}
		case CONTAINER_SWITCH_SUBTITLE: {
#ifdef DEBUG
        printf("%s::%s CONTAINER_SWITCH_SUBTITLE id=%d\n", FILENAME, __FUNCTION__, *((int*) argument));
#endif

		    SrtSwitchSubtitle(context, (int*) argument);
			break;
		}
		default:
#ifdef DEBUG
			printf("%s::%s ConatinerCmd not supported! %d\n", FILENAME, __FUNCTION__, command);
#endif
			break;
	}

	return 0;
}

static char *SrtCapabilities[] = { "srt", NULL };

Container_t SrtContainer = {
	"SRT",
	&Command,
	SrtCapabilities,
};
