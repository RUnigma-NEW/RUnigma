#!/bin/bash

if [ "$1" == -h ] || [ "$1" == --help ]; then
 echo "Parameter 1: Поддерживаемая платформа (1)"
 echo "Parameter 2: Ядро (1-3)"
 echo "Parameter 3: Режим отладки (Y/N)"
 echo "Parameter 4: Плэйер (1)"
 echo "Parameter 5: Мультиком (1)"
 echo "Parameter 6: Мультимедийная платформа (1-2)"
 echo "Parameter 7: Поддержка внешнего LCD-дисплея (1-2)"
 echo "Parameter 8: Графическая платформа (1-2)"
 exit
fi

CURDIR=`pwd`
KATIDIR=${CURDIR%/cvs/cdk}
export PATH=/usr/sbin:/sbin:$PATH
DIALOG=${DIALOG:-`which dialog`}

CONFIGPARAM=" \
 --enable-maintainer-mode \
 --prefix=$KATIDIR/tufsbox \
 --with-cvsdir=$KATIDIR/cvs \
 --with-customizationsdir=$KATIDIR/custom \
 --with-archivedir=$HOME/Archive \
 --enable-ccache"

##############################################
echo ""
echo "---------------------------------------"
echo ""
echo "Вас приветствует TeslaNet Piterkadet Greder"
echo ""
echo "---------------------------------------"
echo ""
echo "Подтвердите, пожалуйста, параметры компиляции"
echo ""
echo "---------------------------------------"
##############################################

# config.guess generates different answers for some packages
# Ensure that all packages use the same host by explicitly specifying it.

# First obtain the triplet
AM_VER=`automake --version | awk '{print $NF}' | grep -oEm1 "^[0-9]+.[0-9]+"`
host_alias=`/usr/share/automake-${AM_VER}/config.guess`

# Then undo Suse specific modifications, no harm to other distribution
case `echo ${host_alias} | cut -d '-' -f 1` in
  i?86) VENDOR=pc ;;
  *   ) VENDOR=unknown ;;
esac
host_alias=`echo ${host_alias} | sed -e "s/suse/${VENDOR}/"`

# And add it to the config parameters.
CONFIGPARAM="${CONFIGPARAM} --host=${host_alias} --build=${host_alias}"

##############################################
echo ""
echo ""
echo ""
echo "Поддерживаемая платформа:"
echo "---------------------------------------"
echo "   Opticum 9500 HD (HL-101)"
TARGET="--enable-hl101"
CONFIGPARAM="$CONFIGPARAM $TARGET"

##############################################
echo ""
echo -e "Ядро:"
echo "---------------------------------------"
#echo "   1) STM 24 P0207"
#echo "   2) STM 24 P0210"
echo "   STM 24 P0211"
#echo "   4) STM 24 P0302"
#case $2 in
#        [1-4]) REPLY=$2
#        echo -e "\nВыбранное ядро: $REPLY\n"
#        ;;
#        *)
#        read -p "Выберите ядро (1-3)? ";;
#esac

#case "$REPLY" in
#	1) KERNEL="--enable-stm24 --enable-p0207";STMFB="stm24";;
#	2) KERNEL="--enable-stm24 --enable-p0210";STMFB="stm24";;
#	3) KERNEL="--enable-stm24 --enable-p0211";STMFB="stm24";;
#	4) KERNEL="--enable-stm24 --enable-p0302";STMFB="stm24";;
#	*) KERNEL="--enable-stm24 --enable-p0211";STMFB="stm24";;
#esac
KERNEL="--enable-stm24 --enable-p0211"
STMFB="stm24"
CONFIGPARAM="$CONFIGPARAM $KERNEL"

##############################################

echo "   python 2.7.3"
PYTHON="--enable-py27"

echo ""
echo -e "\npython:"
echo " 1) Python 2.7.3 - по умолчанию"
echo " 2) Python 2.7.5 - test"
echo " 3) Python 3.3.2 - test"
case $4 in
	[1-3]) REPLY=$4
	echo -e "\nВыбор Python: $REPLY\n"
	;;
	*)
	read -p "Выбор Python (1-3)? ";;
esac
echo "---------------------------------------"
case "$REPLY" in
	1 ) PYTHON="--enable-py273"
	echo "   Python 2.7.3" ;;
	2 ) PYTHON="--enable-py275"
	echo "   Python 2.7.5" ;;
	3 ) PYTHON="--enable-py332"
	echo "   Python 3.3.2" ;;
	*) PYTHON="--enable-py273"
	echo "   Python 2.7.3" ;;
