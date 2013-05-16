/*
	LCD-Daemon  -   DBoxII-Project

	Copyright (C) 2001 Steffen Hehn 'McClean'
	Homepage: http://dbox.cyberphoria.org/



	License: GPL

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __vfd__
#define __vfd__

//#define VFD_UPDATE

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef VFD_UPDATE
// TODO Why is USE_FILE_OFFSET64 not defined, if file.h is included here????
#ifndef __USE_FILE_OFFSET64
#define __USE_FILE_OFFSET64 1
#endif
#include "driver/file.h"
#endif // LCD_UPDATE

#include <pthread.h>
#include <string>

typedef enum {
/*** just not complete implemented
#if defined(PLATFORM_OCTAGON1008) || defined(PLATFORM_FORTIS_HDBOX)
// AV700/0
	ICON_STANDBY = 0x10,
	ICON_SAT,
	ICON_REC,
	ICON_TIMESHIFT,
	ICON_TIMER,
	ICON_HD,
	ICON_USB,
	ICON_SCRAMBLED,
	ICON_DOLBY,
	ICON_MUTE,
	ICON_TUNER1,
	ICON_TUNER2,
	ICON_MP3,
	ICON_REPEAT,
	ICON_Play,
	VFD_ICON_PAUSE,
	ICON_TER,
	ICON_FILE,
	ICON_480i,
	ICON_480p,
	ICON_576i,
	ICON_576p,
	ICON_720p,
	ICON_1080i,
	ICON_1080p,
	ICON_Play_1,
// for Neutrino
	VFD_ICON_USB = ICON_USB,
	VFD_ICON_REC = ICON_REC,
	VFD_ICON_CLOCK = ICON_TIMER,
	VFD_ICON_HD = ICON_HD,
	VFD_ICON_LOCK = ICON_SCRAMBLED,
	VFD_ICON_DD = ICON_DOLBY,
	VFD_ICON_MUTE = ICON_MUTE,
	VFD_ICON_MP3 = ICON_MP3,
	VFD_ICON_PLAY = ICON_Play,

	VFD_ICON_HDD,
	VFD_ICON_MUSIC,
	VFD_ICON_MAIL,
	VFD_ICON_FF,
	VFD_ICON_FR,

#elif defined(PLATFORM_UFS910) || defined(PLATFORM_FLASH_UFS910)
// UFS910
	VFD_ICON_USB = 0x10,
	VFD_ICON_HD,
	VFD_ICON_HDD,
	VFD_ICON_LOCK,
	VFD_ICON_BT,
	VFD_ICON_MP3,
	VFD_ICON_MUSIC,
	VFD_ICON_DD,
	VFD_ICON_MAIL,
	VFD_ICON_MUTE,
	VFD_ICON_PLAY,
	VFD_ICON_PAUSE,
	VFD_ICON_FF,
	VFD_ICON_FR,
	VFD_ICON_REC,
	VFD_ICON_CLOCK,

#else
***/
// old
	VFD_ICON_BAR8	= 0x00000004,
	VFD_ICON_BAR7	= 0x00000008,
	VFD_ICON_BAR6	= 0x00000010,
	VFD_ICON_BAR5	= 0x00000020,
	VFD_ICON_BAR4	= 0x00000040,
	VFD_ICON_BAR3	= 0x00000080,
	VFD_ICON_BAR2	= 0x00000100,
	VFD_ICON_BAR1	= 0x00000200,
	VFD_ICON_FRAME	= 0x00000400,
	VFD_ICON_HDD	= 0x00000800,
	VFD_ICON_MUTE	= 0x00001000,
	VFD_ICON_DOLBY	= 0x00002000,
	VFD_ICON_POWER	= 0x00004000,
	VFD_ICON_TIMESHIFT = 0x00008000,
	VFD_ICON_SIGNAL	= 0x00010000,
	VFD_ICON_TV	= 0x00020000,
	VFD_ICON_RADIO	= 0x00040000,
	VFD_ICON_HD	= 0x01000001,
	VFD_ICON_1080P	= 0x02000001,
	VFD_ICON_1080I	= 0x03000001,
	VFD_ICON_720P	= 0x04000001,
	VFD_ICON_480P	= 0x05000001,
	VFD_ICON_480I	= 0x06000001,
	VFD_ICON_USB	= 0x07000001,
	VFD_ICON_MP3	= 0x08000001,
	VFD_ICON_PLAY	= 0x09000001,
	VFD_ICON_COL1	= 0x09000002,
	VFD_ICON_PAUSE	= 0x0A000001,
	VFD_ICON_CAM1	= 0x0B000001,
	VFD_ICON_COL2	= 0x0B000002,
	VFD_ICON_CAM2	= 0x0C000001,

//#endif
	VFD_ICON_MAX
} vfd_icon;

// used???
typedef enum {
	VFD_FLAG_NONE		= 0x00,
	VFD_FLAG_SCROLL_ON	= 0x01,		/* switch scrolling on */
	VFD_FLAG_SCROLL_LTR	= 0x02,		/* scroll from left to rightinstead of default right to left direction (i.e. for arabic text) */
	VFD_FLAG_SCROLL_SIO	= 0x04		/* start/stop scrolling with empty screen (scroll in/out) */
} vfd_flag;

#ifdef __sh__
#define VFDDISPLAYCHARS		0xc0425a00
#define VFDWRITECGRAM		0x40425a01
#define VFDBRIGHTNESS		0xc0425a03
#define VFDICONGETSTATE		0xc0425a0b

/* added by zeroone, also used in nuvoton.h; set PowerLed Brightness on HDBOX*/
#define VFDPWRLED		0xc0425a04

