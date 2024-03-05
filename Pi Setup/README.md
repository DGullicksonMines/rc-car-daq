# Raspberry Pi Setup

## Install the OS

1. Download **Raspberry Pi Imager** from https://www.raspberrypi.com/software/

2. Open **Raspberry Pi Imager**.

3. Select the "Raspberry Pi Device"

4. Select the "Operating System":  
Raspberry Pi OS (other) > Raspberry Pi OS Lite (64-bit)

5. Select the "Storage"

6. //TODO setup instructions for creating user

## Configure the WiFi connection

1. Configure the WiFi connection:
	- Run `sudo raspi-config`
	- Navigate to System Options > Wireless LAN
	- Enter the WiFi connection's SSID and password.  
	On Mines Campus, use `Mines-IoT`

2. To connect to the Mines network, register the
**Raspberry Pi**'s **MAC address** at https://netreg.mines.edu/
	- The **MAC address** may be found by running `ethtool -P eth0`

## Finish setup

Run `sudo setup.sh` to complete setup.

# `setup.sh`

Broadly, `setup.sh` performs **4** setup actions:
1. Updates base packages
2. Installs required packages
	- `git` (for the repo)
	- `gcc` (for compiling)
	- `pip` (for **AutoPi**)
	- `i2c-tools` (useful, but not required)
3. Enables I2C and SSH interfaces.
4. Installs **AutoPi** and restarts.
5. //TODO clone repository

The script will exit early if any commands fail, and keeps track of its progress by creating a file called `setup.cache`.
