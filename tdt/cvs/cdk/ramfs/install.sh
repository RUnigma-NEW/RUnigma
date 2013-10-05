echo "Копирую образ в Оперативную память"
echo "Копирую" > /dev/vfd
cd /rootfs
cp *.tar.gz /install
echo "Готово"
echo "Сохраняю" > /dev/vfd
if [ -e /rootfs/etc/enigma2 ]; then
	echo "Проверяю, есть ли пользовательские настройки Энигмы..."
	echo "Сохраняю" > /dev/vfd
	cd /rootfs/etc/enigma2
	rm settings
	tar -czvf /install/backup/E2Settings.tar.gz ./ > /dev/null 2>&1
	cd /
else
	echo "Настройки не найдены"
	echo "Проверяю раздел SDA2"
	cd /
	umount /dev/sda1
	mount /dev/sda2 /rootfs
	if [ -e /rootfs/etc/enigma2 ]; then
		echo "Проверяю, есть ли пользовательские настройки Энигмы..."
		cd /rootfs/etc/enigma2
		rm settings
		tar -czvf /install/backup/E2Settings.tar.gz ./ > /dev/null 2>&1
		cd /
		umount /dev/sda2
		mount /dev/sda1 /rootfs
	else
		echo "Настройки в разделе SDA2 не найдены"
		cd /
		umount /dev/sda2
		mount /dev/sda1 /rootfs
	fi
fi
if [ -e /rootfs/var/keys ]; then
	cd /rootfs/var/keys
	tar -czvf /install/backup/keys.tar.gz ./ > /dev/null 2>&1
	echo "Готово"
	cd /
else
	echo "Ключи не найдены"
	echo "Проверяю раздел  SDA2"
	cd /
	umount /dev/sda1
	mount /dev/sda2 /rootfs
	if [ -e /rootfs/var/keys ]; then
		cd /rootfs/var/keys
		tar -czvf /install/backup/keys.tar.gz ./ > /dev/null 2>&1
		echo "Готово"
		cd /
		umount /dev/sda2
		mount /dev/sda1 /rootfs
	else
		echo "Настройки в разделе SDA2 не найдены"
		cd /
		umount /dev/sda2
		mount /dev/sda1 /rootfs
	fi
fi
echo "Запускаю загрузчик..."
cd /rootfs/boot
cp uImage.gz /install
echo "Демонтирую загрузочный раздел /dev/sda1"
cd /
umount /dev/sda1
echo "Готово"
echo "Формат" > /dev/vfd
echo "Готовлю структуру разбиения диска"
HDD=/dev/sda
ROOTFS=$HDD"1"
SYSFS=$HDD"2"
DATAFS=$HDD"3"
sfdisk --re-read $HDD
# Löscht die Festplatte/Stick und erstellt 4 Partitionen
#  1: 256MB Linux Uboot ext3
#  2:   1GB Linux System ext4
#  3: rest freier Speicher LINUX ext4 (bei HDD record)
sfdisk $HDD -uM << EOF
,256,L
,1024,L
,,L
;
EOF
echo "Готово"
echo "Формат" > /dev/vfd
echo "Начинаю форматирование..."
echo "Форматирую загрузочный раздел"
mkfs.ext3 -I 128 -b 4096 -L BOOTFS $HDD"1"
echo "Форматирую системный раздел"
mkfs.ext4 -L ROOTFS $HDD"2"
echo "Форматирую оставшееся свободное место"
mkfs.ext4 -L RECORD $HDD"3"
echo "Готово"
echo "Монтирую раздел /dev/sda2"
mount /dev/sda2 /rootfs
echo "Загрузка" > /dev/vfd
echo "Копирую системные файлы в системный раздел /dev/sda2..."
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
echo "Демонтирую системный раздел /dev/sda2 и монтирую загрузочный раздел /dev/sda1"
umount /dev/sda2
sleep 1
mount /dev/sda1 /rootfs
echo "Загрузка" > /dev/vfd
echo "Восстанавливаю загрузчик"
mkdir /rootfs/boot
cp /install/uImage.gz /rootfs/boot
rm /install/uImage.gz
cd /rootfs/boot
ln -s uImage.gz uImage
cd ../..
sleep 2
umount /dev/sda1
sleep 1
echo "Проверка" > /dev/vfd
echo "Запускаю проверку дисков..."
fsck.ext3  -f -y /dev/sda1
sleep 1
fsck.ext4  -f -y /dev/sda2
sleep 1
fsck.ext4  -f -y /dev/sda3
sleep 1
echo "######################################################"
echo ""
echo "   Всё готово!!! Сейчас начнётся загрузка сборки...   "
echo ""
echo "######################################################"
sleep 1
mount /dev/sda1 /rootfs
echo "Готово" > /dev/vfd
