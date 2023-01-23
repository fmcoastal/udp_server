
all:  socket_server_udp 

CCFLAGS = -c -g -O -D_GNU_SOURCE -Wall
LDFLAGS = -Wl,-v -Wl,-Map=a.map -Wl,--cref -Wl,-t -lpthread -pthread
ARFLAGS = -rcs

TOOL_PATH=/home/fsmith/depot/arm_socket/marvell-tools-1026.0/bin/aarch64-marvell-linux-gnu-


CC = $(TOOL_PATH)gcc
LD = $(TOOL_PATH)gcc
AR = $(TOOL_PATH)ar




socket_server_udp: socket_server_udp.o
	$(LD) $(LDFLAGS) -o socket_server_udp  socket_server_udp.o

socket_server_udp.o: socket_server_udp.c
	$(CC)  $(CCFLAGS) -o socket_server_udp.o  socket_server_udp.c


clean:
	rm -rf *.o
	rm -rf so_type
	rm -rf socket_server_udp

