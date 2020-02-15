CC=g++
CFLAGS=-c -g
# CFLAGS=-c -O3 -ferror-limit=11111

main: JfifDecode.o DocLog.o main.o WindowBuf.o DbSigs.o ImgDecode.o General.o
	$(CC) -o snoop *.o

main.o: main.cpp
	$(CC) $(CFLAGS) main.cpp

JfifDecode.o: source/JfifDecode.cpp source/JfifDecode.h source/DocLog.h source/WindowsClasses.h
	$(CC) $(CFLAGS) source/JfifDecode.cpp

DocLog.o: source/DocLog.cpp source/DocLog.h source/WindowsClasses.h
	$(CC) $(CFLAGS) source/DocLog.cpp

WindowBuf.o: source/WindowBuf.cpp source/WindowBuf.h source/WindowsClasses.h
	$(CC) $(CFLAGS) source/WindowBuf.cpp

DbSigs.o: source/DbSigs.cpp source/DbSigs.h source/WindowsClasses.h
	$(CC) $(CFLAGS) source/DbSigs.cpp

ImgDecode.o: source/ImgDecode.cpp source/ImgDecode.h source/WindowsClasses.h
	$(CC) $(CFLAGS) source/ImgDecode.cpp

General.o: source/General.cpp source/General.h source/WindowsClasses.h
	$(CC) $(CFLAGS) source/General.cpp

clean:
	rm *.o snoop