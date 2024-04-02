# Copyright 2024 by Dawson J. Gullickson

builddir := out
cflags := -Wall -Wextra -Werror -Wunreachable-code -pedantic
lflags := -lm

$(builddir)/read_rpm: $(builddir) gpio.c read_rpm.c
	gcc $(cflags) gpio.c read_rpm.c $(lflags) -o $(builddir)/read_rpm

$(builddir):
	mkdir $(builddir)
	echo "*" > $(builddir)/.gitignore

clean:
	rm -r $(builddir)
