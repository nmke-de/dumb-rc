all: drc

drc: drc.c print/libprint.a
	tcc -Wall -c -o drc.o drc.c
	ld -static -s -z norelro -z noseparate-code -o drc drc.o print/libprint.a /opt/diet/lib-x86_64/libc.a

print/libprint.a: print
	cd print; CC=tcc make

print:
	git submodule update --init --remote

clean:
	rm -f drc *.o */*.o
	make -C print clean
