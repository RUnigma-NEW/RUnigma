#!/bin/sh

input=$1            
                                              
pngfile="/tmp/vdump.png"
vmode=`cat /proc/stb/video/videomode`
stream_aspect="undef"

if [ -z $input ]; then
	# hole e2service referenc nummer
	e2service=`wget -q -O - http://127.0.0.1/web/getcurrent | grep "<e2servicereference>" | sed s/".*<e2servicereference>"/""/ | sed s/"<\/e2servicereference>"/""/`
else
	e2service=$input
fi

rm -f "$pngfile"

# check mode
if [ "$vmode" == "576p50" ] || [ "$vmode" == "576i50" ] || [ "$vmode" == "pal" ]; then
  echo "[grab.sh] videomod = $vmode"
  stream_aspect=`cat /proc/stb/vmpeg/0/aspect`
fi
                         
# erzeuge tv bild
ffmpeg -itsoffset -4 -i "http://127.0.0.1:8001/$e2service" -vframes 1 -vcodec png -sn -an -y -f image2 "$pngfile"
#ffmpeg -itsoffset -4 -i "http://127.0.0.1:8001/1:0:1:2EE3:441:1:C00000:0:0:0:" -vframes 1 -vcodec png -sn -an -y -f image2 "$pngfile"
echo "[grab.sh] used video grab file: $pngfile"

# merge tv bild mit osd bild
if [ "$stream_aspect" == "1" ]; then
	echo "[grab.sh] convert to 16:9"
  grab -r 1024 -i 576 -p -f "$pngfile"
else
  grab -p -f "$pngfile"
fi
rm -f "$pngfile"
mv /tmp/screenshot.png /tmp/dump.png