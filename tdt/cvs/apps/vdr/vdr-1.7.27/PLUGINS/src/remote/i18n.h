/*
 * Remote Control plugin for the Video Disk Recorder
 *
 * i18n.h: Internationalization
 *
 * See the README file for copyright information and how to reach the author.
 */


#ifndef __PLUGIN_REMOTE_I18N_H
#define __PLUGIN_REMOTE_I18N_H


#include <vdr/i18n.h>


#if APIVERSNUM <= 10506
#define trNOOP(s) (s)
extern const tI18nPhrase remotePhrases[];
#endif


#endif // __PLUGIN_REMOTE_I18N_H
