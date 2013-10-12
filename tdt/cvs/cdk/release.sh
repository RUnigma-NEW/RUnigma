#!/bin/bash

cd ../../tufsbox/
mkdir -p release_install release_install/boot
mkdir -p release_install release_install/service
touch release_install/service/install
cp -fr release/boot/uImage release_install/boot/uImage.gz
cd release_install/boot/
ln -sf uImage.gz uImage
cd - > /dev/null 2>&1
cd release
mv -f boot/uImage ../../tufsbox/
tar -czvf ../../tufsbox/release_install/service/release.tar.gz *
cd - > /dev/null 2>&1
mv -f uImage release/boot/
cd release_install
chmod 755 *
cd - > /dev/null 2>&1
