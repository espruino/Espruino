FROM ubuntu:14.04

RUN apt-get update && apt-get install -y software-properties-common && apt-get update \
 && add-apt-repository ppa:team-gcc-arm-embedded/ppa \
 && apt-get update \ 
 && apt-get install -y git gcc-arm-embedded build-essential python

RUN git clone https://github.com/espruino/Espruino espruino
WORKDIR /espruino

# If compiling for a non-linux target that has internet support, use WIZnet support, not TI CC3000
#ENV WIZNET=1
# If compiling for a non-linux target that has internet support, use ESP8266 support, not TI CC3000
#ENV ESP8266 1

ENV RELEASE 1
#ENV PICO_1V3 1

CMD ["make"]
