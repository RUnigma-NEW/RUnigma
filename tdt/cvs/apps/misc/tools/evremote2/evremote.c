/*
 * evremote.c
 *
 * (c) 2009 donald@teamducktales
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#include <termios.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include <time.h>

#include <semaphore.h>
#include <pthread.h>

#include "global.h"
#include "remotes.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

int processSimple (Context_t * context, int argc, char* argv[]) {

    int         vCurrentCode      = -1;

    if (((RemoteControl_t*)context->r)->Init)
       context->fd = (((RemoteControl_t*)context->r)->Init)(context, argc, argv);
    else
    {
       fprintf(stderr, "driver does not support init function\n");
       exit(1);
    }

    if (context->fd < 0)
    {
       fprintf(stderr, "error in device initialization\n");
       exit(1);
    }

    while ( true ) {

        //wait for new command
        if (((RemoteControl_t*)context->r)->Read)
           vCurrentCode = ((RemoteControl_t*)context->r)->Read(context);
        if(vCurrentCode <= 0)
            continue;

        //activate visual notification
        if (((RemoteControl_t*)context->r)->Notification)
           ((RemoteControl_t*)context->r)->Notification(context, 1);

        //Check if tuxtxt is running
        if (checkTuxTxt(vCurrentCode) == false)
           sendInputEvent(vCurrentCode);

        //deactivate visual notification
        if (((RemoteControl_t*)context->r)->Notification)
           ((RemoteControl_t*)context->r)->Notification(context, 0);
    }

    if (((RemoteControl_t*)context->r)->Shutdown)
       ((RemoteControl_t*)context->r)->Shutdown(context);
    else
       close(context->fd);


    return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

static unsigned int gBtnPeriod  = 100;
static unsigned int gBtnDelay = 10;

static struct timeval profilerLast;

static unsigned int gKeyCode = 0;
static unsigned int gNextKey = 0;
static unsigned int gNextKeyFlag = 0xFF;

static sem_t keydown_sem;
static pthread_t keydown_thread;

#ifdef NITS_SEM_PATCH_WOULD_WORK
// Your patch crashes the long key press detection.
// You know there was a reason why I didn't do it this way.
// Sem_getvaulue and counting sems does not work as someone would initialy expect.
#else
static unsigned char keydown_sem_helper = 1;
#endif
static void sem_up(void) {
#ifdef NITS_SEM_PATCH_WOULD_WORK
  int sem_val;
  sem_getvalue(&keydown_sem, &sem_val);
  if(sem_val <= 0) {
#else
  if(keydown_sem_helper == 0) {
#endif
    printf("[SEM] UP\n");
    sem_post(&keydown_sem);
#ifdef NITS_SEM_PATCH_WOULD_WORK
#else
    keydown_sem_helper = 1;
#endif
  }
}

static void sem_down(void) {
  printf("[SEM] DOWN\n");
#ifdef NITS_SEM_PATCH_WOULD_WORK
#else
  keydown_sem_helper = 0;
#endif
  sem_wait(&keydown_sem);
}

int
timeval_subtract (result, x, y)
    struct timeval *result, *x, *y;
{
 /* Perform the carry for the later subtraction by updating y. */
 if (x->tv_usec < y->tv_usec) {
   int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
   y->tv_usec -= 1000000 * nsec;
   y->tv_sec += nsec;
 }
 if (x->tv_usec - y->tv_usec > 1000000) {
   int nsec = (x->tv_usec - y->tv_usec) / 1000000;
   y->tv_usec += 1000000 * nsec;
   y->tv_sec -= nsec;
 }

 /* Compute the time remaining to wait.
    tv_usec is certainly positive. */
 result->tv_sec = x->tv_sec - y->tv_sec;
 result->tv_usec = x->tv_usec - y->tv_usec;

 /* Return 1 if result is negative. */
 return x->tv_sec < y->tv_sec;
}

static long diffMilli(struct timeval from, struct timeval to)
{
  struct timeval diff;
  timeval_subtract(&diff, &to, &from);
  return (long)(diff.tv_sec*1000 + diff.tv_usec/1000);
}


void *detectKeyUpTask(void* dummy);

