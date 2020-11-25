all:
	make posix
	make sysV
posix:
	gcc posix.c -o posix -lrt
sysV:
	gcc sysV.c -o sysV
comp:
	gcc compute.c -o compu
clean:
	rm sysV posix comp
