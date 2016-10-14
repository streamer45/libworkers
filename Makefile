all:
	gcc -O2 -Wall -Werror -g -c -fPIC src/workers.c -o workers.o
	ar rcs libworkers.a workers.o
	gcc -shared -Wl,-soname,libworkers.so -lpthread -lm -o libworkers.so \
	 workers.o
	make -C tests/

test:
	make -C tests/

clean:
	rm -f *.so
	rm -f *.o
	rm -f *.a
	rm -f tests/test*
