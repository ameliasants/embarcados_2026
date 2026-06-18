CC=arm-none-eabi-gcc

all: directories app

directories:
	mkdir -p bin obj src

app: obj/main.o
	$(CC) obj/main.o -lc -lrdimon -o bin/app

obj/main.o: src/main.c
	$(CC) -c src/main.c -Iinc -o obj/main.o

clean:
	rm -rf obj/*.o bin/app

