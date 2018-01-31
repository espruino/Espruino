#!/bin/bash
#
# This script is run by Vagrant when a new machine is provisioned.
#

# To mute 'dpkg-preconfigure: unable to re-open stdin: No such file or directory'
# warning the DEBIAN_FRONTEND=noninteractive needs to be set for `apt-get install`

sudo DEBIAN_FRONTEND=noninteractive apt-get -qq -y install git

# move to 'Espruino' directory
cd `dirname $0`/..

# Attempt to provision for all required platforms
source scripts/provision.sh ESP8266_BOARD
source scripts/provision.sh ESP32
source scripts/provision.sh PUCKJS

# apply setup config to add compiler to PATH
# from stackoverflow i.e http://stackoverflow.com/a/28279205
echo "source /vagrant/scripts/vagrant_setup.sh" >> /home/vagrant/.bashrc
