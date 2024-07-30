#!/bin/sh

iptables-restore < /home/ctf/rules.txt
su -c "./ynetd -p 1337 'python3 challenge.py'" ctf
