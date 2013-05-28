echo "UPDATE..." > /dev/vfd
#mountet die ziel Partition nach /update, dadurch können die Updates größer 95MB sein
mount /dev/sda2 /update
# Kopiert das Update File auf die /dev/sda2 Partition um es Später zu installieren
echo "Copy Update auf Ziel Partition"
echo "Copy..." > /dev/vfd
cp /rootfs/*.tar.gz /update
echo "done"
echo "lösche Update vom SDA1"
# Löscht das Update File von der Partition
rm /rootfs/*.tar.gz
rm /rootfs/update
echo "Install Update"
echo "Install..."
# Entpackt das Update und löscht es anschliessend
cd /update
tar -xf *.tar.gz
echo "done"
echo "Lösche tar.gz von SDA2"
rm /update/*.tar.gz
echo "Update Komplett"
cd /
echo "Starte System"
umount /dev/sda2
echo "Ready..." > /dev/vfd

