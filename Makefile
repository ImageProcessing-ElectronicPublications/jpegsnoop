PROJECT=jpegsnoop
CC=g++
CFLAGS=-Wall -fPIC -Isrc -Isrc/lib
AR=ar
OBJS=windowbuf.o jfifdecode.o doclog.o dbsigs.o imgdecode.o general.o

all: $(PROJECT) $(PROJECT)-static

lib$(PROJECT).a: $(OBJS)
	$(AR) rcs $@ $^

lib$(PROJECT).so: $(OBJS)
	$(CC) -shared $(CFLAGS) $^ -o $@

$(PROJECT): jpegsnoop.o lib$(PROJECT).so
	$(CC) $(CFLAGS) $^ -o $@

$(PROJECT)-static: jpegsnoop.o lib$(PROJECT).a
	$(CC) $(CFLAGS) $^ -o $@

jpegsnoop.o: src/jpegsnoop.cpp
	$(CC) $(CFLAGS) -DSNOOP_TEST -c $^

jfifdecode.o: src/lib/jfifdecode.cpp
	$(CC) $(CFLAGS) -c $^

doclog.o: src/lib/doclog.cpp
	$(CC) $(CFLAGS) -c $^

windowbuf.o: src/lib/windowbuf.cpp
	$(CC) $(CFLAGS) -c $^

dbsigs.o: src/lib/dbsigs.cpp
	$(CC) $(CFLAGS) -c $^

imgdecode.o: src/lib/imgdecode.cpp
	$(CC) $(CFLAGS) -c $^

general.o: src/lib/general.cpp
	$(CC) $(CFLAGS) -c $^

clean:
	rm $(OBJS) $(PROJECT).o lib$(PROJECT).a lib$(PROJECT).so $(PROJECT) $(PROJECT)-static
