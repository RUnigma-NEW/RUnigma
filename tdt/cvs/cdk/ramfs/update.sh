echo "Обновляю" > /dev/vfd
# Kopiert das Update File auf die /dev/sda1 Partition um es Später zu installieren
rm /rootfs/service/update
# Entpackt das Update und löscht es anschliessend
cd /rootfs
tar -xf service/*.tar.gz
echo "Готово"
echo "Удаляю tar.gz из системного раздела"
rm /rootfs/service/*.tar.gz
echo "Обновление завершено..."
cd /
echo "Запускаю систему..."
echo "Готово" > /dev/vfd
