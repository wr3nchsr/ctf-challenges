FROM ubuntu:22.04

RUN apt update && apt -y upgrade
RUN apt -y install qemu-user-static sudo python3 python3-pip
RUN pip3 install jefferson

RUN echo "ctf ALL=(root) NOPASSWD: /usr/sbin/chroot" >> /etc/sudoers
RUN useradd -d /home/ctf/ -s /bin/bash ctf

WORKDIR /home/ctf/

COPY fs.jffs2 .
RUN jefferson fs.jffs2
RUN cp /usr/bin/qemu-mips-static jffs2-root/bin

# add flags, apply permissions and enable ping
COPY setup.sh .
RUN bash setup.sh

WORKDIR /home/ctf/jffs2-root
USER ctf
EXPOSE 10080
EXPOSE 10023
CMD ["sudo", "chroot", ".", "/bin/init"]