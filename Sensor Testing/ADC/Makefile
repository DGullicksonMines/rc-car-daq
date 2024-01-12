builddir := out
cflags := -Wall -Werror
lflags := 

$(builddir)/read_adc: $(builddir)/mcp3424.a read_adc.c
	gcc $(lflags) $(builddir)/mcp3424.a read_adc.c -o $(builddir)/read_adc

$(builddir)/mcp3424.a: $(builddir) mcp3424.c mcp3424.h
	gcc $(cflags) -c mcp3424.c -o $(builddir)/mcp3424.a

$(builddir):
	mkdir $(builddir)
	echo "*" > $(builddir)/.gitignore

clean:
	rm -r $(builddir)
