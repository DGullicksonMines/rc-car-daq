#! /usr/bin/env bash

# Copyright 2024 by Dawson J. Gullickson

AUTOPI_TARBALL="autopi-install.tgz"
AUTOPI_INSTALL="autopi-install/install"

DEVICE_ID=$(cat "autopi-deviceid.txt")
DEVICE_ID_FILE="/boot/CSM_device_id.txt"

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
	sudo apt install git gcc i2c-tools pip
	sudo apt clean

	echo -n 2 > "$CACHE"
fi

if (( $PROGRESS < 3 )); then
	# Configure I2C
	#NOTE 0 corresponds to *enabling* these interfaces
	sudo raspi-config nonint do_spi 0
	sudo raspi-config nonint do_i2c 0

	echo -n 3 > "$CACHE"
fi

if (( $PROGRESS < 4 )); then
	# Install Autopi
	if [[ ! -f "$AUTOPI_TARBALL" ]]; then
		echo
		exit 1
	fi
	tar -vxz -f "$AUTOPI_TARBALL"
	sudo "$AUTOPI_INSTALL"
	echo "$DEVICE_ID" > "$DEVICE_ID_FILE"

	echo -n 4 > "$CACHE"

	sudo reboot
fi