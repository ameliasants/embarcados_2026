CHAIN=arm-none-eabi
CFLAGS=-std=c99 -Wall
IPATH=-Iinc/
SRC=src/
OBJ=obj/
BIN=bin/

# Descobre o caminho dinâmico da libgcc usando sua variável CHAIN
LIBGCC = $(shell $(CHAIN)-gcc -print-libgcc-file-name)

all: app

app: start.o main.o timer.o uart.o
	$(CHAIN)-ld $(OBJ)startup.o $(OBJ)main.o $(OBJ)timer.o $(OBJ)uart.o -T $(SRC)memmap.ld -o $(OBJ)main.elf $(LIBGCC)
	$(CHAIN)-objcopy $(OBJ)main.elf $(BIN)spl.boot -O binary
	cp $(BIN)spl.boot /tftpboot/uart.bin

start.o: $(SRC)startup.s
	$(CHAIN)-as $(IPATH) $(SRC)startup.s -o $(OBJ)startup.o

main.o: $(SRC)main.c
	$(CHAIN)-gcc $(CFLAGS) $(IPATH) -c $(SRC)main.c -o $(OBJ)main.o

timer.o: $(SRC)timer.c
	$(CHAIN)-gcc $(CFLAGS) $(IPATH) -c $(SRC)timer.c -o $(OBJ)timer.o

uart.o: $(SRC)uart.c
	$(CHAIN)-gcc $(CFLAGS) $(IPATH) -c $(SRC)uart.c -o $(OBJ)uart.o

copy:
	cp $(BIN)spl.boot /tftpboot/uart.bin

clean:
	rm -rf $(OBJ)*.o
	rm -rf $(OBJ)*.elf
	rm -rf $(BIN)*.boot

dump:
	$(CHAIN)-objdump -D $(OBJ)main.elf