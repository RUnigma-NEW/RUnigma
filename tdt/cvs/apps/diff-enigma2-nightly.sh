diff -Nur --exclude=".git" --exclude="tuxtxt"  enigma2-nightly.org enigma2-nightly.patched > enigma2-nightly.newest.diff
sed -i "s/enigma2-nightly.newest.patched/enigma2-nightly/g" enigma2-nightly.newest.diff
sed -i "s/enigma2-nightly.newest/enigma2-nightly.org/g" enigma2-nightly.newest.diff
ls -lh enigma2-nightly.newest.diff
