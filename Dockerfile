FROM ubuntu:22.04

ENV TZ=Europe \
	DEBIAN_FRONTEND=noninteractive \
	DISPLAY=:0.0

RUN apt-get update -y && apt-get upgrade -y

# Setting up build environment
RUN apt-get install -y \
	build-essential \
	wget

RUN apt-get install -y libgstreamer1.0-dev \
	libgstreamer-plugins-base1.0-dev \
	libgstreamer-plugins-bad1.0-dev \
	gstreamer1.0-plugins-base \
	gstreamer1.0-plugins-good \
	gstreamer1.0-plugins-bad \
	gstreamer1.0-plugins-ugly \
	gstreamer1.0-libav \
	gstreamer1.0-tools \
	gstreamer1.0-x \
	gstreamer1.0-alsa \
	gstreamer1.0-gl \
	gstreamer1.0-gtk3 \
	gstreamer1.0-qt5 \
	gstreamer1.0-pulseaudio

RUN apt-get install -y cmake git curl

WORKDIR /
RUN git clone https://github.com/ninja-build/ninja.git
WORKDIR /ninja
RUN mkdir build && \
	cmake -B build -S . && \
	cmake --build build/ --target install


RUN apt-get install -y meson pkgconf vim sudo gdb python3-dev python3-pip

RUN pip3 install numpy cython 

# Create app directory

# RUN mkdir -p /app -m 777
#
# RUN groupadd -f admin
#
# RUN useradd -rm -d /home/newuser -s /bin/bash -g admin -G sudo -G audio -u 1000 newuser
# RUN echo "newuser:pass" | chpasswd
# USER newuser
WORKDIR /dynrt
COPY . .
