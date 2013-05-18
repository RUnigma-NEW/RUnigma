/****************************************************************************
 * DESCRIPTION: 
 *             Internationalisation Strings
 *
 * $Id: i18n.cpp,v 1.15 2006/03/05 09:47:25 ralf Exp $
 *
 * Contact:    ranga@vdrtools.de
 *
 * Copyright (C) 2004 by Ralf Dotzert 
 ****************************************************************************/
#include "i18n.h"

const tI18nPhrase Phrases[] = {
  { "VDR-Setup Extension", // English
    "VDR-Setup Erweiterung", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Português
    "Extension de configuration", // Français
    "", // Norsk
    "", // suomi
    "", // Polski
    "", // Español
    "", // ÅëëçíéêÜ (Greek)
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Català
    "", // ÀãááÚØÙ (Russian)
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // Èesky (Czech)
  },
  { "Setup",
    "Einstellungen",
    "Nastavitve",
    "Opzioni",
    "Instellingen",
    "Configurar",
    "Configuration",
    "Konfigurasjon",
    "Asetukset",
    "Ustawienia",
    "Configuración",
    "Ñõèìéóåéò",
    "Inställningar",
    "Configuraşie",
    "Beállítások",
    "Configuració",
    "½ĞáâàŞÙÚĞ",
    "Konfiguracija",
    "Sätted",
    "Indstillinger",
    "Nastavení",
  },
  { "Menu Edit",
    "Menü bearbeiten",
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Português
    "Edition du menu", // Français
    "", // Norsk
    "", // suomi
    "", // Polski
    "", // Español
    "", // ÅëëçíéêÜ (Greek)
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Català
    "", // ÀãááÚØÙ (Russian)
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // Èesky (Czech)
  },
  { "Move",
    "Verschieben",
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Português
    "Déplacer", // Français
    "", // Norsk
    "", // suomi
    "", // Polski
    "", // Español
    "", // ÅëëçíéêÜ (Greek)
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Català
    "", // ÀãááÚØÙ (Russian)
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // Èesky (Czech)
  },
  { "Create",
    "Neu",
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Português
    "Créer", // Français
    "", // Norsk
    "", // suomi
    "", // Polski
    "", // Español
    "", // ÅëëçíéêÜ (Greek)
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Català
    "", // ÀãááÚØÙ (Russian)
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // Èesky (Czech)
  },
  { "Edit",
    "Editieren",
    "Uredi",
    "Modifica",
    "Verander",
    "Modificar",
    "Modifier",
    "Editer",
    "Muokkaa",
    "Edytuj",
    "Modificar",
    "ĞñïóáñìïãŞ",
    "Ändra",
    "Modificã",
    "Beállítani",
    "Editar",
    "ÀÕÔĞÚâØàŞÒĞâì",
    "Promijeni",
    "Muuda",
    "Rediger",
    "Editace",
  },
  { "Title",
    "Titel",
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Português
    "", // Français
    "", // Norsk
    "", // suomi
    "", // Polski
    "", // Español
    "", // ÅëëçíéêÜ (Greek)
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Català
    "", // ÀãááÚØÙ (Russian)
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // Èesky (Czech)
  },
  { "Command",
    "Befehl",
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Português
    "", // Français
    "", // Norsk
    "", // suomi
    "", // Polski
    "", // Español
    "", // ÅëëçíéêÜ (Greek)
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Català
    "", // ÀãááÚØÙ (Russian)
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // Èesky (Czech)
  },
  { "Has to confirm",
    "Zu bestätigen",
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Português
    "", // Français
    "", // Norsk
    "", // suomi
    "", // Polski
    "", // Español
    "", // ÅëëçíéêÜ (Greek)
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Català
    "", // ÀãááÚØÙ (Russian)
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // Èesky (Czech)
  },
  { "Execute as thread",
    "Als Thread ausführen",
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Português
    "", // Français
    "", // Norsk
    "", // suomi
    "", // Polski
    "", // Español
    "", // ÅëëçíéêÜ (Greek)
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Català
    "", // ÀãááÚØÙ (Russian)
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // Èesky (Czech)
  },
  { "Create Command",
    "Neuer Befehl",
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Português
    "", // Français
    "", // Norsk
    "", // suomi
    "", // Polski
    "", // Español
    "", // ÅëëçíéêÜ (Greek)
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Català
    "", // ÀãááÚØÙ (Russian)
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // Èesky (Czech)
  },
  { "Create Menu",
    "Neues Menü",
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Português
    "", // Français
    "", // Norsk
    "", // suomi
    "", // Polski
    "", // Español
    "", // ÅëëçíéêÜ (Greek)
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Català
    "", // ÀãááÚØÙ (Russian)
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // Èesky (Czech)
  },
  { "Edit Command",
    "Editiere Befehl",
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Português
    "", // Français
    "", // Norsk
    "", // suomi
    "", // Polski
    "", // Español
    "", // ÅëëçíéêÜ (Greek)
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Català
    "", // ÀãááÚØÙ (Russian)
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // Èesky (Czech)
  },
  { "Edit Plugin",
    "Editiere Plugin",
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Português
    "", // Français
    "", // Norsk
    "", // suomi
    "", // Polski
    "", // Español
    "", // ÅëëçíéêÜ (Greek)
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Català
    "", // ÀãááÚØÙ (Russian)
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // Èesky (Czech)
  },
  { "Edit Menu",
    "Editiere Menü",
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Português
    "", // Français
    "", // Norsk
    "", // suomi
    "", // Polski
    "", // Español
    "", // ÅëëçíéêÜ (Greek)
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Català
    "", // ÀãááÚØÙ (Russian)
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // Èesky (Czech)
  },
  { "Delete Menu?",
    "Menü löschen?",
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Português
    "Effacer le menu?", // Français
    "", // Norsk
    "", // suomi
    "", // Polski
    "", // Español
    "", // ÅëëçíéêÜ (Greek)
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Català
    "", // ÀãááÚØÙ (Russian)
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // Èesky (Czech)
  },
  { "Delete",
    "Löschen",
    "Izbri¹i",
    "Cancella",
    "Verwijder",
    "Apagar",
    "Supprimer",
    "Slett",
    "Poista",
    "Usuñ",
    "Borrar",
    "ÄéáãñáöŞ",
    "Ta bort",
    "ªterge",
    "Törölni",
    "Esborrar",
    "ÃÔĞÛØâì",
    "Obri¹i",
    "Kustuta",
    "Slet",
    "Smazat",
  },
  { "Before",
    "Davor",
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Português
    "Avant", // Français
    "", // Norsk
    "", // suomi
    "", // Polski
    "", // Español
    "", // ÅëëçíéêÜ (Greek)
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Català
    "", // ÀãááÚØÙ (Russian)
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // Èesky (Czech)
  },
  { "After",
    "Dahinter",
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Português
    "Après", // Français
    "", // Norsk
    "", // suomi
    "", // Polski
    "", // Español
    "", // ÅëëçíéêÜ (Greek)
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Català
    "", // ÀãááÚØÙ (Russian)
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // Èesky (Czech)
  },
  { "Into",
    "Hinein",
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Português
    "Dans", // Français
    "", // Norsk
    "", // suomi
    "", // Polski
    "", // Español
    "", // ÅëëçíéêÜ (Greek)
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Català
    "", // ÀãááÚØÙ (Russian)
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // Èesky (Czech)
  },
  { "Plugins activate / deactivate",
    "Plugins aktivieren / sortieren",
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Português
    "Activer/desactiver des plugins", // Français
    "", // Norsk
    "", // suomi
    "", // Polski
    "", // Español
    "", // ÅëëçíéêÜ (Greek)
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Català
    "", // ÀãááÚØÙ (Russian)
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // Èesky (Czech)
  },
  { "Reboot",
    "Reboot",
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Português
    "Redémarer le système", // Français
    "", // Norsk
    "", // suomi
    "", // Polski
    "", // Español
    "", // ÅëëçíéêÜ (Greek)
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Català
    "", // ÀãááÚØÙ (Russian)
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // Èesky (Czech)
  },
  { "protected",
    "geschützt",
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Português
    "protégé", // Français
    "", // Norsk
    "", // suomi
    "", // Polski
    "", // Español
    "", // ÅëëçíéêÜ (Greek)
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Català
    "", // ÀãááÚØÙ (Russian)
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // Èesky (Czech)
  },
  { "Error in configuration files",
    "Fehler in der VDR-Konfiguration",
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Português
    "Erreur dans fichiers de configurations", // Français
    "", // Norsk
    "", // suomi
    "", // Polski
    "", // Español
    "", // ÅëëçíéêÜ (Greek)
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Català
    "", // ÀãááÚØÙ (Russian)
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // Èesky (Czech)
  },
  { "Enter Pin: ",
    "Pin Eingabe: ",
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Português
    "Le code pin: ", // Français
    "", // Norsk
    "", // suomi
    "", // Polski
    "", // Español
    "", // ÅëëçíéêÜ (Greek)
    "", // Svenska
    "", // Romaneste
    "Pin kód: ", // Magyar
    "", // Català
    "¿Øİ ÚŞÔ: ", // ÀãááÚØÙ (Russian)
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // Èesky (Czech)
  },
  { "Invalid Pin!",
    "Ungültige Pin!",
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Português
    "La clef est fausse!", // Français
    "", // Norsk
    "", // suomi
    "", // Polski
    "", // Español
    "", // ÅëëçíéêÜ (Greek)
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Català
    "", // ÀãááÚØÙ (Russian)
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // Èesky (Czech)
  },
  { "missing channels/xxx.conf",
    "fehlende channels/xxx.conf",
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Português
    "channels/xxx.conf manquant", // Français
    "", // Norsk
    "", // suomi
    "", // Polski
    "", // Español
    "", // ÅëëçíéêÜ (Greek)
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Català
    "", // ÀãááÚØÙ (Russian)
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // Èesky (Czech)
  },
  { "Execute",
    "Starte",
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Português
    "Effectuer", // Français
    "", // Norsk
    "", // suomi
    "", // Polski
    "", // Español
    "", // ÅëëçíéêÜ (Greek)
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Català
    "", // ÀãááÚØÙ (Russian)
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // Èesky (Czech)
  },
  { "PageUp",
    "Seite ^",
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Português
    "Page ^", // Français
    "", // Norsk
    "", // suomi
    "", // Polski
    "", // Español
    "", // ÅëëçíéêÜ (Greek)
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Català
    "", // ÀãááÚØÙ (Russian)
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // Èesky (Czech)
  },
  { "PageDown",
    "Seite v",
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Português
    "Page v", // Français
    "", // Norsk
    "", // suomi
    "", // Polski
    "", // Español
    "", // ÅëëçíéêÜ (Greek)
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Català
    "", // ÀãááÚØÙ (Russian)
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // Èesky (Czech)
  },
  { "Up/Dn for new location - color key to move",
    "Auf/Ab für neue Position - dann Farbtaste",
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Português
    "^/v pour nouvel emplacement - touches couleurs pour déplacer", // Français
    "", // Norsk
    "", // suomi
    "", // Polski
    "", // Español
    "", // ÅëëçíéêÜ (Greek)
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Català
    "", // ÀãááÚØÙ (Russian)
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // Èesky (Czech)
  },
  { "Help",
    "Hilfe",
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Português
    "Aide", // Français
    "", // Norsk
    "", // suomi
    "", // Polski
    "", // Español
    "", // ÅëëçíéêÜ (Greek)
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Català
    "", // ÀãááÚØÙ (Russian)
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // Èesky (Czech)
  },
  { "Really reboot?",
    "Wirklich neu booten?",
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Português
    "Vraiment redémarrer le système?", // Français
    "", // Norsk
    "", // suomi
    "", // Polski
    "", // Español
    "", // ÅëëçíéêÜ (Greek)
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Català
    "", // ÀãááÚØÙ (Russian)
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // Èesky (Czech)
  },
  { "setupSetup$Main menu entry",
    "Hauptmenü Eintrag",
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Português
    "", // Français
    "", // Norsk
    "", // suomi
    "", // Polski
    "", // Español
    "", // ÅëëçíéêÜ (Greek)
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Català
    "", // ÀãááÚØÙ (Russian)
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // Èesky (Czech)
  },
  { "setupSetup$Menu suffix",
    "Menü Suffix",
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Português
    "Suffixe Menu", // Français
    "", // Norsk
    "", // suomi
    "", // Polski
    "", // Español
    "", // ÅëëçíéêÜ (Greek)
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Català
    "", // ÀãááÚØÙ (Russian)
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // Èesky (Czech)
  },
  { "setupSetup$Entry prefix",
    "Entry Prefix",
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Português
    "Entry préfix", // Français
    "", // Norsk
    "", // suomi
    "", // Polski
    "", // Español
    "", // ÅëëçíéêÜ (Greek)
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Català
    "", // ÀãááÚØÙ (Russian)
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // Èesky (Czech)
  },
  { "setupSetup$Return value",
    "Rückgabe Wert",
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Português
    "Valeur de retour", // Français
    "", // Norsk
    "", // suomi
    "", // Polski
    "", // Español
    "", // ÅëëçíéêÜ (Greek)
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Català
    "", // ÀãááÚØÙ (Russian)
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // Èesky (Czech)
  },
  { NULL }
};
