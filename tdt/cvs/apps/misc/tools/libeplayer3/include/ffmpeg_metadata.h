#ifndef _ffmpeg_metadata_123
#define _ffmpeg_metadata_123

/* these file contains a list of metadata tags which can be used by applications
 * to stream specific information. it maps the tags to ffmpeg specific tags.
 *
 * fixme: if we add other container for some resons later (maybe some other libs
 * support better demuxing or something like this), then we should think on a
 * more generic mechanism!
 */

/* metatdata map list:
 */
char* metadata_map[] =
{
 /* our tags      ffmpeg tag / id3v2 */  
   "Title",       "title",
   "Artist",      "artist",
   "Albumartist", "album_artist",
   "Album",       "album",
   "Year",        "date",  /* fixme */
   "Comment",     "unknown",
   "Track",       "track",
   "Copyright",   "copyright",
   "Composer",    "composer",
   "Genre",       "genre",
   "EncodedBy",   "encoded_by",
   "Language",    "language",
   "Performer",   "performer",
   "Publisher",   "publisher",
   "Encoder",     "encoder",
   "Disc",        "disc",
   NULL
};

#endif