esac
##############################################
#if [ "$3" ]; then
#REPLY="$3"
#echo "   Активировать отладку (y/N)? "
#echo -e "\nВыберите вариант: $REPLY\n"
#else
#REPLY=N
#read -p "   Активировать отладку (y/N)? "
echo "---------------------------------------"
#fi
[ "$REPLY" == "y" -o "$REPLY" == "Y" ] && CONFIGPARAM="$CONFIGPARAM --enable-debug"

##############################################

cd ../driver/
echo "# Automatically generated config: don't edit" > .config
echo "#" >> .config
echo "export CONFIG_ZD1211REV_B=y" >> .config
echo "export CONFIG_ZD1211=n"		>> .config
cd - > /dev/null 2>&1

##############################################
echo ""
echo -e "\nПлэйер:"
echo "---------------------------------------"
echo "   Плэйер 191"
PLAYER="--enable-player191"
cd ../driver/include/
#if [ -L player2 ]; then
#   rm player2
#fi

if [ -L stmfb ]; then
   rm stmfb
fi
#ln -s player2_191 player2
ln -s ../stgfb/stmfb-3.1_stm24_0104/include stmfb
cd - > /dev/null 2>&1

#cd ../driver/
#if [ -L player2 ]; then
#   rm player2
#fi
#ln -s player2_191 player2
echo "export CONFIG_PLAYER_191=y" >> .config
#cd - > /dev/null 2>&1

cd ../driver/stgfb
if [ -L stmfb ]; then
   rm stmfb
fi
#if [ "$STMFB" == "stm24" ]; then
ln -s stmfb-3.1_stm24_0104 stmfb
#else
#    ln -s stmfb-3.1_stm23_0032 stmfb
#fi
cd - > /dev/null 2>&1

##############################################
echo ""
echo -e "\nМультиком:"
echo " 1) Мультиком 4.0.6"
echo " 2) Мультиком 3.2.4 - по умолчанию"
case $5 in
	[1-2]) REPLY=$5
	echo -e "\nSelected multicom: $REPLY\n"
	;;
	*)
	read -p "Select multicom (1-2)? ";;
esac
echo "---------------------------------------"
case "$REPLY" in
	1 ) MULTICOM="--enable-multicom406"
	echo "   Мультиком 4.0.6 "
	cd ../driver/include/
	if [ -L multicom ]; then
		rm multicom
	fi

	ln -s ../multicom-4.0.6/include multicom
	cd - > /dev/null 2>&1

	cd ../driver/
	if [ -L multicom ]; then
		rm multicom
	fi

	ln -s multicom-4.0.6 multicom
	echo "export CONFIG_MULTICOM406=y" >> .config
	cd - > /dev/null 2>&1
	;;
	2 ) MULTICOM="--enable-multicom324"
	echo "   Мультиком 3.2.4 "
	cd ../driver/include/
	if [ -L multicom ]; then
		rm multicom
	fi

	ln -s ../multicom-3.2.4/include multicom
	cd - > /dev/null 2>&1

	cd ../driver/
	if [ -L multicom ]; then
		rm multicom
	fi

	ln -s multicom-3.2.4 multicom
	echo "export CONFIG_MULTICOM324=y" >> .config
	cd - > /dev/null 2>&1
	;;
	*) MULTICOM="--enable-multicom324"
	echo "   Мультиком 3.2.4 "
	cd ../driver/include/
	if [ -L multicom ]; then
		rm multicom
	fi

	ln -s ../multicom-3.2.4/include multicom
	cd - > /dev/null 2>&1

	cd ../driver/
	if [ -L multicom ]; then
		rm multicom
	fi

	ln -s multicom-3.2.4 multicom
	echo "export CONFIG_MULTICOM324=y" >> .config
	cd - > /dev/null 2>&1
	;;
esac
##############################################
echo ""
echo -e "\nПоддержка внешнего LCD-дисплея:"
echo "---------------------------------------"
echo "   Без поддержки LCD-дисплея"
EXTERNAL_LCD=""
##############################################

#CONFIGPARAM="$CONFIGPARAM $PLAYER $MULTICOM $MEDIAFW $EXTERNAL_LCD"

