echo "Обновляю" > /dev/vfd
cd /rootfs
tar -xf service/*.tar.gz
echo "Готово"
echo "Удаляю tar.gz из системного раздела"
rm -rf /rootfs/service
echo "Обновление завершено..."
cd /
echo "Запускаю систему..."
echo "Готово" > /dev/vfd
