#!/bin/bash
#
# This script is run by Vagrant when a new machine is provisioned.
# It can also be used on an Ubuntu 
# Contains pre-requisits for native Espruino and cross compilation for puck.js 
#

if [ -d "/home/ubuntu" ]; then # probably a vagrant setup
    THISUSER="ubuntu"
    THISHOME="/home/ubuntu"
else
    THISUSER=${USER}
    THISHOME="~/"
fi
# A method of keeping order in source, repos, repos providers and projects
THISESPRUINO="${THISHOME}/source/repos/github/espruino"

sudo add-apt-repository ppa:team-gcc-arm-embedded/ppa
sudo apt-get update
sudo apt-get install -y \
  build-essential git python python-pip
# User choice for placement of source repos
mkdir -p ${THISESPRUINO}
cd ${THISESPRUINO}
git clone https://github.com/espruino/Espruino.git
cd Espruino
make clean && make # Create a version of Espruino that runs on your machine 
# vagrant provisioning clean up or else root is owner
chown -R ${THISUSER} ${THISESPRUINO}
# echo Path
