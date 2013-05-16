/*
  Neutrino-GUI  -   DBoxII-Project

  UPnP Browser (c) 2007 by Jochen Friedrich

  Homepage: http://dbox.cyberphoria.org/

  Kommentar:

  Diese GUI wurde von Grund auf neu programmiert und sollte nun vom
  Aufbau und auch den Ausbaumoeglichkeiten gut aussehen. Neutrino basiert
  auf der Client-Server Idee, diese GUI ist also von der direkten DBox-
  Steuerung getrennt. Diese wird dann von Daemons uebernommen.


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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sstream>
#include <stdexcept>
#include <gui/upnpbrowser.h>

#include <global.h>
#include <neutrino.h>
#include <xmltree.h>
#include <upnpclient.h>

#include <driver/encoding.h>
#include <driver/fontrenderer.h>
#include <driver/rcinput.h>
#include <driver/audioplay.h>
#include <driver/audiofile.h>
#include <driver/audiometadata.h>

#include <daemonc/remotecontrol.h>

#include <gui/eventlist.h>
#include <gui/color.h>
#include <gui/infoviewer.h>

#include <gui/widget/buttons.h>
#include <gui/widget/icons.h>
#include <gui/widget/menue.h>
#include <gui/widget/messagebox.h>
#include <gui/widget/hintbox.h>
#include <gui/widget/stringinput.h>
#include <gui/widget/stringinput_ext.h>

#include <system/settings.h>

#include <video_cs.h>
extern cVideo * videoDecoder;

#ifdef __sh__
#include <playback_cs.h>
#endif

#ifdef ConnectLineBox_Width
#undef ConnectLineBox_Width
#endif
#define ConnectLineBox_Width    15

const struct button_label RescanButton = {NEUTRINO_ICON_BUTTON_BLUE  , LOCALE_UPNPBROWSER_RESCAN};
const struct button_label StopButton   = {NEUTRINO_ICON_BUTTON_YELLOW, LOCALE_AUDIOPLAYER_STOP};
const struct button_label PUpButton    = {NEUTRINO_ICON_BUTTON_RED   , LOCALE_FILEBROWSER_NEXTPAGE};
const struct button_label PDownButton  = {NEUTRINO_ICON_BUTTON_GREEN , LOCALE_FILEBROWSER_PREVPAGE};

#ifdef __sh__
#include <audio_cs.h>
extern cAudio * audioDecoder;
#endif

//------------------------------------------------------------------------

CUpnpBrowserGui::CUpnpBrowserGui()
{
	m_socket = new CUPnPSocket();
	m_frameBuffer = CFrameBuffer::getInstance();
	m_playing_entry_is_shown = false;

}

//------------------------------------------------------------------------

CUpnpBrowserGui::~CUpnpBrowserGui()
{
	delete m_socket;
}

//------------------------------------------------------------------------

int CUpnpBrowserGui::exec(CMenuTarget* parent, const std::string & /*actionKey*/)
{

	CAudioPlayer::getInstance()->init();

	if(parent)
	{
		parent->hide();
	}

	g_Zapit->stopPlayBack();

	videoDecoder->ShowPicture(DATADIR "/neutrino/icons/mp3.jpg");

	// tell neutrino we're in audio mode
	CNeutrinoApp::getInstance()->handleMsg( NeutrinoMessages::CHANGEMODE , NeutrinoMessages::mode_audio );
	// remember last mode
#if 0
	CZapitClient::responseGetLastChannel firstchannel;
	g_Zapit->getLastChannel(firstchannel.channelNumber, firstchannel.mode);
	if ((firstchannel.mode == 'r') ?
	    (CNeutrinoApp::getInstance()->zapto_radio_on_init_done) :
	    (CNeutrinoApp::getInstance()->zapto_tv_on_init_done))
		m_LastMode=(CNeutrinoApp::getInstance()->getLastMode() | NeutrinoMessages::norezap);
	else
		m_LastMode=(CNeutrinoApp::getInstance()->getLastMode());
#endif
	m_LastMode=(CNeutrinoApp::getInstance()->getLastMode());

	m_width=(g_settings.screen_EndX - g_settings.screen_StartX) - ConnectLineBox_Width;
	m_height = (g_settings.screen_EndY - g_settings.screen_StartY);

	m_sheight = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight();
	m_buttonHeight = std::min(25, m_sheight);
	m_theight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight();
	m_mheight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight();
	m_fheight = g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->getHeight();
	m_title_height = m_mheight*2 + 20 + m_sheight + 4;
	m_info_height = m_mheight*2;
	m_listmaxshow = (m_height - m_info_height - m_title_height - m_theight - 2*m_buttonHeight) / (m_fheight);
	m_height = m_theight + m_info_height + m_title_height + 2*m_buttonHeight + m_listmaxshow * m_fheight; // recalc height

	m_x = (((g_settings.screen_EndX - g_settings.screen_StartX) - (m_width + ConnectLineBox_Width)) / 2)
		+ g_settings.screen_StartX + ConnectLineBox_Width;
	m_y = (((g_settings.screen_EndY- g_settings.screen_StartY) - m_height)/ 2) + g_settings.screen_StartY;

	// Stop sectionsd
	g_Sectionsd->setPauseScanning(true);

	m_indexdevice=0;
	m_selecteddevice=0;

	selectDevice();

	if(CAudioPlayer::getInstance()->getState() != CBaseDec::STOP)
		CAudioPlayer::getInstance()->stop();

	//g_Zapit->setStandby(false);

	// Start Sectionsd
	g_Sectionsd->setPauseScanning(false);
	videoDecoder->StopPicture();

	CNeutrinoApp::getInstance()->handleMsg( NeutrinoMessages::CHANGEMODE , m_LastMode );
	g_RCInput->postMsg( NeutrinoMessages::SHOW_INFOBAR, 0 );

	return menu_return::RETURN_EXIT_ALL;
}

//------------------------------------------------------------------------

void CUpnpBrowserGui::splitProtocol(std::string &protocol, std::string &prot, std::string &network, std::string &mime, std::string &additional)
{
	std::string::size_type pos;
	std::string::size_type startpos = 0;

	pos = protocol.find(":", startpos);
	if (pos != std::string::npos)
	{
		prot = protocol.substr(startpos, pos-startpos);
		startpos = pos + 1;

		pos = protocol.find(":", startpos);
		if (pos != std::string::npos)
		{
			network = protocol.substr(startpos, pos-startpos);
			startpos = pos + 1;

			pos = protocol.find(":", startpos);
			if (pos != std::string::npos)
			{
				mime = protocol.substr(startpos, pos-startpos);
				startpos = pos + 1;

				pos = protocol.find(":", startpos);
				if (pos != std::string::npos)
				{
					additional = protocol.substr(startpos, pos-startpos);
				}
			}
		}
	}
//printf("%s -> %s - %s - %s - %s\n", protocol.c_str(), prot.c_str(), network.c_str(), mime.c_str(), additional.c_str());
}

//------------------------------------------------------------------------

