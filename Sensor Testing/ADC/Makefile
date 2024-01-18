# Copyright 2024 by Dawson J. Gullickson

builddir := out
cflags := -Wall -Werror
lflags := 

$(builddir)/read_adc: $(builddir)/mcp3424.o read_adc.c
	gcc $(lflags) $(builddir)/mcp3424.o read_adc.c -o $(builddir)/read_adc

$(builddir)/mcp3424.o: $(builddir) mcp3424.c mcp3424.h
	gcc $(cflags) -c mcp3424.c -o $(builddir)/mcp3424.o

$(builddir):
	mkdir $(builddir)
	echo "*" > $(builddir)/.gitignore

clean:
	rm -r $(builddir)
