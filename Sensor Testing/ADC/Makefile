# Copyright 2024 by Dawson J. Gullickson

builddir := out
cflags := -Wall -Wextra -Werror -Wunreachable-code -pedantic
lflags := -lm

$(builddir)/read_adc: $(builddir) mcp3424.c read_adc.c
	gcc $(cflags) mcp3424.c read_adc.c $(lflags) -o $(builddir)/read_adc

$(builddir):
	mkdir $(builddir)
	echo "*" > $(builddir)/.gitignore

clean:
	rm -r $(builddir)