std::vector<UPnPEntry> *CUpnpBrowserGui::decodeResult(std::string result)
{
	XMLTreeParser *parser;
	XMLTreeNode   *root, *node, *snode;
	std::vector<UPnPEntry> *entries;

	parser = new XMLTreeParser("UTF-8");
	parser->Parse(result.c_str(), result.size(), 1);
	root=parser->RootNode();
	if (!root){
		delete parser;
		return NULL;
	}
	entries = new std::vector<UPnPEntry>;

	for (node=root->GetChild(); node; node=node->GetNext())
	{
		bool isdir;
		std::string title, artist = "", album = "", id, children;
		char *type, *p;

		if (!strcmp(node->GetType(), "container"))
		{
			std::vector<UPnPResource> resources;
			isdir=true;
			for (snode=node->GetChild(); snode; snode=snode->GetNext())
			{
				type=snode->GetType();
				p = strchr(type,':');
				if (p)
					type=p+1;
				if (!strcmp(type,"title"))
				{
					p=snode->GetData();
					if (!p)
						p=(char *) "";
					title=std::string(p);
				}
			}
			p = node->GetAttributeValue((char *) "id");
			if (!p)
				p=(char *) "";
			id=std::string(p);

			p = node->GetAttributeValue((char *) "childCount");
			if (!p)
				p=(char *) "";
			children=std::string(p);

			UPnPEntry entry={id, isdir, title, artist, album, children, resources, -1};
			entries->push_back(entry);
		}
		if (!strcmp(node->GetType(), "item"))
		{
			std::vector<UPnPResource> resources;
			int preferred = -1;
			std::string protocol, prot, network, mime, additional;
			isdir=false;
			for (snode=node->GetChild(); snode; snode=snode->GetNext())
			{
				std::string duration, url, size;
				unsigned int i;
				type=snode->GetType();
				p = strchr(type,':');
				if (p)
					type=p+1;

				if (!strcmp(type,"title"))
				{
					p=snode->GetData();
					if (!p)
						p=(char *) "";
					title=std::string(p);
				}
				else if (!strcmp(type,"artist"))
				{
					p=snode->GetData();
					if (!p)
						p=(char *) "";
					artist=std::string(p);
				}
				else if (!strcmp(type,"album"))
				{
					p=snode->GetData();
					if (!p)
						p=(char *) "";
					album=std::string(p);
				}
				else if (!strcmp(type,"res"))
				{
					p = snode->GetData();
					if (!p)
						p=(char *) "";
					url=std::string(p);
					p = snode->GetAttributeValue((char *) "size");
					if (!p)
						p=(char *) "0";
					size=std::string(p);
					p = snode->GetAttributeValue((char *) "duration");
					if (!p)
						p=(char *) "";
					duration=std::string(p);
					p = snode->GetAttributeValue((char *) "protocolInfo");
					if (!p)
						p=(char *) "";
					protocol=std::string(p);
					UPnPResource resource = {url, protocol, size, duration};
printf("%s, %s, %s, %s\n", url.c_str(), protocol.c_str(), size.c_str(), duration.c_str());
					resources.push_back(resource);
				}
				int pref=0;
				preferred=-1;
				for (i=0; i<resources.size(); i++)
				{
					protocol=resources[i].protocol;
					splitProtocol(protocol, prot, network, mime, additional);
					if (prot != "http-get")
						continue;
#if 0
					if (mime == "image/jpeg" && pref < 1)
					{
						preferred=i;
						pref=1;
					}
					if (mime == "image/gif" && pref < 2)
					{
						preferred=i;
						pref=2;
					}
#endif

printf("%s: mime %s\n",__func__, mime.c_str());
					if (mime == "audio/mpeg" && pref < 3)
					{
						preferred=i;
						pref=3;
					}
					if (mime == "audio/x-vorbis+ogg" && pref < 4)
					{
						preferred=i;
						pref=4;
					}
#ifdef __sh__
					/* avi */
					if (mime == "video/x-msvideo" && pref < 5)
					{
						preferred=i;
						pref=5;
					}
					/* vob, ts */
					if (mime == "video/mpeg" && pref < 6)
					{
						preferred=i;
						pref=6;
					}
					/* mp4 */
					if (mime == "video/mp4" && pref < 6)
					{
						preferred=i;
						pref=6;
					}
					if (mime == "video/x-matroska" && pref < 7)
					{
						preferred=i;
						pref=7;
					}
					if (mime == "video/x-ms-wmv" && pref < 8)
					{
						preferred=i;
						pref=8;
					}
#endif
				}
			}
			p = node->GetAttributeValue((char *) "id");
			if (!p)
				p=(char *) "";
			id=std::string(p);

			p = node->GetAttributeValue((char *) "childCount");
			if (!p)
				p=(char *) "";
			children=std::string(p);

			UPnPEntry entry={id, isdir, title, artist, album, children, resources, preferred};

			entries->push_back(entry);
		}
	}
	delete parser;
	return entries;
}

//------------------------------------------------------------------------

void CUpnpBrowserGui::selectDevice()
{
	bool loop = true;
	bool changed = true;
	neutrino_msg_t      msg;
	neutrino_msg_data_t data;

	CHintBox *scanBox = new CHintBox(LOCALE_MESSAGEBOX_INFO, g_Locale->getText(LOCALE_UPNPBROWSER_SCANNING)); // UTF-8
	scanBox->paint();
#if 0
	try {
		m_devices = m_socket->Discover("urn:schemas-upnp-org:service:ContentDirectory:1");
	}
	catch (std::runtime_error error)
	{
		delete scanBox;
		ShowMsgUTF(LOCALE_MESSAGEBOX_INFO, error.what(), CMessageBox::mbrBack, CMessageBox::mbBack, "info.raw");
		return;
	}
#endif
	m_devices = m_socket->Discover("urn:schemas-upnp-org:service:ContentDirectory:1");
	scanBox->hide();

	if (!m_devices.size())
	{
		ShowLocalizedMessage(LOCALE_MESSAGEBOX_INFO, LOCALE_UPNPBROWSER_NOSERVERS, CMessageBox::mbrBack, CMessageBox::mbBack, "info.raw");
		delete scanBox;
		return;
	}

	while (loop)
	{
		if (changed)
		{
			paintDevice();
			changed=false;
		}

		g_RCInput->getMsg(&msg, &data, 10); // 1 sec timeout to update play/stop state display
		neutrino_msg_t msg_repeatok = msg & ~CRCInput::RC_Repeat;

		if( msg == CRCInput::RC_timeout)
		{
			// nothing
		}

		else if( msg == CRCInput::RC_home)
		{
			loop=false;
		}

		else if (msg_repeatok == CRCInput::RC_up && m_selecteddevice > 0)
		{
			m_selecteddevice--;
			if (m_selecteddevice < m_indexdevice)
				m_indexdevice-=m_listmaxshow;
			changed=true;
		}

		else if (msg_repeatok == CRCInput::RC_down && m_selecteddevice + 1 < m_devices.size())
		{
			m_selecteddevice++;
			if (m_selecteddevice + 1 > m_indexdevice + m_listmaxshow)
				m_indexdevice+=m_listmaxshow;
			changed=true;
		}

		else if( msg == CRCInput::RC_right || msg == CRCInput::RC_ok)
		{
			m_folderplay = false;
			selectItem("0");
			changed=true;
		}

		else if( msg == CRCInput::RC_blue)
		{
			scanBox->paint();
#if 0
			try {
				m_devices = m_socket->Discover("urn:schemas-upnp-org:service:ContentDirectory:1");
			}
			catch (std::runtime_error error)
			{
				delete scanBox;
				ShowMsgUTF(LOCALE_MESSAGEBOX_INFO, error.what(), CMessageBox::mbrBack, CMessageBox::mbBack, "info.raw");
				return;
			}
#endif
			m_devices = m_socket->Discover("urn:schemas-upnp-org:service:ContentDirectory:1");
			scanBox->hide();
			if (!m_devices.size())
			{
				ShowLocalizedMessage(LOCALE_MESSAGEBOX_INFO, LOCALE_UPNPBROWSER_NOSERVERS, CMessageBox::mbrBack, CMessageBox::mbBack, "info.raw");
				delete scanBox;
				return;
			}
			changed=true;
		}

		else if(msg == NeutrinoMessages::RECORD_START ||
			msg == NeutrinoMessages::ZAPTO ||
			msg == NeutrinoMessages::STANDBY_ON ||
			msg == NeutrinoMessages::SHUTDOWN ||
			msg == NeutrinoMessages::SLEEPTIMER)
		{
			loop=false;
			g_RCInput->postMsg(msg, data);
		}

		else if(msg == NeutrinoMessages::EVT_TIMER)
		{
			CNeutrinoApp::getInstance()->handleMsg( msg, data );
		}

		else
		{
			if( CNeutrinoApp::getInstance()->handleMsg( msg, data ) & messages_return::cancel_all )
				loop = false;
			changed=true;
		}
	}
	delete scanBox;
}

//------------------------------------------------------------------------

