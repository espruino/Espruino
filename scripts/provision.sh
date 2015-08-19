#!/bin/bash
#
# This script is run by Vagrant when a new machine is provisioned.
#

sudo add-apt-repository ppa:terry.guo/gcc-arm-embedded
sudo apt-get update
sudo apt-get install -y gcc-arm-none-eabi git

