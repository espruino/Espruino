# Usage:
# 1: Buld the container image 
# docker build . -t img_name
# 2: Run container image so it builds espruino
# docker run img_name --name container_name
# this will run the container and save build results into the container's filesystem
# 3: Copy build results from the container into your filesystem
# docker cp container_name:espruino/espruino_1v99.4185_stm32f100lqfp48.elf ./

FROM python:3

COPY . /espruino
WORKDIR /espruino

# Change these to provision and compile for a different board
RUN /bin/bash -c "source scripts/provision.sh STM32F100CB"
ENV BOARD STM32F100CB

ENV RELEASE 1

CMD ["/bin/bash", "-c", "source scripts/provision.sh STM32F100CB && make"]
