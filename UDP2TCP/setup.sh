#!/bin/sh -e

sudo chmod +x /home/pi/UDP2TCP/UDP2TCP03

sudo cp -rf /etc/rc.local /etc/rc_copy.local
sudo cp -rf /home/pi/UDP2TCP/rc.local /etc/

sudo cp -rf /etc/dhcpcd.conf /etc/dhcpcd_copy.conf
sudo cp -rf /home/pi/UDP2TCP/dhcpcd.conf /etc/

sudo reboot
