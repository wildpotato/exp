all:
	make posix
	make sysV
posix:
	gcc posix.c -o posix -lrt
sysV:
	gcc sysV.c -o sysV
comp:
	gcc compute.c -o comp
clean:
	rm sysV posix comp
