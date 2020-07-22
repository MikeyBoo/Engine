
build:
	gcc -Wall -std=c99 ./src/*.c \
	-llua5.3 \
	-lSDL2 \
	-o renderer

run:
	./renderer

clean:
	rm renderer