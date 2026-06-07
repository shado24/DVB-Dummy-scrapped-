#!/bin/bash
sudo apt install gnome-dvb-client tvheadend build-essentials make
make
xz dvb_dummy.ko
sudo modprobe mc
sudo modprobe dvb_core
sudo cat << EOF > /etc/modules
# /etc/modules: kernel modules to load at boot time.
#
# This file contains the names of kernel modules that should be loaded
# at boot time, one per line. Lines beginning with "#" are ignored.
# Parameters can be specified after the module name.

i2c-dev
mc
dvb-core
dvb_dummy
EOF
sudo mkdir /lib/modules/$(uname -r)/kernel/drivers/media/dvb_dummy
sudo cp dvb_dummy.ko.xz /lib/modules/$(uname -r)/kernel/drivers/media/dvb_dummy
sudo reboot now
