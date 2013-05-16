#include "manager.h"

extern Manager_t AudioManager;
extern Manager_t VideoManager;
extern Manager_t SubtitleManager;

ManagerHandler_t ManagerHandler = {
	"ManagerHandler",
	&AudioManager,
	&VideoManager,
	&SubtitleManager,
};
