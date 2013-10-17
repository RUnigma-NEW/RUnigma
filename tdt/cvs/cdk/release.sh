#!/bin/bash
cd ../../tufsbox/
mkdir -p release_install release_install/boot
mkdir -p release_install release_install/service
touch release_install/service/install
cd release
mv -f boot/uImage ../../tufsbox/release_install/boot/uImage
tar -czvf ../../tufsbox/release_install/service/release.tar.gz *
cd - > /dev/null 2>&1
cp -fr release_install/boot/uImage release/boot/uImage
cd release_install
chmod 755 *
cd - > /dev/null 2>&1