void CUpnpBrowserGui::playnext(void)
{
	while (true)
	{
		std::list<UPnPAttribute>attribs;
		std::list<UPnPAttribute>results;
		std::list<UPnPAttribute>::iterator i;
		std::stringstream sindex;
		std::vector<UPnPEntry> *entries = NULL;
		bool rfound = false;
		bool nfound = false;
		bool tfound = false;

		sindex << m_playid;
		attribs.push_back(UPnPAttribute("ObjectID", m_playfolder));
		attribs.push_back(UPnPAttribute("BrowseFlag", "BrowseDirectChildren"));
		attribs.push_back(UPnPAttribute("Filter", "*"));
		attribs.push_back(UPnPAttribute("StartingIndex", sindex.str()));
		attribs.push_back(UPnPAttribute("RequestedCount", "1"));
		attribs.push_back(UPnPAttribute("SortCriteria", ""));

#if 0
		try
		{
			results=m_devices[m_selecteddevice].SendSOAP("urn:schemas-upnp-org:service:ContentDirectory:1", "Browse", attribs);
		}
		catch (std::runtime_error error)
		{
			ShowMsgUTF(LOCALE_MESSAGEBOX_INFO, error.what(), CMessageBox::mbrBack, CMessageBox::mbBack, "info.raw");
			m_folderplay = false;
			return;
		}
#endif
		results=m_devices[m_selecteddevice].SendSOAP("urn:schemas-upnp-org:service:ContentDirectory:1", "Browse", attribs);
		for (i=results.begin(); i!=results.end(); i++)
		{
			if (i->first=="NumberReturned")
			{
				if (atoi(i->second.c_str()) != 1)
				{
					m_folderplay = false;
					return;
				}
				nfound=true;
			}
			if (i->first=="TotalMatches")
			{
				tfound=true;
			}
			if (i->first=="Result")
			{
				entries=decodeResult(i->second);
				rfound=true;
			}
		}
		m_playid++;
		if ((entries != NULL) && (!(*entries)[0].isdir))
		{
			int preferred=(*entries)[0].preferred;
			if (preferred != -1)
			{
				std::string protocol, prot, network, mime, additional;
				protocol=(*entries)[0].resources[preferred].protocol;
				splitProtocol(protocol, prot, network, mime, additional);
				if (mime == "audio/mpeg")
				{
					m_playing_entry = (*entries)[0];
					m_playing_entry_is_shown = false;
					CAudiofile mp3((*entries)[0].resources[preferred].url, CFile::FILE_MP3);
					CAudioPlayer::getInstance()->play(&mp3, g_settings.audioplayer_highprio == 1);
					return;
				}
				else if (mime == "audio/x-vorbis+ogg")
				{
					m_playing_entry = (*entries)[0];
					m_playing_entry_is_shown = false;
					CAudiofile mp3((*entries)[0].resources[preferred].url, CFile::FILE_OGG);
					CAudioPlayer::getInstance()->play(&mp3, g_settings.audioplayer_highprio == 1);
					return;
				}
			}
		} else {
			neutrino_msg_t      msg;
			neutrino_msg_data_t data;
			g_RCInput->getMsg(&msg, &data, 10); // 1 sec timeout to update play/stop state display

			if( msg == CRCInput::RC_home)
			{
				m_folderplay = false;
				break;
			}
		}
	}
}

//------------------------------------------------------------------------

void CUpnpBrowserGui::hide()
{
   m_frameBuffer->ClearFrameBuffer();
}

//------------------------------------------------------------------------
#ifdef __sh__
        static cPlayback *playback = NULL;
        static bool   isPlaybackRunning = false;
        static int    m_LocalLastMode;
        static unsigned short g_apids[10];
        static unsigned short g_ac3flags[10];
        static unsigned short g_numpida = 0;
        static unsigned short g_vpid = 0;
        static unsigned short g_vtype = 0;
        static std::string    g_language[10];

        static unsigned int g_currentapid = 0, g_currentac3 = 0, apidchanged = 0;

int CUPNPAPIDSelectExec::exec(CMenuTarget * parent, const std::string & actionKey)
{
	apidchanged = 0;
	unsigned int sel = atoi(actionKey.c_str());
	if (g_currentapid != g_apids[sel - 1]) {
		g_currentapid = g_apids[sel - 1];
		g_currentac3 = g_ac3flags[sel - 1];
		apidchanged = 1;
		printf("[movieplayer] apid changed to %d\n", g_apids[sel - 1]);
	}
	return menu_return::RETURN_EXIT;
}
#endif

//------------------------------------------------------------------------

