# Usage:
#
# Note: if you're in Linux you need to run docker with 'sudo' (unless
#   your user is added to docker group).
#
# 1: Specify board in ENV in docker-compose.yml file
#
# 2: docker-compose up --build
#
# Note: This will run the container and save build results into mounted build directory.

FROM python:2

# Workaround add some stuff that the provision script uses
# in here so it doesn't have to use sudo
RUN apt-get update
RUN apt-get install -qq -y python3-pip
RUN pip install pyserial
RUN pip install nrfutil

COPY . /espruino
WORKDIR /espruino

# This ensures ALL dependencies are installed beforehand
RUN bash -c "source scripts/provision.sh ALL"

ENV BOARD=ALL
CMD ["bash", "-c", "source scripts/provision.sh ${BOARD} && make"]

