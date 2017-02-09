SRC_PATH = src/
OBJ_PATH = obj/
BIN_PATH = ../bin/
# CFLAGS = -pthread -I/usr/local/include/glib-2.0 -I/usr/local/lib/glib-2.0/include -L/usr/local/lib -lgio-2.0 -lgobject-2.0 -lglib-2.0 -std=c99
CFLAGS = `pkg-config --cflags --libs glib-2.0 gio-2.0` -std=c99

vpath %.h src 
vpath %.c src

echodb: echodb-srv libedb-cli.so
echodb-srv: hashtb.o inet.o murmurhash3.o protocol.o server.o wrapper.o
	cc $(CFLAGS) -o $(BIN_PATH)$@ $(addprefix $(OBJ_PATH), $^) 
libedb-cli.so: client.o protocol.o inet.o
	cc $(CFLAGS) -shared -fPIC -o $(BIN_PATH)$@ $(addprefix $(OBJ_PATH), $^) 
test-client: test-client.o libedb-cli.so
	cc $(CFLAGS) -o $(BIN_PATH)$@ $(OBJ_PATH)test-client.o -L$(BIN_PATH) -ledb-cli 

.PHONY:clean
clean:
	rm -f $(OBJ_PATH)*.o $(BIN_PATH)echodb-srv $(BIN_PATH)libedb-cli.so

hashtb.o: hashtb.c hashtb.h murmurhash3.h wrapper.h
	cc $(CFLAGS) -o $(OBJ_PATH)$@ -c $<
inet.o: inet.c inet.h
	cc $(CFLAGS) -o $(OBJ_PATH)$@ -c $<
murmurhash3.o: murmurhash3.c murmurhash3.h
	cc $(CFLAGS) -o $(OBJ_PATH)$@ -c $<
protocol.o: protocol.c protocol.h
	cc $(CFLAGS) -o $(OBJ_PATH)$@ -c $<
server.o: server.c inet.h protocol.h hashtb.h wrapper.h
	cc $(CFLAGS) -o $(OBJ_PATH)$@ -c $< -D _XOPEN_SOURCE
wrapper.o: wrapper.c wrapper.h
	cc $(CFLAGS) -o $(OBJ_PATH)$@ -c $<
client.o: client.c client.h protocol.h inet.h
	cc $(CFLAGS) -o $(OBJ_PATH)$@ -c $<
test-client.o: test-client.c client.h
	cc $(CFLAGS) -o $(OBJ_PATH)$@ -c $<

