
all:  socket_server_udp 

CCFLAGS = -c -g -O -D_GNU_SOURCE -Wall
LDFLAGS = -Wl,-v -Wl,-Map=a.map -Wl,--cref -Wl,-t -lpthread
ARFLAGS = -rcs

CC = gcc
LD = gcc
AR = ar


socket_server_udp: socket_server_udp.o
	$(LD) $(LDFLAGS) -o socket_server_udp  socket_server_udp.o

socket_server_udp.o: socket_server_udp.c
	$(CC)  $(CCFLAGS) -o socket_server_udp.o  socket_server_udp.c


clean:
	rm -rf *.o
	rm -rf so_type
	rm -rf socket_server_udp

