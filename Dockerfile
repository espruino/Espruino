# Usage:
#
# Note: if you're in Linux you need to run socker with 'sudo'.
#   But honestly if you're on linux you should just save yourself
#   gigabytes of downloads and disk space and build Espruino directly.
#
# 1: Build the container image 
#
#   docker build . -t img_name
#
# 2: Run container image so it builds espruino
#
#   docker run -e BOARD='PICO_R1_3' --name container_name img_name
#
# This will run the container and save build results into the container's filesystem.
# Near the end of the build the filename will be displayed, for example espruino_2v00_pico_1r3.bin 
#
# 3: Copy build results from the container into your filesystem
#
#   docker cp container_name:espruino/espruino_2v00_pico_1r3.bin ./
#

FROM python:3

COPY ./scripts/provision.sh provision.sh
COPY ./targetlibs /targetlibs

# Workaround add some stuff that the provision script uses
# in here so it doesn't have to use sudo
RUN apt-get update
RUN apt-get install -qq -y python3-pip
RUN pip install pyserial
RUN pip install nrfutil

# This ensures ALL dependencies are installed beforehand
RUN bash -c "source provision.sh ALL"

COPY . /espruino
RUN bash -c "mv targetlibs espruino/targetlibs"
WORKDIR /espruino

ENV RELEASE 1
CMD "make"
