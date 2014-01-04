echo "Копирую образ в Оперативную память"
echo "Копирую" > /dev/vfd
cd /rootfs
cp service/*.tar.gz /install
echo "Готово"
if [ -e /rootfs/backup/backup.tar.gz ]; then
	echo "Сохраняю" > /dev/vfd
	cp /backup/backup.tar.gz /install/backup
	cd /
fi
echo "Запускаю загрузчик..."
cd /rootfs/boot
cp uImage /install
echo "Демонтирую загрузочный раздел /dev/sda1"
cd /
umount /dev/sda1
echo "Готово"
echo "Формат" > /dev/vfd
echo "Готовлю структуру разбиения диска"
HDD=/dev/sda
ROOTFS=$HDD"1"
# Löscht die Festplatte/Stick und erstellt 4 Partitionen
#  1: ALL Linux Uboot ext3
sfdisk $HDD -uM << EOF
,,L
;
EOF
echo "Готово"
echo "Формат" > /dev/vfd
echo "Начинаю форматирование..."
echo "Форматирую загрузочный раздел"
mkfs.ext3 -I 128 -b 4096 -L BOOTFS $HDD"1"
echo "Готово"
echo "Монтирую раздел /dev/sda1"
mount /dev/sda1 /rootfs
while [ -e `mount | grep -i "dev" | awk '{ print $2 }'` ]
do
	umount /dev/sda1
	sleep 1
	echo "ошибка монтирования"
	mount /dev/sda1 /rootfs
done
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
if [ -e /install/backup/backup.tar.gz ]; then
	cp /install/backup/keys.tar.gz /rootfs/backup
	rm /install/backup/*
	cd /rootfs
	tar -xf /backup/backup.tar.gz
	cd ../..
fi
cd /
echo "Загрузка" > /dev/vfd
echo "Восстанавливаю загрузчик"
cp /install/uImage /rootfs/boot
rm /install/uImage
cd /rootfs/boot
cd ../..
sleep 2
echo "######################################################"
echo ""
echo "   Всё готово!!! Сейчас начнётся загрузка сборки...   "
echo ""
echo "######################################################"
echo "Готово" > /dev/vfd
