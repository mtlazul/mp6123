#!/bin/bash

if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root" 
   exit 1
fi

# Core and tools
apt-get install \
	libgstreamer1.0-0 \
	libgstreamer-plugins-base1.0-0 \
	libgstreamer-plugins-good1.0-0 \
	gstreamer1.0-tools 

# Plugins
apt-get install \
	gstreamer1.0-plugins-base \
	gstreamer1.0-plugins-good \
	gstreamer1.0-plugins-bad \
	gstreamer1.0-plugins-ugly \
	gstreamer1.0-libav

# Development libraries
apt-get install \
	libgstreamer1.0-dev \
	libgstreamer-plugins-base1.0-dev \
	libgstreamer-plugins-good1.0-dev \
	libgstreamer-plugins-bad1.0-dev

echo "Done! Thanks for flying with us :)"