echo "Копирую образ в Оперативную память"
echo "Копирую" > /dev/vfd
cd /rootfs
cp service/*.tar.gz /install
echo "Готово"
if [ -e /rootfs/etc/enigma2 ]; then
	echo "Сохраняю"
	echo "Сохраняю настройки" > /dev/vfd
	tar -czvf /install/backup/E2Settings.tar.gz etc/enigma2/* etc/tuxbox/* var/emu/* var/keys/* etc/init.d/softcam* > /dev/null 2>&1
fi
if [ -e /rootfs/backup/*backup.tar.gz ]; then
	echo "Сохраняю"
	echo "Сохраняю настройки" > /dev/vfd
	cp backup/*backup.tar.gz /install/backup/backup.tar.gz
fi
cd /
echo "Копирую загрузчик..."
cd /rootfs/boot
cp uImage /install
echo "Демонтирую загрузочный раздел /dev/sda1"
cd /
umount /dev/sda1
echo "Готово"
echo "Формат" > /dev/vfd
echo "Готовлю структуру разбиения диска"
HDD=/dev/sda
dd if=/dev/zero of=$HDD bs=512 count=64
sfdisk --re-read $HDD
# Löscht die Festplatte/Stick und erstellt 4 Partitionen
#  1: ALL Linux Uboot ext3
sfdisk $HDD -uM << EOF
;
EOF
echo "Начинаю форматирование..."
#echo "Форматирую загрузочный раздел"
mkfs.ext3 -I 128 -b 4096 -L RUNIGMA $HDD"1"
echo "Готово"
sleep 1
echo "FSCK"
fsck.ext2 -y /dev/sda1
sleep 1
echo "Монтирую раздел /dev/sda1"
mount /dev/sda1 /rootfs
echo "Установка" > /dev/vfd
echo "Копирую системные файлы в системный раздел /dev/sda1..."
cp /install/*.tar.gz /rootfs
cd /rootfs
echo "Удаляю образ из Оперативной памяти..."
rm /install/*.tar.gz
echo "Устанавливаю системный раздел ..."
tar -xf *.tar.gz
echo "Удаляю стартовый системный образ"
rm /rootfs/*.tar.gz
echo "Готово"
if [ -e /install/backup/E2Settings.tar.gz ]; then
	echo "Копирую настройки в системный раздел /dev/sda1..."
	cp /install/backup/E2Settings.tar.gz /rootfs
	echo "Удаляю настройки из Оперативной памяти..."
	rm /install/backup/E2Settings.tar.gz
	echo "Востановление настройки"
	tar -xf E2Settings.tar.gz
	echo "Удаляю настройки в архиве"
	rm E2Settings.tar.gz
fi
if [ -e /install/backup/backup.tar.gz ]; then
	echo "Копирую настройки в системный раздел /dev/sda1..."
	cp /install/backup/backup.tar.gz /rootfs
	echo "Удаляю настройки из Оперативной памяти..."
	rm /install/backup/backup.tar.gz
	echo "Востановление настройки"
	tar -xf backup.tar.gz
	echo "Удаляю настройки в архиве"
	rm backup.tar.gz
fi
cd /
echo "Загрузка" > /dev/vfd
echo "Восстанавливаю загрузчик"
cp /install/uImage /rootfs/boot
rm /install/uImage
sleep 1
echo "done"
echo "######################################################"
echo ""
echo "   Всё готово!!! Сейчас начнётся загрузка сборки...   "
echo ""
echo "######################################################"
echo "Готово" > /dev/vfd
