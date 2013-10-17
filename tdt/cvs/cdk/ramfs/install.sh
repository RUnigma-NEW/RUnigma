echo "Копирую образ в Оперативную память"
echo "Копирую" > /dev/vfd
cd /rootfs
cp service/*.tar.gz /install
echo "Готово"
echo "Сохраняю" > /dev/vfd
if [ -e /rootfs/etc/enigma2 ]; then
	echo "Проверяю, есть ли пользовательские настройки Энигмы..."
	echo "Сохраняю" > /dev/vfd
	cd /rootfs/etc/enigma2
	rm settings
	tar -czvf /install/backup/E2Settings.tar.gz ./ > /dev/null 2>&1
	cd /
fi
if [ -e /rootfs/var/keys ]; then
	cd /rootfs/var/keys
	tar -czvf /install/backup/keys.tar.gz ./ > /dev/null 2>&1
	echo "Готово"
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
sfdisk --re-read $HDD
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
echo "Загрузка" > /dev/vfd
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
echo "Восстанавливаю настройки"
if [ -e /install/backup/E2Settings.tar.gz ]; then
	cp /install/backup/E2Settings.tar.gz /rootfs/etc/enigma2
	cd /rootfs/etc/enigma2
	tar -xf E2Settings.tar.gz
	cd ../../..
else
	echo "Нет данных для сохранения или восстановления"
fi
if [ -e /install/backup/keys.tar.gz ]; then
	cp /install/backup/keys.tar.gz /rootfs/var/keys
	cd /rootfs/var/keys
	tar -xf keys.tar.gz
	cd ../../..
else
	echo "Нет ключей для сохранения или восстановления"
fi
cd /
echo "Загрузка" > /dev/vfd
echo "Восстанавливаю загрузчик"
cp /install/uImage /rootfs/boot
rm /install/uImage
cd /rootfs/boot
cd ../..
sleep 2
umount /dev/sda1
sleep 2
echo "Проверка" > /dev/vfd
echo "Запускаю проверку дисков..."
fsck.ext3 -y /dev/sda1
sleep 1
echo "######################################################"
echo ""
echo "   Всё готово!!! Сейчас начнётся загрузка сборки...   "
echo ""
echo "######################################################"
sleep 1
mount /dev/sda1 /rootfs
echo "Готово" > /dev/vfd
