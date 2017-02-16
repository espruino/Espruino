#!/bin/bash
#
# This script is run by Vagrant when a new machine is provisioned.
#
 
sudo apt-get -y install git
# add support for 64 bit (which vagrant is set to configure)
sudo apt-get -y install lib32z1 lib32ncurses5 lib32bz2-1.0 lib32ncurses5 ia32-libs

sudo add-apt-repository ppa:team-gcc-arm-embedded/ppa
sudo apt-get update
sudo apt-get install gcc-arm-embedded
 
# apply setup config to add compiler to PATH
# from stackoverflow i.e http://stackoverflow.com/a/28279205
echo "source /vagrant/scripts/vagrant_setup.sh" >> /home/vagrant/.bashrc

