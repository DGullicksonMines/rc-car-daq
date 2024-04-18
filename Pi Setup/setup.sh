#! /usr/bin/env bash

# Copyright 2024 by Dawson J. Gullickson

set -e # Exit on errors

# Read cache
CACHE="setup.cache"
if [[ -f CACHE ]]; then
	PROGRESS=0
else
	PROGRESS=$(cat "$CACHE")
fi

if (( $PROGRESS < 1 )); then
	# Update packages
	sudo apt update
	sudo apt full-upgrade
	sudo apt clean

	echo -n 1 > "$CACHE"
fi

if (( $PROGRESS < 2 )); then
	# Install necessary packages
	sudo apt install git gcc i2c-tools gpsd libgps-dev
	sudo apt clean

	echo -n 2 > "$CACHE"
fi

if (( $PROGRESS < 3 )); then
	#NOTE 0 corresponds to *enabling* these interfaces
	# Enable I2C
	sudo raspi-config nonint do_spi 0
	sudo raspi-config nonint do_i2c 0
	# Enable SSH
	sudo raspi-config nonint do_ssh 0

	echo -n 3 > "$CACHE"

	sudo reboot
fi
