RC_PATH = src/
OBJ_PATH = obj/
BIN_PATH = ../bin/
# CFLAGS = -pthread -I/usr/local/include/glib-2.0 -I/usr/local/lib/glib-2.0/include -L/usr/local/lib -lgio-2.0 -lgobject-2.0 -lglib-2.0 -std=gnu99
CFLAGS = `pkg-config --cflags --libs glib-2.0 gio-2.0` -std=gnu99

vpath %.h src
vpath %.c src

echol: binary_tree.o echol.tab.o lex.yy.o main.o native.o parse.o semantic.o
	cc $(CFLAGS) -o $(BIN_PATH)$@ $(addprefix $(OBJ_PATH), $^) -L$(BIN_PATH) -ledb-cli -Wl,-rpath=bin

.PHONY: clean
clean:
	rm -f $(OBJ_PATH)*.o $(BIN_PATH)echol

binary_tree.o: binary_tree.c binary_tree.h
	cc $(CFLAGS) -o $(OBJ_PATH)$@ -c $<
echol.tab.o: echol.tab.c
	cc $(CFLAGS) -o $(OBJ_PATH)$@ -c $<
lex.yy.o: lex.yy.c
	cc $(CFLAGS) -o $(OBJ_PATH)$@ -c $<
main.o: main.c semantic.h native.h
	cc $(CFLAGS) -o $(OBJ_PATH)$@ -c $<
native.o: native.c native.h client.h
	cc $(CFLAGS) -o $(OBJ_PATH)$@ -c $<
parse.o: parse.c semantic.h binary_tree.h native.h
	cc $(CFLAGS) -o $(OBJ_PATH)$@ -c $<
semantic.o: semantic.c semantic.h
	cc $(CFLAGS) -o $(OBJ_PATH)$@ -c $<

