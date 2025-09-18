output: src/common.o src/decoder.o src/input.o src/ring_buffer.o src/utils.o src/main.o
	gcc src/common.o src/decoder.o src/input.o src/ring_buffer.o src/utils.o src/main.o -o dcf77_dec -lm -lasound

common.o: src/common.h src/common.c
	gcc -c src/common.c

decoder.o: src/decoder.h src/decoder.c
	gcc -c src/decoder.c

input.o: src/input.h src/input.c
	gcc -c src/input.c

ring_buffer.o: src/ring_buffer.h src/ring_buffer.c
	gcc -c src/ring_buffer.c

utils.o: src/utils.h src/utils.c
	gcc -c src/utils.c

main.o: src/main.c
	gcc -c src/main.c

clean:
	rm src/*.o dcf77_dec

install:
	scp dcf77_dec /usr/bin

uninstall:
	rm /usr/bin/dcf77_dec
