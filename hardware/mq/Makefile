CC := gcc
TARGET := posix sysV comp

all: $(TARGET)

posix:
	$(CC) posix.c -o $@ -lrt
sysV:
	$(CC) sysV.c -o $@
comp:
	$(CC) compute.c -o $@
clean:
	rm $(TARGET)
