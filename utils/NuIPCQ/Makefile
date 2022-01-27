PROG = Test
CC = g++
CPPFLAGS = -g -Wall -I.
LDFLAGS =
OBJS = main.o NuCommon.o NuIPCQ.o

$(PROG) : $(OBJS)
	$(CC) $(LDFLAGS) -o $(PROG) $(OBJS)

main.o :
	$(CC) $(CPPFLAGS) -c main.c

NuCommon.o :
	$(CC) $(CPPFLAGS) -c NuCommon.c

NuIPCQ.o :
	$(CC) $(CPPFLAGS) -c NuIPCQ.c

clean:
	rm -rf core $(PROG) $(OBJS)
