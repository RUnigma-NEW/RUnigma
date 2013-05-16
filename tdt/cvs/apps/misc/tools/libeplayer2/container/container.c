#include <stdio.h>
#include <string.h>

#include "container.h"
#include "common.h"

static const char FILENAME[] = "container.c";

static void printContainerCapabilities() {
	int i, j;
	printf("%s::%s\n", FILENAME, __FUNCTION__);
	printf("Capabilities: ");
	for (i = 0; AvailableContainer[i] != NULL; i++)	
		for (j = 0; AvailableContainer[i]->Capabilities[j] != NULL; j++)
			printf("%s ", AvailableContainer[i]->Capabilities[j]);
	printf("\n");
}

static int selectContainer(Context_t  *context, char * extension) {
	int i, j;
	int ret = -1;
	
	printf("%s::%s\n", FILENAME, __FUNCTION__);
	for (i = 0; AvailableContainer[i] != NULL; i++)	
		for (j = 0; AvailableContainer[i]->Capabilities[j] != NULL; j++)
			if (!strcasecmp(AvailableContainer[i]->Capabilities[j], extension)) {
				context->container->selectedContainer = AvailableContainer[i];
				printf("Selected Container: %s\n", context->container->selectedContainer->Name);
				ret = 0;
				break;
			}
	
	if (ret != 0) {
		printf("No Container found :-(\n");
	}
	
	return ret;
}


static int Command(Context_t  *context, ContainerCmd_t command, void * argument) {
	printf("%s::%s\n", FILENAME, __FUNCTION__);
	
	int ret = 0;

	switch(command) {
		case CONTAINER_ADD: {
			ret = selectContainer(context, (char*) argument);
			break;
		}
		case CONTAINER_CAPABILITIES: {
			printContainerCapabilities();
			break;
		}
		case CONTAINER_DEL: {
			context->container->selectedContainer = NULL;
			break;
		}
		default:
			printf("%s::%s ContainerCmd %d not supported!\n", FILENAME, __FUNCTION__, command);
			break;
	}

	return ret;
}

extern Container_t SrtContainer;
extern Container_t SsaContainer;

ContainerHandler_t ContainerHandler = {
	"Output",
	NULL,
    &SrtContainer,
    &SsaContainer,
	Command,
};
