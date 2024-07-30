FROM ubuntu:22.04

RUN apt update && apt upgrade -y
RUN apt -y install python3 python3-pip iptables net-tools 

RUN useradd -d /home/ctf/ -s /bin/bash ctf

WORKDIR /home/ctf/
COPY files/ynetd .
COPY files/challenge.py  .
COPY files/rules.txt  .
COPY files/run.sh  .
COPY files/flag.txt /
RUN chmod 111 ./ynetd
RUN chmod 644 /flag.txt
RUN chown root:root /flag.txt
RUN chown -R root:root .

# USER ctf
EXPOSE 1337
CMD ["sh", "./run.sh"]