//light on off
#define VFDDISPLAYWRITEONOFF	0xc0425a05
#define VFDDRIVERINIT		0xc0425a08
#define VFDICONDISPLAYONOFF	0xc0425a0a

/* ufs912 */
#define VFDGETVERSION		0xc0425af7
#define VFDLEDBRIGHTNESS	0xc0425af8
#define VFDGETWAKEUPMODE	0xc0425af9

#define VFDGETTIME		0xc0425afa
#define VFDSETTIME		0xc0425afb
#define VFDSTANDBY		0xc0425afc
#define VFDREBOOT		0xc0425afd
#define VFDSETLED		0xc0425afe

/* ufs912, 922, hdbox ->unset compat mode */
#define VFDSETMODE		0xc0425aff
#define VFDDISPLAYCLR		0xc0425b00

// used???
#else
#define IOC_VFD_SET_BRIGHT	_IOW(0xDE,  1, unsigned char)	/* set the display brighness in 16 steps between 0 to 15 */
#define IOC_VFD_CLEAR_ALL	_IOW(0xDE,  2, unsigned int)	/* clear the entire display (both text and icons) */
#define IOC_VFD_SET_TEXT	_IOW(0xDE,  3, char*)		/* set a text to be displayed on the display. If arg == NULL, the text is cleared */
#define IOC_VFD_SET_ICON	_IOW(0xDE,  4, vfd_icon)	/* switch the given icon on */
#define IOC_VFD_CLEAR_ICON	_IOW(0xDE,  5, vfd_icon)	/* switch the given icon off */
#define IOC_VFD_SET_OUTPUT	_IOW(0xDE,  6, unsigned char)	/* switch the given output on (supported by the controller, but not used in the hardware) */
#define IOC_VFD_CLEAR_OUTPUT	_IOW(0xDE,  7, unsigned char)	/* switch the given output off (supported by the controller, but not used in the hardware) */

#endif

class CVFD
{
	public:

		enum MODES
		{
			MODE_TVRADIO,
			MODE_SCART,
			MODE_SHUTDOWN,
			MODE_STANDBY,
			MODE_MENU_UTF8,
			MODE_AUDIO
#ifdef VFD_UPDATE
                ,       MODE_FILEBROWSER,
                        MODE_PROGRESSBAR,
                        MODE_PROGRESSBAR2,
                        MODE_INFOBOX
#endif // LCD_UPDATE

		};
		enum AUDIOMODES
		{
			AUDIO_MODE_PLAY,
			AUDIO_MODE_STOP,
			AUDIO_MODE_FF,
			AUDIO_MODE_PAUSE,
			AUDIO_MODE_REV
		};


	private:
		MODES				mode;

		std::string			servicename;
		char				volume;
		unsigned char			percentOver;
		bool				muted;
		bool				showclock;
		pthread_t			thrTime;
		int                             last_toggle_state_power;
		bool				clearClock;
		unsigned int                    timeout_cnt;
		int fd;
		int brightness;
		char text[256];

		void wake_up();
		void count_down();

		CVFD();

		static void* TimeThread(void*);
		void setlcdparameter(int dimm, int power);
	public:

		~CVFD();
		bool has_lcd;
		void setlcdparameter(void);

		static CVFD* getInstance();
		void init(const char * fontfile, const char * fontname);

		void setMode(const MODES m, const char * const title = "");

		void showServicename(const std::string & name); // UTF-8
		void showTime(bool force = false);
		/** blocks for duration seconds */
		void showRCLock(int duration = 2);
		void showVolume(const char vol, const bool perform_update = true);
		void showPercentOver(const unsigned char perc, const bool perform_update = true);
		void showMenuText(const int position, const char * text, const int highlight = -1, const bool utf_encoded = false);
		void showAudioTrack(const std::string & artist, const std::string & title, const std::string & album);
		void showAudioPlayMode(AUDIOMODES m=AUDIO_MODE_PLAY);
		void showAudioProgress(const char perc, bool isMuted);
		void setBrightness(int);
		int getBrightness();

		void setBrightnessStandby(int);
		int getBrightnessStandby();

		void setPower(int);
		int getPower();

		void togglePower(void);

		void setInverse(int);
		int getInverse();

		void setMuted(bool);

		void resume();
		void pause();
		void Lock();
		void Unlock();
		void Clear();
		void ShowIcon(vfd_icon icon, bool show);
		void ShowText(char * str);
		
#ifdef __sh__
                void openDevice();
		void closeDevice();
#endif
		
#ifdef LCD_UPDATE
        private:
                CFileList* m_fileList;
                int m_fileListPos;
                std::string m_fileListHeader;

                std::string m_infoBoxText;
                std::string m_infoBoxTitle;
                int m_infoBoxTimer;   // for later use
                bool m_infoBoxAutoNewline;

                bool m_progressShowEscape;
                std::string  m_progressHeaderGlobal;
                std::string  m_progressHeaderLocal;
                int m_progressGlobal;
                int m_progressLocal;
        public:
                MODES getMode(void){return mode;};

                void showFilelist(int flist_pos = -1,CFileList* flist = NULL,const char * const mainDir=NULL);
                void showInfoBox(const char * const title = NULL,const char * const text = NULL,int autoNewline = -1,int timer = -1);
                void showProgressBar(int global = -1,const char * const text = NULL,int show_escape = -1,int timer = -1);
                void showProgressBar2(int local = -1,const char * const text_local = NULL,int global = -1,const char * const text_global = NULL,int show_escape = -1);
#endif // LCD_UPDATE

};

#endif
