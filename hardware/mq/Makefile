all:
	make posix
	make sysV
posix:
	gcc posix.c -o posix -lrt
sysV:
	gcc sysV.c -o sysV
clean:
	rm sysV posix
