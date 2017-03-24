FROM ubuntu:14.04

RUN git clone https://github.com/espruino/Espruino espruino
WORKDIR /espruino

# Change these to provision and compile for a different board
RUN source scripts/provision.sh PICO_R1_3
ENV BOARD PICO_R1_3

ENV RELEASE 1

CMD ["make"]