echo -e "\nГрафическая платформа:"
echo "   1) Enigma2 OpenPli"
#echo "   2) XBMC"
#echo "   3) Enigma2"
#case $8 in
#        [1-3]) REPLY=$8
#        echo -e "\nSelected Image: $REPLY\n"
#        ;;
#        *)
#        read -p "Select Image (1-3)? ";;
#esac
#case "$REPLY" in
#                [1-3])
#                if [ "$REPLY" == 1 ]; then
#                    echo -e "\nВыберите один из вариантов:"
#                        echo "   0) Самый свежий вариант   - E2 OpenPli gstreamer / libplayer3 (Возможны проблемы с компиляцией) "
#                        echo "   1) Sat, 17 Mar 2012 19:51 - E2 OpenPli gstreamer              945aeb939308b3652b56bc6c577853369d54a537"
#                        echo "   2) Sat, 18 Aug 2012 11:12 - E2 OpenPli gstreamer / libplayer3 4f3396b610f5524d85e06f51cbd3186b75f4b6d3"
#                        echo "   3) Sat, 20 Aug 2012 19:08 - E2 OpenPli gstreamer / libplayer3 51a7b9349070830b5c75feddc52e97a1109e381e"
#                        echo "   4) Sat, 24 Aug 2012 23:42 - E2 OpenPli gstreamer / libplayer3 002b85aa8350e9d8e88f75af48c3eb8a6cdfb880"
#                        echo "   5) Fri, 05 Oct 2012 21:37 - E2 OpenPli gstreamer / libplayer3 7e38f7f6c911cd16106fb3b131e5c2d3a7ea51c7"
#                        echo "   6) AR-P - E2 OpenPli branch testing"
                        echo "   7) AR-P - E2 OpenPli branch last - по умолчанию"
#                        echo "   8) AR-P - E2 OpenPli branch master"
#                        echo "   9) Sat, 17 Sep 2012 17:19 - E2 OpenAAF gstreamer / libplayer3 0f7fa25f26091617213e85b0ed440beb67612ce3"
#                        echo ""
#                        echo ""
#                    read -p "Выберите enigma2 OpenPli (0-9):"
#                       
#                        case "$REPLY" in
#                        0) IMAGE="--enable-e2pd0";;
#                        1) IMAGE="--enable-e2pd1";;
#                        2) IMAGE="--enable-e2pd2";;
#                        3) IMAGE="--enable-e2pd3";;
#                        4) IMAGE="--enable-e2pd4";;
#                        5) IMAGE="--enable-e2pd5";;
#                        6) IMAGE="--enable-e2pd6";;
#                        7) IMAGE="--enable-e2pd7";;
#                        8) IMAGE="--enable-e2pd8";;
#                        9) IMAGE="--enable-e2pd9";;
#                        *) IMAGE="--enable-e2pd7";;
#                        esac
#                elif [ "$REPLY" == 2 ]; then
#                    echo -e "\nВыберите один из вариантов:"
#                        echo "  0) Самый свежий вариант (Возможны проблемы с компиляцией)"
#                        echo "  1) Sat, 14 Apr 2012 12:36 - 460e79416c5cb13010456794f36f89d49d25da75"
#                        echo "  2) Sun, 10 Jun 2012 13:53 - 327710767d2257dad27e3885effba1d49d4557f0"
#                        echo "  3) Fr,  31 Aug 2012 22:34 - Frodo_alpha5 - 12840c28d8fbfd71c26be798ff6b13828b05b168"
#                        echo "  4) Fr,  31 Oct 2012 22:34 - Frodo_alpha7 - e292b1147bd89a7e53742e3e5039b9a906a3b1d0"
#                        echo ""
#                        echo ""
#                    read -p "Выберите XBMC (0-4):"
#                        case "$REPLY" in
#                        0) IMAGE="--enable-xbd0" GFW="--enable-graphicfwdirectfb" MEDIAFW="--enable-mediafwgstreamer";;
#                        1) IMAGE="--enable-xbd1" GFW="--enable-graphicfwdirectfb" MEDIAFW="--enable-mediafwgstreamer";;
#                        2) IMAGE="--enable-xbd2" GFW="--enable-graphicfwdirectfb" MEDIAFW="--enable-mediafwgstreamer";;
#                        3) IMAGE="--enable-xbd3" GFW="--enable-graphicfwdirectfb" MEDIAFW="--enable-mediafwgstreamer";;
#                        4) IMAGE="--enable-xbd4" GFW="--enable-graphicfwdirectfb" MEDIAFW="--enable-mediafwgstreamer";;
#                        *) IMAGE="--enable-xbd3" GFW="--enable-graphicfwdirectfb" MEDIAFW="--enable-mediafwgstreamer";;
#                        esac
#                elif [ "$REPLY" == 3 ]; then
#                    echo -e "\nВыберите один из вариантов:"
#                        echo "  0) Самый свежий вариант (Возможны проблемы с компиляцией)"
#                        echo "  1) Sat, 29 Mar 2011 13:49 - E2 V3.0 e013d09af0e010f15e225a12dcc217abc052ee19"
#                        echo "  2) inactive"
#                        echo "  3) inactive"
#                        echo "  4) inactive"
#                        echo "  5) Fri,  5 Nov 2010 00:16 - E2 V2.4 libplayer3 7fd4241a1d7b8d7c36385860b24882636517473b"
#                        echo "  6) Wed,  6 Jul 2011 11:17 - E2 V3.1 gstreamer  388dcd814d4e99720cb9a6c769611be4951e4ad4"
#                        echo "  7) Current E2 gitgui arp-team gstreamer / eplayer3 - по умолчанию"
#                        echo "  8) Current E2 gitgui arp-team no gstreamer"
#                        echo ""
                        echo ""
