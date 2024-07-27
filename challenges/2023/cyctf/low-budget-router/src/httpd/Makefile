TARGET = httpd.out
CC = mips-linux-gnu-gcc
CFLAGS = -z noexecstack -fno-stack-protector -w
SOURCES = main.c httpd.c
OBJECTS = $(SOURCES:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(CFLAGS)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -rf *.o $(TARGET)