int processComplex (Context_t * context, int argc, char* argv[]) {

    int         vCurrentCode      = -1;

    printf("%s >\n", __func__);

    if (((RemoteControl_t*)context->r)->Init)
       context->fd = (((RemoteControl_t*)context->r)->Init)(context, argc, argv);
    else
    {
       fprintf(stderr, "driver does not support init function\n");
       exit(1);
    }

    if (context->fd < 0)
    {
       fprintf(stderr, "error in device initialization\n");
       exit(1);
    }

    if(((RemoteControl_t*)context->r)->LongKeyPressSupport != NULL)
    {
        tLongKeyPressSupport lkps = *((RemoteControl_t*)context->r)->LongKeyPressSupport;
        gBtnPeriod = lkps.period;
        gBtnDelay = lkps.delay;
        printf("Using period=%d delay=%d\n", gBtnPeriod, gBtnDelay);
    }

    setInputEventRepeatRate(500, 200);

    sem_init(&keydown_sem, 0, 0);
    pthread_create(&keydown_thread, NULL, detectKeyUpTask, context);

    struct timeval time;
    gettimeofday(&profilerLast, NULL);

    while ( true ) {

        //wait for new command
        if (((RemoteControl_t*)context->r)->Read)
           vCurrentCode = ((RemoteControl_t*)context->r)->Read(context);
        if(vCurrentCode <= 0)
            continue;

        gKeyCode = vCurrentCode & 0xFFFF;
        unsigned int nextKeyFlag = (vCurrentCode>>16) & 0xFFFF;
        if(gNextKeyFlag != nextKeyFlag)
        {
          gNextKey++;
          gNextKey%=20;
          gNextKeyFlag = nextKeyFlag;
        }

        gettimeofday(&time, NULL);
        printf("**** %12u %d ****\n", diffMilli(profilerLast, time), gNextKey);
        profilerLast = time;

        sem_up();

    }

    if (((RemoteControl_t*)context->r)->Shutdown)
       ((RemoteControl_t*)context->r)->Shutdown(context);
    else
       close(context->fd);

    printf("%s <\n", __func__);

    return 0;
}