bool CUpnpBrowserGui::selectItem(std::string id)
{
	bool loop = true;
	bool endall = false;
	bool changed = true;
	bool rchanged = true;
	neutrino_msg_t      msg;
	neutrino_msg_data_t data;
	std::vector<UPnPEntry> *entries;
	unsigned int index, selected, dirnum;

	index=0;
	selected=0;
	dirnum=0;
	entries=NULL;

#ifdef __sh__
        printf("%s >\n", __func__);
#endif

	while (loop)
	{
#ifdef __sh__
            if (!isPlaybackRunning)
#endif		       
		updateTimes();
		if (rchanged)
		{
			if (entries)
				delete entries;
			entries=NULL;

			std::list<UPnPAttribute>attribs;
			std::list<UPnPAttribute>results;
			std::list<UPnPAttribute>::iterator i;
			std::stringstream sindex;
			std::stringstream scount;
			unsigned int returned = 0;

			bool rfound = false;
			bool nfound = false;
			bool tfound = false;

			sindex << index;
			scount << m_listmaxshow;

			attribs.push_back(UPnPAttribute("ObjectID", id));
			attribs.push_back(UPnPAttribute("BrowseFlag", "BrowseDirectChildren"));
			attribs.push_back(UPnPAttribute("Filter", "*"));
			attribs.push_back(UPnPAttribute("StartingIndex", sindex.str()));
			attribs.push_back(UPnPAttribute("RequestedCount", scount.str()));
			attribs.push_back(UPnPAttribute("SortCriteria", ""));
#if 0
			try
			{
				results=m_devices[m_selecteddevice].SendSOAP("urn:schemas-upnp-org:service:ContentDirectory:1", "Browse", attribs);
			}
			catch (std::runtime_error error)
			{
				ShowMsgUTF(LOCALE_MESSAGEBOX_INFO, error.what(), CMessageBox::mbrBack, CMessageBox::mbBack, "info.raw");
				if (entries)
					delete entries;
				return endall;
			}
#endif
			results=m_devices[m_selecteddevice].SendSOAP("urn:schemas-upnp-org:service:ContentDirectory:1", "Browse", attribs);
			for (i=results.begin(); i!=results.end(); i++)
			{
				if (i->first=="NumberReturned")
				{
					returned=atoi(i->second.c_str());
					nfound=true;
				}
				if (i->first=="TotalMatches")
				{
					dirnum=atoi(i->second.c_str());
					tfound=true;
				}
				if (i->first=="Result")
				{
					entries=decodeResult(i->second);
					rfound=true;
				}
			}
			if (!entries)
				return endall;
			if (!nfound || !tfound || !rfound)
			{
				delete entries;
				return endall;
			}
			if (returned != entries->size())
			{
				delete entries;
				return endall;
			}

			if (returned == 0)
			{
				delete entries;
				return endall;
			}
			rchanged=false;
			changed=true;
		}

#ifdef __sh__
            if (isPlaybackRunning)
	        changed = false;
#endif		       
		if (changed)
		{
			paintItem(entries, selected - index, dirnum - index, index);
			changed=false;
		}

		g_RCInput->getMsg(&msg, &data, 10); // 1 sec timeout to update play/stop state display
		neutrino_msg_t msg_repeatok = msg & ~CRCInput::RC_Repeat;

		if( msg == CRCInput::RC_timeout)
		{
			// nothing
		}
		else if(msg == CRCInput::RC_home)
		{
			loop=false;
			endall=true;
		}
		else if(msg == CRCInput::RC_left)
		{
			loop=false;
		}

		else if (msg_repeatok == CRCInput::RC_up && selected > 0)
		{
#ifdef __sh__
		    if (isPlaybackRunning)
		       continue;
#endif		       
			selected--;
			if (selected < index)
			{
				index-=m_listmaxshow;
				rchanged=true;
			}
			changed=true;
		}

#ifndef __sh__
		else if(msg == CRCInput::RC_green && selected > 0)
#else
		else if(msg == CRCInput::RC_green)
#endif
		{

#ifdef __sh__
		    if (isPlaybackRunning)
		    {
printf("upnp audio\n");
			CMenuWidget APIDSelector(LOCALE_APIDSELECTOR_HEAD, "audio.raw", 300);
			
			playback->FindAllPids(g_apids, g_ac3flags, &g_numpida, g_language);
			
			if (g_numpida > 0) {

				APIDSelector.addItem(GenericMenuSeparator);
				CUPNPAPIDSelectExec *APIDChanger = new CUPNPAPIDSelectExec;
				bool enabled;
				bool defpid;
				for (unsigned int count = 0; count < g_numpida; count++) {
					bool name_ok;
					char apidnumber[10];
					sprintf(apidnumber, "%d %X", count + 1, g_apids[count]);
					enabled = true;
					defpid = g_currentapid ? (g_currentapid == g_apids[count]) : (count == 0);
					std::string apidtitle = "Stream ";

#ifdef keine_ahnung
					if(!is_file_player){
						name_ok = get_movie_info_apid_name(g_apids[count], p_movie_info, &apidtitle);
					}
					else 
#endif
					if (!g_language[count].empty()){
						apidtitle = g_language[count];
						name_ok = true;
					}
					if (!name_ok)
						apidtitle = "Stream ";

					switch(g_ac3flags[count])
					{
						case 1: /*AC3,EAC3*/
							if (apidtitle.find("AC3") < 0)
								apidtitle.append(" (AC3)");
							break;
						case 2: /*teletext*/
							apidtitle.append(" (Teletext)");
							enabled = false;
							break;
						case 3: /*MP2*/
							apidtitle.append(" (MP2)");
							break;
						case 4: /*MP3*/
							apidtitle.append(" (MP3)");
							break;
						case 5: /*AAC*/
							apidtitle.append(" (AAC)");
							break;
						case 6: /*DTS*/
							apidtitle.append(" (DTS)");
							break;
						case 7: /*MLP*/
							apidtitle.append(" (MLP)");
							break;
						default:
							break;
					}
					if (!name_ok)
						apidtitle.append(apidnumber);

					APIDSelector.addItem(new CMenuForwarderNonLocalized(apidtitle.c_str(), enabled, NULL, APIDChanger, apidnumber, CRCInput::convertDigitToKey(count + 1)), defpid);
				}

				apidchanged = 0;
				APIDSelector.exec(NULL, "");
				if (apidchanged) {
					if (g_currentapid == 0) {
						g_currentapid = g_apids[0];
						g_currentac3 = g_ac3flags[0];
					}
					playback->SetAPid(g_currentapid, g_currentac3);
					apidchanged = 0;
				}
				delete APIDChanger;
				CVFD::getInstance()->setMode(CVFD::MODE_MENU_UTF8);
			} else {
				DisplayErrorMessage(g_Locale->getText(LOCALE_AUDIOSELECTMENUE_NO_TRACKS)); // UTF-8
			}
		       continue;
		    } /* playbackrunning */
#endif		       

#ifdef __sh__
                    if (selected <= 0)
		       continue;
#endif

 			if (index > 0)
			{
				index-=m_listmaxshow;
				selected-=m_listmaxshow;
				rchanged=true;
			}
			else
				selected=0;
			changed=true;
		}

		else if (msg_repeatok == CRCInput::RC_down && selected + 1 < dirnum)
		{
#ifdef __sh__
		    if (isPlaybackRunning)
		       continue;
#endif		       
			selected++;
			if (selected + 1 > index + m_listmaxshow)
			{
				index+=m_listmaxshow;
				rchanged=true;
			}
			changed=true;
		}

		else if(msg == CRCInput::RC_red && selected + 1 < dirnum)
		{
#ifdef __sh__
		    if (isPlaybackRunning)
		       continue;
#endif		       
			if (index < ((dirnum - 1) / m_listmaxshow) * m_listmaxshow)
			{
				index+=m_listmaxshow;
				selected+=m_listmaxshow;
				if (selected + 1 >= dirnum)
					selected=dirnum - 1;
				rchanged=true;
			}
			else
				selected=dirnum - 1;
			changed=true;
		}

		else if(msg == CRCInput::RC_right)
		{
#ifdef __sh__
		    if (isPlaybackRunning)
		       continue;
#endif		       
			if ((*entries)[selected - index].isdir)
			{
				endall=selectItem((*entries)[selected - index].id);
				if (endall)
					loop=false;
			}
			changed=true;
		}
		else if(msg == CRCInput::RC_ok)
		{

#ifdef __sh__
		        if (isPlaybackRunning)
		            continue;

			if ((*entries)[selected - index].isdir)
			{
				endall=selectItem((*entries)[selected - index].id);
				if (endall)
					loop=false;
			        changed=true;
			} else
			
#endif		       

			if (!(*entries)[selected - index].isdir)
			{
#ifndef __sh__
				m_folderplay = false;
#endif
				int preferred=(*entries)[selected - index].preferred;
				if (preferred != -1)
				{
					std::string protocol, prot, network, mime, additional;
					protocol=(*entries)[selected - index].resources[preferred].protocol;
					splitProtocol(protocol, prot, network, mime, additional);

printf("mime = %s, url %s\n", mime.c_str(), (*entries)[selected - index].resources[preferred].url.c_str());

#ifndef __sh__
					if (mime == "audio/mpeg")
					{
						CAudiofile mp3((*entries)[selected - index].resources[preferred].url, CFile::FILE_MP3);
						CAudioPlayer::getInstance()->play(&mp3, g_settings.audioplayer_highprio == 1);
					}
					else if (mime == "audio/x-vorbis+ogg")
					{
						CAudiofile mp3((*entries)[selected - index].resources[preferred].url, CFile::FILE_OGG);
						CAudioPlayer::getInstance()->play(&mp3, g_settings.audioplayer_highprio == 1);
					}
#else
					if ((mime == "audio/mpeg") || 
					    (mime == "video/mpeg") || 
					    (mime == "video/mp4") || 
					    (mime == "video/x-msvideo") ||
					    (mime == "video/x-matroska") ||
					    (mime == "video/x-ms-wmv") ||
					    (mime == "audio/x-vorbis+ogg") /* dont know if ogg vorbis work through libeplayer2/player2 I  have no testfiles */
				           ) 
					{
                                           char* filename = (char*) malloc(strlen((char*) (*entries)[selected - index].resources[preferred].url.c_str()) + 8);
					   
printf("play %d\n", selected - index);
					   sprintf(filename, "upnp://%s", (char*) (*entries)[selected - index].resources[preferred].url.c_str());
					   
	                                   if (playback == NULL)
					   {
					      playback = new cPlayback(3);

	                                      CNeutrinoApp::getInstance()->handleMsg( NeutrinoMessages::CHANGEMODE , NeutrinoMessages::mode_ts );
	                                      m_LocalLastMode = (CNeutrinoApp::getInstance()->getLastMode() | NeutrinoMessages::norezap);

	                                      g_Zapit->setStandby(true);
	                                      g_Sectionsd->setPauseScanning(true);
	                                      
					      audioDecoder->Close();
	                                      videoDecoder->Close();
					   }
					   
			                   playback->Open(PLAYMODE_FILE);

			                   if (!playback->Start(filename, g_vpid, g_vtype, g_currentapid, g_currentac3)) 
					   {
				              printf("%s Starting Playback failed!\n", __FUNCTION__);
				              playback->Close();

					      isPlaybackRunning = false;

	                                      delete playback;
	                                      playback = NULL;
	   
	                                      audioDecoder->Open();
	                                      videoDecoder->Open();

	                                      g_Zapit->setStandby(false);
	                                      g_Zapit->stopPlayBack();
			                   } else 
					   {
					      hide();
				              CVFD::getInstance()->ShowIcon(VFD_ICON_PLAY, true);
					      playback->SetSpeed(1);
					      isPlaybackRunning = true;
				           }
					   free(filename);
					}
#endif
					m_playing_entry = (*entries)[selected - index];
#if 0
// #ifdef ENABLE_PICTUREVIEWER
					else if ((mime == "image/gif") || (mime == "image/jpeg"))
					{
						CPictureViewer *viewer = new CPictureViewer();
						bool loop=true;
						viewer->SetScaling((CPictureViewer::ScalingMode)g_settings.picviewer_scaling);
						viewer->SetVisible(g_settings.screen_StartX, g_settings.screen_EndX, g_settings.screen_StartY, g_settings.screen_EndY);

						if(g_settings.video_Format==1)
							viewer->SetAspectRatio(16.0/9);
						else if(g_settings.video_Format==0)
						{
							CControldClient cdc;
							cdc.setVideoFormat(CControldClient::VIDEOFORMAT_4_3);
							viewer->SetAspectRatio(4.0/3);
						}
						else
							viewer->SetAspectRatio(4.0/3);

						m_frameBuffer->setMode(720, 576, 16);
						m_frameBuffer->setTransparency(0);
						viewer->ShowImage((*entries)[selected - index].resources[preferred].url, true);
						while (loop)
						{
							g_RCInput->getMsg(&msg, &data, 10); // 1 sec timeout to update play/stop state display

							if( msg == CRCInput::RC_home)
								loop=false;
						}
						m_frameBuffer->setMode(720, 576, 8 * sizeof(fb_pixel_t));
						m_frameBuffer->setBlendLevel(g_settings.gtx_alpha1, g_settings.gtx_alpha2);
						m_frameBuffer->ClearFrameBuffer();
						delete viewer;
					}
// #endif
#endif
				}

			}
#ifndef __sh__
			 else {
				m_folderplay = true;
				m_playfolder = (*entries)[selected - index].id;
				m_playid = 0;
				playnext();
			}
#endif
			changed=true;
		}
#ifndef __sh__
		else if( msg == CRCInput::RC_yellow)
		{
			if(CAudioPlayer::getInstance()->getState() != CBaseDec::STOP)
				CAudioPlayer::getInstance()->stop();
			m_folderplay = false;
		}
/* on __sh__ we use yellow as audio selector */
#else
		else if( msg == CRCInput::RC_yellow)
		{
#ifdef fixme
/* fixme: this should replace m_folderplay later */

		    if (m_repeatMode == eRepeatNone)
		    
		    else
		    if (m_repeatMode == eRepeatOne)
		    
		    else
		    if (m_repeatMode == eRepeatDir)
#endif
		}
#endif
		else if(msg == NeutrinoMessages::RECORD_START ||
			msg == NeutrinoMessages::ZAPTO ||
			msg == NeutrinoMessages::STANDBY_ON ||
			msg == NeutrinoMessages::SHUTDOWN ||
			msg == NeutrinoMessages::SLEEPTIMER)
		{
			loop = false;
			g_RCInput->postMsg(msg, data);
		}

		else if(msg == NeutrinoMessages::EVT_TIMER)
		{
			CNeutrinoApp::getInstance()->handleMsg( msg, data );
		}
#ifdef __sh__
		else if (isPlaybackRunning)
		{
		   int speed;
		   
		   speed = playback->GetCurrPlaybackSpeed();
		   
		   if (msg == CRCInput::RC_rewind) 
		   {	// rewind
printf("upnp rewind\n");
			   if (speed >= 0)
				   speed = -1;
			   else
				   speed --;

			   playback->SetSpeed(speed);
		   } else if (msg == CRCInput::RC_forward) 
		   {	// fast-forward
printf("upnp forward\n");
			   if (speed <= 0)
				   speed = 2;
			   else
				   speed ++;

			   playback->SetSpeed(speed);
		   } else
		   if (msg == CRCInput::RC_1) {	// Jump Backwards 1 minute
			   playback->SetPosition(-60 * 1000);
		   } else if (msg == CRCInput::RC_3) {	// Jump Forward 1 minute
			   playback->SetPosition(60 * 1000);
		   } else if (msg == CRCInput::RC_4) {	// Jump Backwards 5 minutes
			   playback->SetPosition(-5 * 60 * 1000);
		   } else if (msg == CRCInput::RC_6) {	// Jump Forward 5 minutes
			   playback->SetPosition(5 * 60 * 1000);
		   } else if (msg == CRCInput::RC_7) {	// Jump Backwards 10 minutes
			   playback->SetPosition(-10 * 60 * 1000);
		   } else if (msg == CRCInput::RC_9) {	// Jump Forward 10 minutes
			   playback->SetPosition(10 * 60 * 1000);
		   } else if (msg == CRCInput::RC_2) {	// goto start
			   playback->SetPosition(0, true);
		   } else if (msg == CRCInput::RC_page_up) {
			   playback->SetPosition(10 * 1000);
		   } else if (msg == CRCInput::RC_page_down) {
			   playback->SetPosition(-10 * 1000);
	           }
                   else if (msg == CRCInput::RC_stop) 
		   {
	              playback->Close();
           
	              delete playback;
	              playback = NULL;
	   
	              audioDecoder->Open();
	              videoDecoder->Open();

	              g_Zapit->setStandby(false);
	              g_Zapit->stopPlayBack();
           
	              isPlaybackRunning = false;
		      changed = true;  
		   }
                   else if (msg == CRCInput::RC_pause) 
		   {
			if (speed == 0) 
			{
			    speed = 1;
			    playback->SetSpeed(speed);
			} else 
			{
			    speed = 0;
			    playback->SetSpeed(speed);
			}
		   }
                   else if (msg == CRCInput::RC_play) 
		   {
			speed = 1;
			playback->SetSpeed(speed);
		   }
                   else if (msg == CRCInput::RC_help) 
		   {
			//fixme showaudioselectdialog = true;                
		   }
		}   
#endif
		else
		{
			if( CNeutrinoApp::getInstance()->handleMsg( msg, data ) & messages_return::cancel_all )
				loop = false;
			changed=true;
		}


#ifndef __sh__
		if (m_folderplay && (CAudioPlayer::getInstance()->getState() == CBaseDec::STOP))
			playnext();
#else
		if (isPlaybackRunning)
		{
	             int position = 0, duration = 0;
                     int file_prozent = 0;
		     int speed;

		     if(playback->GetPosition(position, duration)) {
			     if(duration > 100)
				     file_prozent = (unsigned char) (position / (duration / 100));
			     speed = playback->GetCurrPlaybackSpeed();
			     printf("CUpnpBrowserGui::PlayFile: speed %d position %d duration %d (%d, %d%%)\n", speed, position, duration, duration-position, file_prozent);
		     }

		     /* has playback ended ? */
		     if (playback->IsPlaying() == false)
		     {
printf("%s: playack ended\n", __func__);
		         if (m_folderplay)
			 {
printf("m_folderplayer = true\n");
	                    delete playback;
	                    playback = NULL;

	                    audioDecoder->Open();
	                    videoDecoder->Open();

	                    g_Zapit->setStandby(false);
	                    g_Zapit->stopPlayBack();

			    isPlaybackRunning = false;

			    selected++;
			    if (selected + 1 > index + m_listmaxshow)
			    {
				    index+=m_listmaxshow;
			    }
                            
			    msg = CRCInput::RC_ok;
			 } else
			 {
printf("m_folderplayer = false\n");
			    delete playback;
			    playback = NULL;

	                    audioDecoder->Open();
	                    videoDecoder->Open();

	                    g_Zapit->setStandby(false);
	                    g_Zapit->stopPlayBack();

			    isPlaybackRunning = false;
		            changed = true;
			 }
		     }

		}
#endif
	} /* while loop */

#ifdef __sh__
printf("nach while loop\n");
        if (isPlaybackRunning)
	{
	   playback->Close();
           
	   delete playback;
	   playback = NULL;
	   
	   audioDecoder->Open();
	   videoDecoder->Open();

	   g_Zapit->setStandby(false);
	   g_Zapit->stopPlayBack();
	   g_Sectionsd->setPauseScanning(false);
	   CNeutrinoApp::getInstance()->handleMsg(NeutrinoMessages::CHANGEMODE, m_LocalLastMode);
           
	   isPlaybackRunning = false;
        }
	
        printf("%s <\n", __func__);
#endif
	if (entries)
		delete entries;
	return endall;
}

