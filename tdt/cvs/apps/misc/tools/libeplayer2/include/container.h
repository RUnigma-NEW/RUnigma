#ifndef CONTAINER_H_
#define CONTAINER_H_

#include <stdio.h>

typedef enum { CONTAINER_INIT, CONTAINER_ADD, CONTAINER_CAPABILITIES, CONTAINER_PLAY, CONTAINER_STOP, CONTAINER_SEEK, CONTAINER_LENGTH, CONTAINER_DEL, CONTAINER_SWITCH_AUDIO, CONTAINER_SWITCH_SUBTITLE, CONTAINER_INFO } ContainerCmd_t;

typedef struct Container_s {
	char * Name;
	int (* Command) (/*Context_t*/void  *, ContainerCmd_t, void *);
	char ** Capabilities;

} Container_t;


extern Container_t MkvContainer;
extern Container_t AviContainer;
extern Container_t Mp4Container;
extern Container_t AUDIOContainer;
extern Container_t TSContainer;
extern Container_t MPGContainer;
extern Container_t ASFContainer;

static Container_t * AvailableContainer[] = {
    &MkvContainer,
    &AviContainer,
    &Mp4Container,
    &AUDIOContainer,
    &TSContainer,
    &MPGContainer,
    &ASFContainer,
    NULL
};

typedef struct ContainerHandler_s {
	char * Name;
	Container_t * selectedContainer;
    Container_t * textSrtContainer;
    Container_t * textSsaContainer;
	int (* Command) (/*Context_t*/void  *, ContainerCmd_t, void *);
} ContainerHandler_t;

#endif
