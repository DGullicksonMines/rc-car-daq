# Copyright 2024 by Dawson J. Gullickson

builddir := out
cflags := -Wall -Wextra -Werror -Wunreachable-code -pedantic
lflags := -lm

all: $(builddir)/read_imu $(builddir)/read_gps

$(builddir)/read_imu: $(builddir) jy901.c read_imu.c
	gcc $(cflags) jy901.c read_imu.c $(lflags) -o $(builddir)/read_imu

$(builddir)/read_gps: $(builddir) read_gps.c
	gcc $(cflags) read_gps.c $(lflags) -lgps -o $(builddir)/read_gps

$(builddir):
	mkdir $(builddir)
	echo "*" > $(builddir)/.gitignore

clean:
	rm -r $(builddir)