void *detectKeyUpTask(void* dummy)
{
  Context_t * context = (Context_t*) dummy;
  struct timeval time;
  while (1)
  {
    unsigned int keyCode = 0;
    unsigned int nextKey = 0;
    sem_down(); // Wait till the next keypress
    while(1)
    {
      int tux = 0;

      keyCode = gKeyCode;
      nextKey = gNextKey;

      //activate visual notification
      if (((RemoteControl_t*)context->r)->Notification)
         ((RemoteControl_t*)context->r)->Notification(context, 1);

      printf("KEY_PRESS - %02x %d\n", keyCode, nextKey);

      //Check if tuxtxt is running
      tux = checkTuxTxt(keyCode);
      if (tux == false)
         sendInputEventT(INPUT_PRESS, keyCode);

      //usleep(gBtnDelay*1000);
      while (gKeyCode && nextKey == gNextKey)
      {
        gettimeofday(&time, NULL);
        unsigned int sleep = gBtnPeriod + gBtnDelay - diffMilli(profilerLast, time);

        if (sleep > (gBtnPeriod + gBtnDelay))
             sleep = (gBtnPeriod + gBtnDelay);

        printf("++++ %12u ms ++++\n", diffMilli(profilerLast, time));
        gKeyCode = 0;
        usleep(sleep*1000);
      }
      printf("KEY_RELEASE - %02x %02x %d %d CAUSE=%s\n", keyCode, gKeyCode, nextKey, gNextKey, (gKeyCode==0)?"Timeout":"New key");

      //Check if tuxtxt is running
      if (tux == false)
          sendInputEventT(INPUT_RELEASE, keyCode);

      //deactivate visual notification
      if (((RemoteControl_t*)context->r)->Notification)
         ((RemoteControl_t*)context->r)->Notification(context, 0);

      gettimeofday(&time, NULL);
      printf("---- %12u ms ----\n", diffMilli(profilerLast, time));

      if (nextKey == gNextKey)
        break;
    }
  }

  return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////


int getModel()
{
    int         vFd             = -1;
    const int   cSize           = 128;
    char        vName[129]      = "Unknown";
    int         vLen            = -1;
    eBoxType    vBoxType        = Hl101;

    printf("vBoxType: %d\n", vBoxType);

    return vBoxType;
}

void ignoreSIGPIPE()
{
       struct sigaction vAction;

       vAction.sa_handler = SIG_IGN;

       sigemptyset(&vAction.sa_mask);
       vAction.sa_flags = 0;
       sigaction(SIGPIPE,  &vAction, (struct sigaction*)NULL);
}

int main (int argc, char* argv[])
{
    
    freopen("/dev/console", "a", stdout);

    eBoxType vBoxType = getModel();
    Context_t context;

    /* Dagobert: if tuxtxt closes the socket while
     * we are writing a sigpipe occures which kills
     * evremote. so lets ignore it ...
     */
    ignoreSIGPIPE();

        if(!getEventDevice())
        {
            printf("unable to open event device\n");
            return 5;
        }
    selectRemote(&context, vBoxType);

    printf("Selected Remote: %s\n", ((RemoteControl_t*)context.r)->Name);

    if(((RemoteControl_t*)context.r)->RemoteControl != NULL) {
        printf("RemoteControl Map:\n");
        printKeyMap((tButton*)((RemoteControl_t*)context.r)->RemoteControl);
    }

    if(((RemoteControl_t*)context.r)->Frontpanel != NULL) {
        printf("Frontpanel Map:\n");
        printKeyMap((tButton*)((RemoteControl_t*)context.r)->Frontpanel);
    }

    const int cMaxButtonExtension = 128; // Up To 128 Extension Buttons
    tButton vButtonExtension[cMaxButtonExtension];
    int vButtonExtensionCounter = 0;

    if (argc >= 2) {
        if (argc == 3 && !strncmp(argv[1], "-r", 2)) {
            char   vKeyName[64];
            char   vKeyWord[64];
            int    vKeyCode;
            char * vRemoteFile  = argv[2];
            FILE * vRemoteFileD = NULL;

            vRemoteFileD = fopen(vRemoteFile, "r");
            if (vRemoteFileD != NULL) {
                while ( fscanf(vRemoteFileD,"%s %s %d", vKeyName, vKeyWord, &vKeyCode) == 3 ) {
                    strncpy(vButtonExtension[vButtonExtensionCounter].KeyName, vKeyName, 20);
                    strncpy(vButtonExtension[vButtonExtensionCounter].KeyWord, vKeyWord, 2);
                    vButtonExtension[vButtonExtensionCounter].KeyCode = vKeyCode;
                    vButtonExtensionCounter++;
                    if (vButtonExtensionCounter + 1 == cMaxButtonExtension)
                        break;
                }
                fclose(vRemoteFileD);

                strncpy(vButtonExtension[vButtonExtensionCounter].KeyName, "\0", 1);
                strncpy(vButtonExtension[vButtonExtensionCounter].KeyWord, "\0", 1);
                vButtonExtension[vButtonExtensionCounter].KeyCode = KEY_NULL;

                printf("RemoteControl Extension Map:\n");
                printKeyMap(vButtonExtension);
            }
        }
    }

    if (vButtonExtensionCounter > 0)
        ((RemoteControl_t*)context.r)->RemoteControl = vButtonExtension;
    // TODO
    //if(((RemoteControl_t*)context.r)->RemoteControl == NULL && vButtonExtensionCounter > 0)
        //((RemoteControl_t*)context.r)->RemoteControl = vButtonExtension;
    //else if (vButtonExtensionCounter > 0) {
        //int vRemoteControlSize    = sizeof(((RemoteControl_t*)context.r)->RemoteControl) / sizeof(tButton);
        //int vRemoteControlExtSize = vButtonExtensionCounter;
        //((RemoteControl_t*)context.r)->RemoteControl = malloc((vRemoteControlSize + vRemoteControlExtSize - 1)*sizeof(tButton));
    //}

    //printf("RemoteControl Map:\n");
    //printKeyMap((tButton*)((RemoteControl_t*)context.r)->RemoteControl);

    printf("Supports Long KeyPress: %d\n", ((RemoteControl_t*)context.r)->supportsLongKeyPress);
    if(((RemoteControl_t*)context.r)->supportsLongKeyPress)
      processComplex(&context, argc, argv);
    else
      processSimple(&context, argc, argv);

    return 0;
}
