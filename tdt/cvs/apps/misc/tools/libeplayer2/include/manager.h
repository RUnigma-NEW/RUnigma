#ifndef MANAGER_H_
#define MANAGER_H_

#include <stdio.h>

typedef enum {	MANAGER_ADD, MANAGER_LIST, MANAGER_GET, MANAGER_GETNAME, MANAGER_SET, MANAGER_GETENCODING, MANAGER_DEL } ManagerCmd_t;

typedef struct Track_s {
	char * Name;
	char * Encoding;
	int Id;
} Track_t;

typedef struct Manager_s {
	char * Name;
	int (* Command) (/*Context_t*/void  *, ManagerCmd_t, void *);
	char ** Capabilities;

} Manager_t;

typedef struct ManagerHandler_s {
	char * Name;
	Manager_t * audio;
	Manager_t * video;
	Manager_t * subtitle;
} ManagerHandler_t;

#endif