//------------------------------------------------------------------------

void CUpnpBrowserGui::paintDevicePos(unsigned int pos)
{
	int ypos = m_y + m_title_height + m_theight + pos*m_fheight;
	uint8_t    color;
	fb_pixel_t bgcolor;

	if (pos == m_selecteddevice)
	{
		color   = COL_MENUCONTENT + 2;
		bgcolor = COL_MENUCONTENT_PLUS_2;
	}
	else
	{
		color   = COL_MENUCONTENT;
		bgcolor = COL_MENUCONTENT_PLUS_0;
	}
	m_frameBuffer->paintBoxRel(m_x, ypos, m_width - 15, m_fheight, bgcolor);

	if (pos + m_indexdevice >= m_devices.size())
		return;

	char sNr[20];
	sprintf(sNr, "%2d", pos + 1);
	std::string num = sNr;

	std::string name = m_devices[pos + m_indexdevice].friendlyname;

	int w = g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->getRenderWidth(name) + 5;
	g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->RenderString(m_x + 10, ypos + m_fheight, m_width - 30 - w,
		num, color, m_fheight, true); // UTF-8
	g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->RenderString(m_x + m_width - 15 - w, ypos + m_fheight,
		w, name, color, m_fheight, true); // UTF-8
}

//------------------------------------------------------------------------

