#!/bin/bash

ROOT=$(realpath "$0" | sed 's|\(.*\)/.*|\1|')
BUILD=${ROOT}/build
PLAYERS=${BUILD}/players
CHALLENGE=${BUILD}/challenge
FS=${PLAYERS}/fs
CFS=${CHALLENGE}/fs

echo "[*] Removing existing fs"
sudo rm -rf ${FS}
sudo rm -rf ${CFS}

echo "[*] Build fs directory structure"
mkdir -p ${FS}
mkdir -p ${CFS}

mkdir ${FS}/bin
mkdir ${FS}/dev
mkdir ${FS}/etc
mkdir ${FS}/home
mkdir ${FS}/home/httpd
mkdir ${FS}/home/clid
mkdir ${FS}/lib
mkdir ${FS}/mnt
mkdir ${FS}/mnt/jail
mkdir ${FS}/proc
mkdir ${FS}/root
mkdir ${FS}/sbin
mkdir ${FS}/sys
mkdir ${FS}/tmp
mkdir ${FS}/usr
mkdir ${FS}/usr/bin
mkdir ${FS}/usr/sbin
mkdir ${FS}/var

# add base libs
echo "[*] Copying libraries"
cp -RP /usr/mips-linux-gnu/lib/* ${FS}/lib
rm ${FS}/lib/*.a
rm ${FS}/lib/*.o

# add busybox and symlinks
echo "[*] Copying binaries and symlinks"
cp ${BUILD}/busybox ${FS}/bin/
SYMLINKS=${BUILD}/busybox_symlinks

cd ${FS}/bin
for i in $(sed '2!d' ${SYMLINKS}); do
    ln -s ./busybox $i
done

cd ${FS}/sbin
for i in $(sed '4!d' ${SYMLINKS}); do
    ln -s ../bin/busybox $i
done

cd ${FS}/usr/bin
for i in $(sed '6!d' ${SYMLINKS}); do
    ln -s ../../bin/busybox $i
done

cd ${FS}/usr/sbin
for i in $(sed '8!d' ${SYMLINKS}); do
    ln -s ../../bin/busybox $i
done

# create teasing jail
echo "[*] Creating teasing jail"
touch ${FS}/mnt/jail/0_YOU
touch ${FS}/mnt/jail/1_CANT
touch ${FS}/mnt/jail/2_ESCAPE
touch ${FS}/mnt/jail/3_ME

# copy init service files
echo "[*] Compiling and copying init"
cd ${ROOT}/src/init/
make clean
make
cp ${ROOT}/src/init/init.out ${FS}/bin/init

# copy HTTPD service files
echo "[*] Compiling and copying httpd"
cd ${ROOT}/src/httpd/
make clean
make
cp ${ROOT}/src/httpd/httpd.out ${FS}/bin/httpd
cp ${ROOT}/src/httpd/_httpd/*  ${FS}/home/httpd

# copy CLID service files
echo "[*] Compiling and copying clid"
cd ${ROOT}/src/clid/
make clean
make
cp ${ROOT}/src/clid/clid.out ${FS}/bin/clid
cp ${ROOT}/src/clid/libcli_functions.so ${FS}/lib
cp ${ROOT}/src/clid/libcli_helpers.so ${FS}/lib

# write flags
echo "[*] Creating flags"
echo "cyctf{0bv10u5ly_4_FAKE_f14g}" >  ${FS}/home/httpd/flag.txt
echo "cyctf{0bv10u5ly_4_FAKE_f14g}" >  ${FS}/home/clid/flag.txt
echo "cyctf{ecc_c4n_b3_tr0ubl3s0m3_f0r_f1rmw4r3_dump1ng}" >  ${FS}/etc/flag.txt

# avoid leakage with strings or binwalk
gzip ${FS}/etc/flag.txt
mv ${FS}/etc/flag.txt.gz ${FS}/etc/low_budget_anti_strings.txt
gzip ${FS}/etc/low_budget_anti_strings.txt
mv ${FS}/etc/low_budget_anti_strings.txt.gz ${FS}/etc/read_me_fast_for_points.txt
gzip ${FS}/etc/read_me_fast_for_points.txt


echo "[*] Creating /etc files"
# passwd
echo "root:x:0:0:root:/root:/bin/sh
ctf:x:1000:1000::/home/ctf/:/bin/sh
httpd:x:1001:1001::/home/httpd:/bin/sh
clid:x:1002:1002::/home/clid:/bin/sh" > ${FS}/etc/passwd

# groups
echo "root:x:0:
ctf:x:1000:
httpd:x:1001:
clid:x:1002:" > ${FS}/etc/group

# shadow
echo "root:*:19634:0:99999:7:::
ctf:!:19669:0:99999:7:::
httpd:!:19669:0:99999:7:::
clid:!:19669:0:99999:7:::" > ${FS}/etc/shadow

# hostname
echo "LBRouter" > ${FS}/etc/hostname

# PERMISSIONS
echo "[*] Configuring fs permissions and ownership"
sudo chmod -R 755 ${FS}/bin
sudo chmod -R 755 ${FS}/dev
sudo chmod -R 644 ${FS}/etc
sudo chmod 640 ${FS}/etc/shadow
sudo chmod -R 750 ${FS}/home
sudo chmod 555 ${FS}/home
sudo chmod -R 755 ${FS}/lib
sudo chmod -R 777 ${FS}/mnt
sudo chmod -R 555 ${FS}/proc
sudo chmod -R 700 ${FS}/root
sudo chmod -R 755 ${FS}/sbin
sudo chmod -R 555 ${FS}/sys
sudo chmod -R 777 ${FS}/tmp
sudo chmod -R 755 ${FS}/usr
sudo chmod -R 755 ${FS}/var

sudo chown -R root:root ${FS}
sudo chown -R root:1001 ${FS}/home/httpd
sudo chown -R root:1002 ${FS}/home/clid

# enable ping
sudo setcap cap_net_raw+eip ${FS}/bin/busybox

echo "[*] Replicating player fs for challenge"
sudo cp -rf ${FS}/* ${CFS}/

echo "[*] Building jffs2 file"
sudo mkfs.jffs2 -d ${FS}/. -o ${PLAYERS}/extra/fs.jffs2
sudo mkfs.jffs2 -d ${CFS}/. -o ${CHALLENGE}/fs.jffs2

echo "[*] Creating fake firmware bins"
cd ${PLAYERS}
python3 create_fw.py

cd ${CHALLENGE}
echo "[*] Building docker image"
sudo docker build -t low_budget_router .
echo "[*] Running docker image"
sudo docker run -it -p10080:10080 -p10023:10023 --rm low_budget_router
