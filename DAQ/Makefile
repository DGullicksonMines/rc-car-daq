# Copyright 2024 by Dawson J. Gullickson

builddir := out
cflags := -Wall -Wextra -Werror -Wunreachable-code -pedantic
lflags := -lm -lgps

$(builddir)/daq: $(builddir) mcp3424.c jy901.c gpio.c daq.c
	gcc $(cflags) mcp3424.c jy901.c gpio.c daq.c $(lflags) -o $(builddir)/daq

$(builddir):
	mkdir $(builddir)
	echo "*" > $(builddir)/.gitignore

clean:
	rm -r $(builddir)