void CUpnpBrowserGui::paintItemPos(std::vector<UPnPEntry> *entry, unsigned int pos, unsigned int selected)
{
	int ypos = m_y + m_title_height + m_theight + pos*m_fheight;
	uint8_t    color;
	fb_pixel_t bgcolor;

	if (pos == selected)
	{
		color   = COL_MENUCONTENT + 2;
		bgcolor = COL_MENUCONTENT_PLUS_2;
		paintDetails(entry, pos);
		if ((*entry)[pos].isdir)
			paintItem2DetailsLine (-1, pos); // clear it
		else
			paintItem2DetailsLine (pos, pos);
	}
	else
	{
		color   = COL_MENUCONTENT;
		bgcolor = COL_MENUCONTENT_PLUS_0;
	}
	m_frameBuffer->paintBoxRel(m_x, ypos, m_width - 15, m_fheight, bgcolor);

	if (pos >= entry->size())
		return;

	int preferred=(*entry)[pos].preferred;
	std::string info;
	std::string fileicon;
	if ((*entry)[pos].isdir)
	{
		info = "<DIR>";
		fileicon = "folder.raw";
	}
	else
	{
		if (preferred != -1)
		{
			info = (*entry)[pos].resources[preferred].duration;
			fileicon = "mp3.raw";
		}
		else
		{
			info = "(none)";
			fileicon = "file.raw";
		}
	}

	std::string name = (*entry)[pos].title;
	char tmp_time[] = "00:00:00.0";
	int w = g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->getRenderWidth(tmp_time);

	m_frameBuffer->paintIcon(fileicon, m_x + 5 , ypos + (m_fheight - 16) / 2);
	g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->RenderString(m_x + m_width - 15 - w, ypos + m_fheight,
		w, info, color, m_fheight);
	g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->RenderString(m_x + 30, ypos + m_fheight, m_width - 50 - w,
		name, color, m_fheight, true); // UTF-8

}

//------------------------------------------------------------------------

void CUpnpBrowserGui::paintDevice()
{
	std::string tmp;
	int w, xstart, ypos, top;
	int c_rad_mid = RADIUS_MID;

	// LCD
	CVFD::getInstance()->setMode(CVFD::MODE_MENU_UTF8, "Select UPnP Device");
	CVFD::getInstance()->showMenuText(0, m_devices[m_selecteddevice].friendlyname.c_str(), -1, true);

	// Info
	m_frameBuffer->paintBoxRel(m_x, m_y, m_width, m_title_height - 10, COL_MENUCONTENT_PLUS_6, c_rad_mid);
	m_frameBuffer->paintBoxRel(m_x + 2, m_y + 2, m_width - 4, m_title_height - 14, COL_MENUCONTENTSELECTED_PLUS_0, c_rad_mid);

	// first line
	tmp = m_devices[m_selecteddevice].manufacturer + " " +
		m_devices[m_selecteddevice].manufacturerurl;
	w = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(tmp, true); // UTF-8
	xstart = (m_width - w) / 2;
	if(xstart < 10)
		xstart = 10;
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(m_x + xstart, m_y + 4 + 1*m_mheight, m_width - 20,
		tmp, COL_MENUCONTENTSELECTED, 0, true); // UTF-8

	// second line
	tmp = m_devices[m_selecteddevice].modelname + " " +
		m_devices[m_selecteddevice].modelnumber + " " +
		m_devices[m_selecteddevice].modeldescription;
	w = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(tmp, true); // UTF-8
	xstart = (m_width - w) / 2;
	if(xstart < 10)
		xstart = 10;
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(m_x + xstart, m_y + 4 + 2*m_mheight, m_width - 20,
		tmp, COL_MENUCONTENTSELECTED, 0, true); // UTF-8
	// third line
	tmp = m_devices[m_selecteddevice].modelurl;
	w = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(tmp, true); // UTF-8
	xstart = (m_width - w) / 2;
	if(xstart < 10)
		xstart = 10;
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(m_x + xstart, m_y + 4 + 3*m_mheight, m_width - 20,
		tmp, COL_MENUCONTENTSELECTED, 0, true); // UTF-8

	// Head
	tmp = g_Locale->getText(LOCALE_UPNPBROWSER_HEAD);
	m_frameBuffer->paintBoxRel(m_x, m_y + m_title_height, m_width, m_theight, COL_MENUHEAD_PLUS_0, c_rad_mid, CORNER_TOP);
	m_frameBuffer->paintIcon(NEUTRINO_ICON_UPNP, m_x + 7, m_y + m_title_height + 6);
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(m_x + 35, m_y + m_theight + m_title_height + 0,
		m_width - 45, tmp, COL_MENUHEAD, 0, true); // UTF-8
	ypos = m_y + m_title_height;
	if(m_theight > 26)
		ypos = (m_theight - 26) / 2 + m_y + m_title_height;
	m_frameBuffer->paintIcon(NEUTRINO_ICON_BUTTON_DBOX, m_x + m_width - 30, ypos);
#if 0
	if( CNeutrinoApp::getInstance()->isMuted() )
	{
		xpos = m_x + m_width - 75;
		ypos = m_y + m_title_height;
		if(m_theight > 32)
			ypos = (m_theight - 32) / 2 + m_y + m_title_height;
		m_frameBuffer->paintIcon(NEUTRINO_ICON_BUTTON_MUTE, xpos, ypos);
	}
#endif

	// Items
	for (unsigned int count=0;count<m_listmaxshow;count++)
		paintDevicePos(count);

	ypos = m_y + m_title_height + m_theight;
	int sb = m_fheight * m_listmaxshow;
	m_frameBuffer->paintBoxRel(m_x + m_width - 15, ypos, 15, sb, COL_MENUCONTENT_PLUS_1);

	int sbc = ((m_devices.size() - 1) / m_listmaxshow) + 1;
	int sbs = ((m_selecteddevice) / m_listmaxshow);

	m_frameBuffer->paintBoxRel(m_x + m_width - 13, ypos + 2 + sbs*(sb-4)/sbc, 11, (sb-4)/sbc, COL_MENUCONTENT_PLUS_3);

	// Foot
	top = m_y + (m_height - m_info_height - 2 * m_buttonHeight);

	int ButtonWidth = (m_width - 20) / 4;
	m_frameBuffer->paintBoxRel(m_x, top, m_width, 1 * m_buttonHeight, COL_MENUHEAD_PLUS_0, c_rad_mid, CORNER_BOTTOM);
	m_frameBuffer->paintHLine(m_x, m_x + m_width, top, COL_INFOBAR_SHADOW_PLUS_0);
	::paintButtons(m_frameBuffer, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL], g_Locale,
		m_x + 10, top + 4, ButtonWidth, 1, &RescanButton);

	clearItem2DetailsLine(); // clear it
}

