#!/bin/bash
#
# This script is run by Vagrant when a new machine is provisioned.
#

sudo apt-get -y install git

# move to 'Espruino' directory
cd `dirname $0`/..

# Attempt to provision for all required platforms
source scripts/provision.sh ESP8266
source scripts/provision.sh ESP32
source scripts/provision.sh PUCKJS

# apply setup config to add compiler to PATH
# from stackoverflow i.e http://stackoverflow.com/a/28279205
echo "source /vagrant/scripts/vagrant_setup.sh" >> /home/vagrant/.bashrc
