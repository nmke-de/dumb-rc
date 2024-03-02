all: drc

drc: drc.c print/libprint.a
	tcc -Wall -c -o dsv.o dsv.c
	ld -static -s -z norelro -z noseparate-code -o dsv dsv.o print/libprint.a /opt/diet/lib-x86_64/libc.a

print/libprint.a: print
	cd print; CC=tcc make

print:
	git submodule update --init --remote

clean:
	rm -f dsv *.o */*.o
	make -C print clean
