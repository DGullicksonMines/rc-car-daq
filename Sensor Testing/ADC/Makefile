cflags := -Wall -Werror

out/read_adc: out/mcp3424.a read_adc.c
	gcc $(cflags) out/mcp3424.a read_adc.c -o out/read_adc

out/mcp3424.a: out mcp3424.c mcp3424.h
	gcc $(cflags) -c mcp3424.c -o out/mcp3424.a

out:
	mkdir out
	echo "*" > out/.gitignore

clean:
	rm -r out