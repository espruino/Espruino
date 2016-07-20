#!/bin/bash
#
# This script is run by Vagrant when a new machine is provisioned.
#
 
sudo apt-get -y install git
# add support for 64 bit (which vagrant is set to configure)
sudo apt-get -y install lib32z1 lib32ncurses5 lib32bz2-1.0
sudo apt-get -y install lib32ncurses5
sudo apt-get -y install ia32-libs
 
# download known working version of arm-none-eabi-gcc
sudo mkdir ~/gcc-temp
cd ~/gcc-temp
sudo wget https://launchpad.net/gcc-arm-embedded/4.8/4.8-2014-q3-update/+download/gcc-arm-none-eabi-4_8-2014q3-20140805-linux.tar.bz2
cd /usr/local
sudo tar xjf ~/gcc-temp/gcc-arm-none-eabi-4_8-2014q3-20140805-linux.tar.bz2
 
# apply setup config to add compiler to PATH
# from stackoverflow i.e http://stackoverflow.com/a/28279205
echo "source /vagrant/scripts/vagrant_setup.sh" >> /home/vagrant/.bashrc


# for reference, this is what _should_ work
#sudo add-apt-repository ppa:terry.guo/gcc-arm-embedded #<- sadly wrong version
#sudo apt-get update
#sudo apt-get install -y gcc-arm-none-eabi git