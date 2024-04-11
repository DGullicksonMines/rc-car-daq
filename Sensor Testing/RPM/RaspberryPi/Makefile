# Copyright 2024 by Dawson J. Gullickson

builddir := out
cflags := -Wall -Wextra -Werror -Wunreachable-code -pedantic
lflags := -lm

all: $(builddir)/read_rpm $(builddir)/test_handler

$(builddir)/read_rpm: $(builddir) gpio.c read_rpm.c
	gcc $(cflags) gpio.c read_rpm.c $(lflags) -o $(builddir)/read_rpm

$(builddir)/test_handler: $(builddir) gpio.c test_handler.c
	gcc $(cflags) gpio.c test_handler.c $(lflags) -o $(builddir)/test_handler

$(builddir):
	mkdir $(builddir)
	echo "*" > $(builddir)/.gitignore

clean:
	rm -r $(builddir)
