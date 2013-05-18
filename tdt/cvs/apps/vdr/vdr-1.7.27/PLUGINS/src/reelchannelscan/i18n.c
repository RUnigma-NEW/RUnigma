/***************************************************************************
 *   Copyright (C) 2005 by Reel Multimedia;  Author:  Markus Hahn          *
 *                                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 *
 ***************************************************************************
 *
 *  i18n.c: Internationalization
 *
 ***************************************************************************/

#include "i18n.h"

const tI18nPhrase Phrases[] = {
    {"Channel Scan",
     "Kanalsuche",
     "",
     "Scansione canali",
     "Kanaal scan",
     "",
     "",
     "",
     "Kanavahaku",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "¿ÞØáÚ ÚÐÝÐÛÞÒ",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Vyhledani kanalu",                        // Èesky (Czech)
     },
    {"Search Transponders for DVB Channels",    //EN
     "Durchsucht Transponder nach DVB Kanälen.",        // DE
     "",                        // Slovenski
     "Ricerca Transponders di Canali DVB",      // IT
     "Doorzoek Transponders naar DVB kanalen",  // Nederlands
     "",                        // Português
     "",                        // Français
     "",                        // Norsk
     "Kanavahaku DVB-transpondereille", // FI
     "",                        // Polski
     "",                        // Español
     "",                        // ÅëëçíéêÜ (Greek)
     "",                        // Svenska
     "",                        // Romaneste
     "",                        // Magyar
     "",                        // Català
     "",                        // ÀãááÚØÙ (Russian)
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Vyhledan Transpoderu pro DVB kanaly",                        // Èesky (Czech)
     },
    {"Search Mode",
     "Suchmodus",
     "",
     "Modalita' di Ricerca",
     "Zoek mode",
     "",
     "",
     "",
     "Hakutapa",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Prohledavaci mod",                        // Èesky (Czech)
     },
    {
     "Add new channels to",     //EN
     "Gefundene Kanäle",                        // DE
     "",                        // Slovenski
     "",                        // IT
     "",                        // Nederlands
     "",                        // Português
     "",                        // Français
     "",                        // Norsk
     "",                        // FI
     "",                        // Polski
     "",                        // Español
     "",                        // ÅëëçíéêÜ (Greek)
     "",                        // Svenska
     "",                        // Romaneste
     "",                        // Magyar
     "",                        // Català
     "",                        // ÀãááÚØÙ (Russian)
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "",                        // Èesky (Czech)
    },
    {
     "end of channellist",      // EN
     "am Ende hinzufügen",      // DE
     "",                        // Slovenski
     "",                        // IT
     "",                        // Nederlands
     "",                        // Português
     "",                        // Français
     "",                        // Norsk
     "",                        // FI
     "",                        // Polski
     "",                        // Español
     "",                        // ÅëëçíéêÜ (Greek)
     "",                        // Svenska
     "",                        // Romaneste
     "",                        // Magyar
     "",                        // Català
     "",                        // ÀãááÚØÙ (Russian)
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "",                        // Èesky (Czech)
    },
    {
     "new channellist",         // EN
     "in neue Liste einfügen",  // DE
     "",                        // Slovenski
     "",                        // IT
     "",                        // Nederlands
     "",                        // Português
     "",                        // Français
     "",                        // Norsk
     "",                        // FI
     "",                        // Polski
     "",                        // Español
     "",                        // ÅëëçíéêÜ (Greek)
     "",                        // Svenska
     "",                        // Romaneste
     "",                        // Magyar
     "",                        // Català
     "",                        // ÀãááÚØÙ (Russian)
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "",                        // Èesky (Czech)
    },
    {
     "bouquets",                //EN
     "in Bouquets einfügen",    // DE
     "",                        // Slovenski
     "",                        // IT
     "",                        // Nederlands
     "",                        // Português
     "",                        // Français
     "",                        // Norsk
     "",                        // FI
     "",                        // Polski
     "",                        // Español
     "",                        // ÅëëçíéêÜ (Greek)
     "",                        // Svenska
     "",                        // Romaneste
     "",                        // Magyar
     "",                        // Català
     "",                        // ÀãááÚØÙ (Russian)
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "",                        // Èesky (Czech)
    },
    {"scanning on transponder",
     "Suche auf Transponder",
     "",
     "Ricerca trasponder",
     "Scan op transponder",
     "",
     "",
     "",
     "haetaan transponderilta",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Prohledavani transponderu",                        // Èesky (Czech)
     },
    {"Scanning configured satellites",
     "Durchsuche eingerichtete Satelliten",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Hledani konfigurace satelitu",                        // Èesky (Czech)
     },
    {"DiSEqC",
     "DiSEqC",
     "DiSEqC",
     "DiSEqC",
     "DiSEqC",
     "",
     "",
     "",
     "DiSEqC-kytkin",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "DiSEqC",                        // Èesky (Czech)
     },
    {"FEC",
     "FEC",
     "",
     "FEC",
     "FEC",
     "",
     "",
     "",
     "FEC",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "FEC",                        // Èesky (Czech)
     },
    {"None",
     "Keine",
     "",
     "Nessun",
     "Geen",
     "",
     "",
     "",
     "ei",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Zadny",                        // Èesky (Czech)
     },
    {"Auto",
     "Auto",
     "",
     "Auto",
     "Auto",
     "",
     "",
     "",
     "auto",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Auto",                        // Èesky (Czech)
     },
    {"SearchMode$Auto",
     "Automatisch",
     "",
     "Ricerca automatica",
     "Automatisch",
     "",
     "",
     "",
     "automaattinen",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Automaticke prohledavani",                        // Èesky (Czech)
     },
    {"Manual",
     "Manuell",
     "",
     "Manuale",
     "Handmatig",
     "",
     "",
     "",
     "manuaalinen",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Rucne",                        // Èesky (Czech)
     },
    {"Detailed search",
     "Detaillierte Suche",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Detailne prohledavat",                        // Èesky (Czech)
     },
    {"Position",
     "Position",
     "",
     "Posizione",
     "Positie",
     "",
     "",
     "",
     "Sijainti",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Pozice",                        // Èesky (Czech)
     },
    {"Tuner Error",
     "Tuner Fehler",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Chyba tuneru",                        // Èesky (Czech)
     },
    {"Scanning on transponder",
     "Durchsuche Transponder",
     "",
     "Ricerca Transponders",
     "Scannen op transponder",
     "",
     "",
     "",
     "Haetaan transponderilta",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Prohledavani transponderu",                        // Èesky (Czech)
     },
    {"channels in current list",
     "Kanäle in aktueller Liste",
     "",
     "Canali presenti nella lista",
     "Kanalen in huidige lijst",
     "",
     "",
     "",
     "Tämänhetkiset kanavat",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Aktuelni seznam kanalu",                        // Èesky (Czech)
     },
    {"TV CHANNELS",
     "TV KANÄLE",
     "",
     "CANALI TV",
     "TV KANALEN",
     "",
     "",
     "",
     "TV-kanavat",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Televizni kanaly",                        // Èesky (Czech)
     },
    {"No new channels found",
     "Keine neuen Kanäle gefunden",
     "",
     "Non sono stati trovati nuovi canali",
     "Geen nieuwe kanalen gevonden",
     "",
     "",
     "",
     "Uusia kanavia ei löydetty",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Zadne nove kanaly nenalezeny",                        // Èesky (Czech)
     },
    {"Scanning aborted by User",
     "Suche abgebrochen",
     "",
     "Ricerca interrotta dall'Utente",
     "Scannen afgebroken door User",
     "",
     "",
     "",
     "Haku keskeytetty",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Prohledavani zruseno uzivatelem",                        // Èesky (Czech)
     },

    {"  TV CHANNELS                      RADIO",
     "  TV KANÄLE                          RADIO",
     "",
     "  CANALI TV                      RADIO",
     "  TV KANALEN                      RADIO",
     "",
     "",
     "",
     "  TV-kanavat                      Radio",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "TV KANALY                  RADIO",                        // Èesky (Czech)
     },
    {"Symbolrate",
     "Symbolrate",
     "",
     "Symbolrate",
     "Symbolrate",
     "",
     "",
     "",
     "Symbolinopeus",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "ÁØÜÒ.áÚÞàÞáâì",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Symbolrate",                        // Èesky (Czech)
     },
    {"Frequency",
     "Frequenz",
     "",
     "Frequenza",
     "Frequentie",
     "",
     "",
     "",
     "Taajuus",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "ÇÐáâÞâÐ",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Frekvence",                        // Èesky (Czech)
     },
    {"Scanning %s\tPlease Wait",
     "Durchsuche %s\tBitte warten",
     "",
     "",
     "",
     "",
     "",
     "",
     "Haku käynnissä %s.\tOdota hetkinen...",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Prohledavani %s\tProsim cekejte",                        // Èesky (Czech)
     },
    {"Scanning %s (%iMHz) \tPlease Wait",
     "Durchsuche %s (%iMHz) \tBitte warten",
     "",
     "",
     "",
     "",
     "",
     "",
     "Haku käynnissä %s.\tOdota hetkinen...",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Prohledavani %s (%iMHz) \tProsim cekejte",                        // Èesky (Czech)
     },
    {"Scanning %s (%.3fMHz) \tPlease Wait",
     "Durchsuche %s (%.3fMHz) \tBitte warten",
     "",
     "",
     "",
     "",
     "",
     "",
     "Haku käynnissä %s.\tOdota hetkinen...",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Prohledavani %s (%.3fMHz) \tProsim cekejte",                        // Èesky (Czech)
     },
    {"Button$Change",
     "Auswählen",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Zmenit",                        // Èesky (Czech)
     },
    {"Button$Start",
     "Start",
     "",
     "Start",
     "Start",
     "",
     "",
     "",
     "Aloita",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Start",                        // Èesky (Czech)
     },
    {"Button$Backup",
     "Speichern",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Zpet",                        // Èesky (Czech)
     },
    {"Backup",
     "Speichern",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Zpet",                        // Èesky (Czech)
     },
    {"Button$Delete",
     "Löschen",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Smazat",                        // Èesky (Czech)
     },
    {"Button$New",
     "Neu",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Novy",                        // Èesky (Czech)
     },
    {"Radio only",
     "nur Radio",
     "",
     "Solo radio",
     "Alleen Radio",
     "",
     "",
     "",
     "vain radio",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Pouze radio",                        // Èesky (Czech)
     },
    {"TV only",
     "nur TV",
     "",
     "Solo TV",
     "Alleen TV",
     "",
     "",
     "",
     "vain TV",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Pouze TV",                        // Èesky (Czech)
     },
    {"HDTV only",
     "nur HDTV",
     "",
     "Solo HDTV",
     "Alleen HDTV",
     "",
     "",
     "",
     "vain HDTV",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Pouze HDTV",                        // Èesky (Czech)
     },
    {"Radio + TV",
     "Radio + TV",
     "",
     "Radio + TV",
     "Radio + TV",
     "",
     "",
     "",
     "radio + TV",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Radio + TV",                        // Èesky (Czech)
     },
    {"Radio + TV + NVOD",
     "Radio + TV + NVOD",
     "",
     "Radio + TV + NVOD",
     "Radio + TV + NVOD",
     "",
     "",
     "",
     "radio + TV + NVOD",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Radio + TV + NVOD",                        // Èesky (Czech)
     },
    {"Service type",
     "Serviceart",
     "",
     "Tipo servizio",
     "Service type",
     "",
     "",
     "",
     "Haettavat palvelut",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Servisni typ",                        // Èesky (Czech)
     },
    {"enabled",
     "aktiviert",
     "",
     "abilitato",
     "ingeschakeld",
     "",
     "",
     "",
     "päällä",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Povoleno",                        // Èesky (Czech)
     },
    {"disabled",
     "deaktiviert",
     "",
     "disabilitato",
     "uitgeschakeld",
     "",
     "",
     "",
     "pois",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Zakazano",                        // Èesky (Czech)
     },
    {"Added new channels",
     "Neue Kanäle hinzugefügt",
     "",
     "Aggiunti nuovi canali",
     "Nieuwe kanalen toegevoegd",
     "",
     "",
     "",
     "Uudet kanavat lisätty",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Pridat nove kanaly",                        // Èesky (Czech)
     },
    {"Frequency (kHz)",
     "Frequenz (kHz)",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Frekvence (kHz)",                        // Èesky (Czech)
     },
    {"Frequency (MHz)",
     "Frequenz (MHz)",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Frekvence (MHz)",                        // Èesky (Czech)
     },
    {"Expert",
     "Experten",
     "",
     "Esperto",
     "Expert",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Specialni",                        // Èesky (Czech)
     },
    {"Channel sel.",
     "Kanalliste",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Vyber kanalu",                        // Èesky (Czech)
     },
    {"DVB-S - Satellite",
     "DVB-S - Satellit",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "DVB-S - Satelit",                        // Èesky (Czech)
     },
    {"DVB-C - Cable",
     "DVB-C - Kabel",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "DVB-C - Kabel",                        // Èesky (Czech)
     },
    {"DVB-T - Terrestrial",
     "DVB-T - Terrestrisch",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "DVB-T - Terrestial",                        // Èesky (Czech)
     },
    {"Press OK to proceede",
     "Drücken Sie OK um fortzufahren",
     "",
     "Premere OK per continuare",
     "Druk OK om te vervolgen",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Stisknout OK pro pokracovani",                        // Èesky (Czech)
     },
    {"Press OK to finish or Exit for new scan",
     "Drücken Sie OK zum Beenden oder Exit für eine neue Kanalsuche.",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Stisknout OK pro konec nebo Exit pro nove prohledavani",                        // Èesky (Czech)
     },
    {"Standard",
     "Standard",
     "",
     "Standard",                // Italiano
     "Standaard",
     "",
     "Standart",
     "",
     "Vakio",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Standardni",                        // Èesky (Czech)
     },
    {"Extended",
     "Erweitert",
     "",
     "",
     "",
     "",
     "Précision",
     "",
     "Laaja",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Eesti
     "",                        // Dansk
     "Rozsirene",                        // Èesky (Czech)
     },
    {"Retrieving transponder list from %s",
     "Erhalte Transponderliste von %s",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Hrvatski (Croatian)
     "",                        // Eesti
     "",                        // Dansk
     "Ziskat transponder ze seznamu %s",                        // Èesky (Czech)
     },
    {"Terrestrial",
     "Terr.",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Eesti
     "",                        // Dansk
     "Terrestrial",                        // Èesky (Czech)
     },
    {"Cable",
     "Kabel",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Eesti
     "",                        // Dansk
     "Kabel",                        // Èesky (Czech)
     },
    {"Running services on transponder: %i / %i",
     "Aktive Dienste auf Transponder: %i / %i",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Eesti
     "",                        // Dansk
     "Informace o transponderu: %i / %i",                        // Èesky (Czech)
     },
    {"Scanning %s (%iMHz)\t%s",
     "Durchsuche %s (%iMHz)\t%s",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Eesti
     "",                        // Dansk
     "Prohledavani %s (%iMHz)\t%s",                        // Èesky (Czech)
     },
    {"Scanning %s (%.3fMHz)\t%s",
     "Durchsuche %s (%.3fMHz)\t%s",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Eesti
     "",                        // Dansk
     "Prohledavani %s (%.3fMHz)\t%s",                        // Èesky (Czech)
     },
    {"Changes Done",
     "Änderung übernommen",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Eesti
     "",                        // Dansk
     "Zmeny provedeny",                        // Èesky (Czech)
     },
    {"Changes failed",
     "Änderung fehlgeschlagen",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Eesti
     "",                        // Dansk
     "Zmeny neprovedeny",                        // Èesky (Czech)
     },
    {"Channel lists",
     "Kanallisten",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Eesti
     "",                        // Dansk
     "Seznam kanalu",                        // Èesky (Czech)
     },
    {"Channel lists functions",
     "Kanallisten Funktionen",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Eesti
     "",                        // Dansk
     "",                        // Èesky (Czech)
     },
    {"Please check your Timers!",
     "Bitte Timer überprüfen!",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Eesti
     "",                        // Dansk
     "Prosim zkontrolujte nastaveni casovace",                        // Èesky (Czech)
     },
    {"Intelligent 6900/6875/6111",
     "Intelligent 6900/6875/6111",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Eesti
     "",                        // Dansk
     "Inteligentni 6900/6875/6111",                        // Èesky (Czech)
     },
    {"Try all 6900/6875/6111",
     "Versuche alle 6900/6875/6111",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Eesti
     "",                        // Dansk
     "Zkuste vsechny 6900/6875/6111",                        // Èesky (Czech)
     },
    {"Manual",
     "Manuell",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Eesti
     "",                        // Dansk
     "Rucne",                        // Èesky (Czech)
     },
    {"Symbol Rates",
     "Symbolraten",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Eesti
     "",                        // Dansk
     "Symbol Rate",                        // Èesky (Czech)
     },
    {"Scanning aborted",
     "Scan abgebrochen",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Eesti
     "",                        // Dansk
     "Prohledavani zruseno",                        // Èesky (Czech)
     },
    {"Enable Logfile",
     "Logfile einschalten",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Eesti
     "",                        // Dansk
     "Logfile povoleno",                        // Èesky (Czech)
     },
    {"Skip",
     "Auslassen",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Eesti
     "",                        // Dansk
     "Preskocit",                        // Èesky (Czech)
     },
    {"Recording is running",
     "Nicht möglich - Aufnahme läuft",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Eesti
     "",                        // Dansk
     "Nahravani je spusteno",                        // Èesky (Czech)
     },
    {"Channel List successfully stored",
     "Kanalliste erfolgreich gespeichert.",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Eesti
     "",                        // Dansk
     "",                        // Èesky (Czech)
     },
    {"Channel List successfully loaded",
     "Kanalliste erfolgreich importiert.",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Eesti
     "",                        // Dansk
     "",                        // Èesky (Czech)
     },
    {"Functions",
     "Funktionen",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Eesti
     "",                        // Dansk
     "",                        // Èesky (Czech)
     },
    {"Import current channellist",
     "Aktuelle Kanalliste importieren",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Eesti
     "",                        // Dansk
     "",                        // Èesky (Czech)
     },
    {"Export current channellist",
     "Aktuelle Kanalliste exportieren",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Eesti
     "",                        // Dansk
     "",                        // Èesky (Czech)
     },
    {"Backup channel list to...",
     "Sichere aktuelle Kanalliste nach...",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Eesti
     "",                        // Dansk
     "",                        // Èesky (Czech)
     },
    {"Restore channel list from...",
     "Ersetze aktuelle Kanalliste durch...",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Eesti
     "",                        // Dansk
     "",                        // Èesky (Czech)
     },
    {"Restore failed!",
     "Ersetzen fehlgeschlagen!",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Eesti
     "",                        // Dansk
     "",                        // Èesky (Czech)
     },
    {"Backup failed!",
     "Sichern der Kanalliste fehlgeschlagen!",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Eesti
     "",                        // Dansk
     "",                        // Èesky (Czech)
     },    
     {"Press OK to go back to rotor settings",
     "Drücken sie Ok um zu den Rotoreinstellungen zurückzukehren",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "",                        // Eesti
     "",                        // Dansk
     "",                        // Èesky (Czech)
     },
    {NULL}
};
