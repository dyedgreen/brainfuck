SOURCES = $(wildcard ./src/*.c)
GCC = gcc

build:
	$(GCC) $(SOURCES) -o brainfuck

clean:
	rm brainfuck