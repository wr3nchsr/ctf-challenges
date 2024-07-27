#!/bin/sh

FS=/home/ctf/jffs2-root

echo "cyctf{one_null_byte_can_make_a_difference}" > ${FS}/home/httpd/flag.txt
echo "cyctf{reversing_logic_through_libraries_ftw}" > ${FS}/home/clid/flag.txt

chmod -R 755 ${FS}/bin
chmod -R 755 ${FS}/dev
chmod -R 644 ${FS}/etc
chmod 640 ${FS}/etc/shadow
chmod -R 750 ${FS}/home
chmod 555 ${FS}/home
chmod -R 755 ${FS}/lib
chmod -R 777 ${FS}/mnt
chmod -R 555 ${FS}/proc
chmod -R 700 ${FS}/root
chmod -R 755 ${FS}/sbin
chmod -R 555 ${FS}/sys
chmod -R 777 ${FS}/tmp
chmod -R 755 ${FS}/usr
chmod -R 755 ${FS}/var

chown -R root:root ${FS}
chown -R root:1001 ${FS}/home/httpd
chown -R root:1002 ${FS}/home/clid

apt -y install libcap2-bin
setcap cap_net_raw+eip ${FS}/bin/busybox
rm setup.sh