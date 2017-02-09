SRC_PATH = src/
OBJ_PATH = obj/
BIN_PATH = ../bin/
# CFLAGS = -pthread -I/usr/include/glib-2.0 -I/usr/lib/i386-linux-gnu/glib-2.0/include -L/usr/lib -lgio-2.0 -lgobject-2.0 -lglib-2.0 -std=c99
CFLAGS = `pkg-config --cflags --libs glib-2.0 gio-2.0` -std=c99

vpath %.h src
vpath %.c src

echos: inet.o my_lib.o server.o service.o
	cc $(CFLAGS) -o $(BIN_PATH)$@ $(addprefix $(OBJ_PATH), $^)

.PHONY: clean
clean:
	rm -f $(OBJ_PATH)*.o $(BIN_PATH)echos

inet.o: inet.c inet.h
	cc $(CFLAGS) -o $(OBJ_PATH)$@ -c $<
my_lib.o: my_lib.c my_lib.h
	cc $(CFLAGS) -o $(OBJ_PATH)$@ -c $<
server.o: server.c inet.h my_lib.h config.h service.h
	cc $(CFLAGS) -o $(OBJ_PATH)$@ -c $< -D _XOPEN_SOURCE
service.o: service.c service.h inet.h config.h
	cc $(CFLAGS) -o $(OBJ_PATH)$@ -c $<