//------------------------------------------------------------------------

void CUpnpBrowserGui::paintItem(std::vector<UPnPEntry> *entry, unsigned int selected, unsigned int max, unsigned int offset)
{
	std::string tmp;
	std::stringstream ts;
	int w, xstart, ypos, top;
	int preferred=(*entry)[selected].preferred;

	// LCD
	CVFD::getInstance()->setMode(CVFD::MODE_MENU_UTF8, "Select UPnP Entry");
	CVFD::getInstance()->showMenuText(0, (*entry)[selected].title.c_str(), -1, true);

	// Info
	m_frameBuffer->paintBoxRel(m_x, m_y, m_width, m_title_height - 10, COL_MENUCONTENT_PLUS_6);
	m_frameBuffer->paintBoxRel(m_x + 2, m_y + 2, m_width - 4, m_title_height - 14, COL_MENUCONTENTSELECTED_PLUS_0);

	// first line
	ts << "Resources: " << (*entry)[selected].resources.size() << " Selected: " << preferred+1 << " ";
	tmp = ts.str();

	if (preferred != -1)
		tmp = tmp + "Duration: " + (*entry)[selected].resources[preferred].duration;
	else
		tmp = tmp + "No resource for Item";
	w = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(tmp, true); // UTF-8
	if (w > m_width - 20)
		w = m_width - 20;
	xstart = (m_width - w) / 2;
	if(xstart < 10)
		xstart = 10;
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(m_x + xstart, m_y + 4 + 1*m_mheight, m_width - 20,
		tmp, COL_MENUCONTENTSELECTED, 0, true); // UTF-8

	// second line
	if ((*entry)[selected].isdir)
		tmp = "Directory";
	else
	{
		tmp = "";
		if (preferred != -1)
		{
			std::string proto, network, mime, info;
			splitProtocol((*entry)[selected].resources[preferred].protocol, proto, network, mime, info);
			tmp = "Protocol: " + proto + ", MIME-Type: " + mime;
		}

	}
	w = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(tmp, true); // UTF-8
	if (w > m_width - 20)
		w = m_width - 20;
	xstart = (m_width - w) / 2;
	if(xstart < 10)
		xstart = 10;
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(m_x + xstart, m_y + 4 + 2*m_mheight, m_width - 20,
		tmp, COL_MENUCONTENTSELECTED, 0, true); // UTF-8

	//third line
	tmp = "";
	if (!(*entry)[selected].isdir)
	{
		if (preferred != -1)
		{
			tmp = "URL: " + (*entry)[selected].resources[preferred].url;
		}

	}
	w = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(tmp, true); // UTF-8
	if (w > m_width - 20)
		w = m_width - 20;
	xstart = (m_width - w) / 2;
	if(xstart < 10)
		xstart = 10;
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(m_x + xstart, m_y + 4 + 3*m_mheight, m_width - 20,
		tmp, COL_MENUCONTENTSELECTED, 0, true); // UTF-8


	// Head
	tmp = g_Locale->getText(LOCALE_UPNPBROWSER_HEAD);
	m_frameBuffer->paintBoxRel(m_x, m_y + m_title_height, m_width, m_theight, COL_MENUHEAD_PLUS_0);
	m_frameBuffer->paintIcon(NEUTRINO_ICON_UPNP, m_x + 7, m_y + m_title_height + 6);
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(m_x + 35, m_y + m_theight + m_title_height + 0,
		m_width - 45, tmp, COL_MENUHEAD, 0, true); // UTF-8
	ypos = m_y + m_title_height;
	if(m_theight > 26)
		ypos = (m_theight - 26) / 2 + m_y + m_title_height;
	m_frameBuffer->paintIcon(NEUTRINO_ICON_BUTTON_DBOX, m_x + m_width - 30, ypos);
#if 0
	if( CNeutrinoApp::getInstance()->isMuted() )
	{
		xpos = m_x + m_width - 75;
		ypos = m_y + m_title_height;
		if(m_theight > 32)
			ypos = (m_theight - 32) / 2 + m_y + m_title_height;
		m_frameBuffer->paintIcon(NEUTRINO_ICON_BUTTON_MUTE, xpos, ypos);
	}
#endif

	// Items
	for (unsigned int count=0;count<m_listmaxshow;count++)
		paintItemPos(entry, count, selected);

	ypos = m_y + m_title_height + m_theight;
	int sb = m_fheight * m_listmaxshow;
	m_frameBuffer->paintBoxRel(m_x + m_width - 15, ypos, 15, sb, COL_MENUCONTENT_PLUS_1);

	int sbc = ((max + offset - 1) / m_listmaxshow) + 1;
	int sbs = ((selected + offset) / m_listmaxshow);

	int sbh = 0;
	if ((sbc > 0) && (sbc > sb-4))
		sbh = 2;
	m_frameBuffer->paintBoxRel(m_x + m_width - 13, ypos + 2 + sbs*((sb-4)/sbc+sbh), 11, (sb-4)/sbc + sbh, COL_MENUCONTENT_PLUS_3);

	// Foot buttons
	top = m_y + (m_height - m_info_height - 2 * m_buttonHeight);
	int ButtonWidth = (m_width - 20) / 4;
	m_frameBuffer->paintBoxRel(m_x, top, m_width, 1 * m_buttonHeight+2, COL_MENUHEAD_PLUS_0);
	m_frameBuffer->paintHLine(m_x, m_x + m_width, top, COL_INFOBAR_SHADOW_PLUS_0);
	::paintButtons(m_frameBuffer, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL], g_Locale,
		m_x + 10, top + 4, ButtonWidth, 1, &StopButton);
	::paintButtons(m_frameBuffer, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL], g_Locale,
		m_x + ButtonWidth + 10, top + 4, ButtonWidth, 1, &PUpButton);
	::paintButtons(m_frameBuffer, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL], g_Locale,
		m_x + 2 * ButtonWidth + 10, top + 4, ButtonWidth, 1, &PDownButton);

	m_frameBuffer->paintIcon(NEUTRINO_ICON_BUTTON_OKAY, m_x + 3 * ButtonWidth + 10, top + 1);
	g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString(m_x + 3 * ButtonWidth + 40,
		top + 19 + 4, ButtonWidth - 40,
		g_Locale->getText(LOCALE_AUDIOPLAYER_PLAY), COL_INFOBAR, 0, true); // UTF-8
}


//------------------------------------------------------------------------