#                    read -p "Выберите enigma2 (0-8):"
#                        case "$REPLY" in
#                        0) IMAGE="--enable-e2d0";;
#                        1) IMAGE="--enable-e2d1";;
#                        2) IMAGE="--enable-e2d2";;
#                        3) IMAGE="--enable-e2d3";;
#                        4) IMAGE="--enable-e2d4";;
#                        5) IMAGE="--enable-e2d5";;
#                        6) IMAGE="--enable-e2d6";;
#                        7) IMAGE="--enable-e2d7";;
#                        8) IMAGE="--enable-e2d8";;
#                        *) IMAGE="--enable-e2d7";;
#                        esac
#                elif  [ "$REPLY" == 4 ]; then
#                    echo -e "\nChoose VDR revisions"
#                        echo "   1) VDR-1.7.22"
#                        echo "   2) VDR-1.7.27"
#                    read -p "Select VDR-1.7.XX (1-2)? "
#                        case "$REPLY" in
#                        1) IMAGE="--enable-vdr1722"
#                            cd ../apps/vdr/
#                        if [ -L vdr ]; then
#                            rm vdr
#                        fi
#                            ln -s vdr-1.7.22 vdr
#                        cd -
#                        ;;
#                        2) IMAGE="--enable-vdr1727"
#                            cd ../apps/vdr/
#                        if [ -L vdr ]; then
#                            rm vdr
#                        fi
#                            ln -s vdr-1.7.27 vdr
#                        cd -
#                        ;;
#                        *) IMAGE="--enable-vdr1722";;
#                        esac
#                fi
#                ;;
#        *)
#esac
IMAGE="--enable-e2pd7"
##############################################
case "$IMAGE" in
    --enable-e2*)
    echo ""
    echo -e "\nМультимедийная платформа:"
    echo "---------------------------------------"
    echo "   1) eplayer3 - по умолчанию"
    echo "   2) gstreamer"
    echo ""
    case $6 in
            [1-2]) REPLY=$6
            echo -e "\nВыбранная мультимедийная платформа: $REPLY\n"
            ;;
            *)
            read -p "Выберите мультимедийную платформу (1-2)? ";;
    esac
    
    case "$REPLY" in
    	1) MEDIAFW="--enable-eplayer3";;
    	2) MEDIAFW="--enable-mediafwgstreamer";;
    	*) MEDIAFW="--enable-eplayer3";;
    esac
esac

##############################################

CONFIGPARAM="$CONFIGPARAM $PLAYER $PYTHON $MULTICOM $MEDIAFW $EXTERNAL_LCD $IMAGE $GFW"

##############################################
# configure still want's this
# ignore errors here
automake --add-missing

echo && \
echo "Performing autogen.sh..." && \
echo "------------------------" && \
./autogen.sh && \
echo && \
echo "Performing configure..." && \
echo "-----------------------" && \
echo && \
##############################################
./configure $CONFIGPARAM
##############################################
echo $CONFIGPARAM >lastChoice
##############################################
echo ""
echo ""
echo "-----------------------"
echo "Параметры Вашей компиляции подтверждены!"
echo "-----------------------"
echo ""
echo "Теперь Вы должны выбрать один из вариантов компиляции:"
echo "-----------------------"
echo ""
opt=${?}
case "$IMAGE" in
    --enable-e2pd*)
        echo "1) make yaud-enigma2-pli-nightly"
        echo "2) make yaud-enigma2-pli-nightly-full - по умолчанию"
    case $7 in
            [1-2]) REPLY=$7
            echo -e "\nвыберите вариант компиляции: $REPLY\n"
            ;;
            *)
            read -p "Выберите вариант компиляции ";;
    esac
    case "$REPLY" in
    	1)MKTARGET="yaud-enigma2-pli-nightly";;
    	2)MKTARGET="yaud-enigma2-pli-nightly-full";;
    	*)MKTARGET="yaud-enigma2-pli-nightly-full";;
    esac
esac
case "$IMAGE" in
    --enable-xbd*)
        echo "make yaud-xbmc-nightly"
        MKTARGET="yaud-xbmc-nightly";;
    --enable-e2d*)
        echo "make yaud-enigma2-nightly"
        MKTARGET="yaud-enigma2-nightly";;
esac
echo ""
echo "-----------------------"
##############################################
make ${MKTARGET} 2>&1
