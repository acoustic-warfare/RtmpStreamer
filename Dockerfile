FROM ubuntu:22.04 AS deps

ENV TZ=Europe \
	DEBIAN_FRONTEND=noninteractive \
	DISPLAY=:0.0

RUN apt-get update && apt-get install -y \
	git \
	python3-dev \
	python3-pip \
	g++ \
	vim \
	libzmq3-dev \
	libusb-1.0-0 \
	libgstreamer1.0-dev \
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
	gstreamer1.0-pulseaudio \
	libfmt-dev && \
	rm -rf /var/lib/apt/lists/*

RUN python3 -m pip install cython numpy meson ninja

FROM deps AS streamer
RUN git clone https://github.com/acoustic-warfare/RtmpStreamer && \
	cd RtmpStreamer && \
	meson setup build -Dpython-bindings=true --native-file native-file.ini && \
	ninja -C build && \
	ninja -C build install