void CUpnpBrowserGui::paintDetails(std::vector<UPnPEntry> *entry, unsigned int index, bool use_playing)
{
	// Foot info
	int top = m_y + (m_height - m_info_height - 1 * m_buttonHeight) + 2;

	if ((!use_playing) && ((*entry)[index].isdir))
	{
		m_frameBuffer->paintBackgroundBoxRel(m_x, top + 2, m_width, 2 * m_buttonHeight);
	}
	else
	{
//		char cNoch[50]; // UTF-8
//		char cSeit[50]; // UTF-8

		if (use_playing) {
			if (!m_playing_entry_is_shown) {
				m_frameBuffer->paintBoxRel(m_x, top + 2, m_width-2, 2 * m_buttonHeight, COL_MENUCONTENTDARK_PLUS_0);
				m_playing_entry_is_shown = true;
				g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->RenderString(m_x + 4,
					top + 1 * m_buttonHeight + 4, m_x + m_width - 8, m_playing_entry.title + " - " + 
									 m_playing_entry.artist, COL_INFOBAR, 0, true); // UTF-8
				g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->RenderString(m_x + 4,
					top + 2 * m_buttonHeight + 4, m_x + m_width - 8, m_playing_entry.album, COL_INFOBAR, 0, true); // UTF-8
			}
		} else {
			if (entry == NULL) return;
			m_frameBuffer->paintBoxRel(m_x, top + 2, m_width-2, 2 * m_buttonHeight, COL_MENUCONTENTDARK_PLUS_0);
			m_playing_entry_is_shown = false;
			g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->RenderString(m_x + 4,
				top + 1 * m_buttonHeight + 4, m_x + m_width - 8, (*entry)[index].title + " - " + 
								 (*entry)[index].artist, COL_INFOBAR, 0, true); // UTF-8
			g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->RenderString(m_x + 4,
				top + 2 * m_buttonHeight + 4, m_x + m_width - 8, (*entry)[index].album, COL_INFOBAR, 0, true); // UTF-8
		}
////		printf("title = %s\n", (*entry)[selected].title.c_str());
//		printf("artist = %s\n", (*entry)[selected].artist.c_str());
//		printf("album = %s\n", (*entry)[selected].album.c_str());
//		printf("children = %s\n", (*entry)[selected].children.c_str());
//		printf("id = %s\n", (*entry)[selected].id.c_str());
#if 0
		struct		tm *pStartZeit = localtime(&chanlist[index]->currentEvent.startTime);
		unsigned 	seit = ( time(NULL) - chanlist[index]->currentEvent.startTime ) / 60;
		sprintf( cSeit, g_Locale->getText(LOCALE_CHANNELLIST_SINCE), pStartZeit->tm_hour, pStartZeit->tm_min); //, seit );
		int seit_len = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getRenderWidth(cSeit, true); // UTF-8

		int noch = ( chanlist[index]->currentEvent.startTime + chanlist[index]->currentEvent.duration - time(NULL)   ) / 60;
		if ( (noch< 0) || (noch>=10000) )
			noch= 0;
		sprintf( cNoch, "(%d / %d min)", seit, noch );
		int noch_len = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getRenderWidth(cNoch, true); // UTF-8

		std::string text1= chanlist[index]->currentEvent.description;
		std::string text2= chanlist[index]->currentEvent.text;

		int xstart = 10;
		if (g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getRenderWidth(text1) > (width - 30 - seit_len) )
		{
			// zu breit, Umbruch versuchen...
		    int pos;
		    do
		    {
				pos = text1.find_last_of("[ -.]+");
				if ( pos!=-1 )
					text1 = text1.substr( 0, pos );
			} while ( ( pos != -1 ) && (g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getRenderWidth(text1) > (width - 30 - seit_len) ) );

			std::string text3= chanlist[index]->currentEvent.description.substr(text1.length()+ 1);
			if (!(text2.empty()))
				text3 += " . ";

			xstart += g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getRenderWidth(text3);
			g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->RenderString(x+ 10, y+ height+ 5+ 2* fheight, width - 30- noch_len, text3, COL_MENUCONTENTDARK);
		}

		if (!(text2.empty()))
		{
			while ( text2.find_first_of("[ -.+*#?=!$%&/]+") == 0 )
				text2 = text2.substr( 1 );
			text2 = text2.substr( 0, text2.find('\n') );
			g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->RenderString(x+ xstart, y+ height+ 5+ 2* fheight, width- xstart- 20- noch_len, text2, COL_MENUCONTENTDARK);
		}

		g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->RenderString(x+ 10, y+ height+ 5+ fheight, width - 30 - seit_len, text1, COL_MENUCONTENTDARK);
		g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->RenderString (x+ width- 10- seit_len, y+ height+ 5+    fheight   , seit_len, cSeit, COL_MENUCONTENTDARK, 0, true); // UTF-8
		g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->RenderString(x+ width- 10- noch_len, y+ height+ 5+ 2* fheight- 2, noch_len, cNoch, COL_MENUCONTENTDARK, 0, true); // UTF-8
#endif

	}
}

//------------------------------------------------------------------------

//
// -- Decoreline to connect ChannelDisplayLine with ChannelDetail display
// -- 2002-03-17 rasc
//

void CUpnpBrowserGui::clearItem2DetailsLine ()
{
	paintItem2DetailsLine (-1, 0);
}

//------------------------------------------------------------------------

void CUpnpBrowserGui::paintItem2DetailsLine (int pos, unsigned int /*ch_index*/)
{
	int xpos  = m_x - ConnectLineBox_Width;
	int ypos1 = m_y + m_title_height+0 + m_theight + pos*m_fheight;
	int ypos2 = m_y + (m_height - m_info_height - 1 * m_buttonHeight) + 2;

	int ypos1a = ypos1 + (m_fheight/2)-2;
	int ypos2a = ypos2 + (m_info_height/2)-2;
	fb_pixel_t col1 = COL_MENUCONTENT_PLUS_6;
	fb_pixel_t col2 = COL_MENUCONTENT_PLUS_1;

	// Clear
	m_frameBuffer->paintBackgroundBoxRel(xpos, m_y + m_title_height, ConnectLineBox_Width, m_height+m_info_height-(m_y + m_title_height));
	if (pos < 0) {
		m_frameBuffer->paintBackgroundBoxRel(m_x, m_y + (m_height - m_info_height - 1 * m_buttonHeight) + 2, m_width, m_info_height);
	}
	// paint Line if detail info (and not valid list pos)
	if (pos >= 0)
	{
		// 1. col thick line
		m_frameBuffer->paintBoxRel(xpos+ConnectLineBox_Width-4,  ypos1,  4,  m_fheight,     col1);
		m_frameBuffer->paintBoxRel(xpos+ConnectLineBox_Width-4,  ypos2,  4,  m_info_height, col1);

		m_frameBuffer->paintBoxRel(xpos+ConnectLineBox_Width-15, ypos1a, 4,  ypos2a-ypos1a, col1);

		m_frameBuffer->paintBoxRel(xpos+ConnectLineBox_Width-15, ypos1a, 12, 4, col1);
		m_frameBuffer->paintBoxRel(xpos+ConnectLineBox_Width-15, ypos2a, 12, 4, col1);

		// 2. col small line
		m_frameBuffer->paintBoxRel(xpos+ConnectLineBox_Width-4,  ypos1,  1,  m_fheight,     col2);
		m_frameBuffer->paintBoxRel(xpos+ConnectLineBox_Width-4,  ypos2,  1,  m_info_height, col2);

		m_frameBuffer->paintBoxRel(xpos+ConnectLineBox_Width-15, ypos1a, 1,  ypos2a-ypos1a+4, col2);

		m_frameBuffer->paintBoxRel(xpos+ConnectLineBox_Width-15, ypos1a, 12, 1, col2);
		m_frameBuffer->paintBoxRel(xpos+ConnectLineBox_Width-12, ypos2a, 8,  1, col2);

		// -- small Frame around infobox
		m_frameBuffer->paintBoxRel(m_x		, ypos2, 			2,		m_info_height, 	col1);
		m_frameBuffer->paintBoxRel(m_x+m_width-2, ypos2, 			2,		m_info_height, 	col1);
		m_frameBuffer->paintBoxRel(m_x		, ypos2, 			m_width-2,	2, 		col1);
		m_frameBuffer->paintBoxRel(m_x		, ypos2+m_info_height-2, 	m_width-2,	2, 		col1);
	}
}

//------------------------------------------------------------------------

void CUpnpBrowserGui::updateTimes(const bool force)
{
	int top;
	if(CAudioPlayer::getInstance()->getState() != CBaseDec::STOP)
	{
		bool updatePlayed = force;

		if ((m_time_played != CAudioPlayer::getInstance()->getTimePlayed()))
		{
			m_time_played = CAudioPlayer::getInstance()->getTimePlayed();
			updatePlayed = true;
		}
		char play_time[8];
		snprintf(play_time, 7, "%ld:%02ld", m_time_played / 60, m_time_played % 60);
		char tmp_time[] = "000:00";
		int w = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getRenderWidth(tmp_time);

		if (updatePlayed)
		{
			paintDetails(NULL, 0, true);
			top = m_y + (m_height - m_info_height - 1 * m_buttonHeight) + m_buttonHeight + 4;
			m_frameBuffer->paintBoxRel(m_x + m_width - w - 15, top + 1, w + 4, m_buttonHeight,
				   COL_MENUCONTENTDARK_PLUS_0);
			g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString(m_x + m_width - w - 11, top + 1 + m_buttonHeight, w, play_time, COL_MENUHEAD);
		}
	}
}

