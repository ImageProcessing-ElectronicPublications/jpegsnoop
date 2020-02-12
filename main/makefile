CC=g++
CFLAGS=-c -zexecstack -fno-stack-protector -g -fpermissive
# CFLAGS=-c -O3 -ferror-limit=11111

main: JfifDecode.o DocLog.o main.o Md5.o WindowBuf.o DbSigs.o ImgDecode.o General.o
	$(CC) -o main *.o

main.o: main.cpp
	$(CC) $(CFLAGS) main.cpp

JfifDecode.o: ../source/JfifDecode.cpp ../source/JfifDecode.h ../source/DocLog.h
	$(CC) $(CFLAGS) ../source/JfifDecode.cpp

DocLog.o: ../source/DocLog.cpp ../source/DocLog.h
	$(CC) $(CFLAGS) ../source/DocLog.cpp

Md5.o: ../source/Md5.cpp ../source/Md5.h
	$(CC) $(CFLAGS) ../source/Md5.cpp

WindowBuf.o: ../source/WindowBuf.cpp ../source/WindowBuf.h
	$(CC) $(CFLAGS) ../source/WindowBuf.cpp

DbSigs.o: ../source/DbSigs.cpp ../source/DbSigs.h
	$(CC) $(CFLAGS) ../source/DbSigs.cpp

ImgDecode.o: ../source/ImgDecode.cpp ../source/ImgDecode.h
	$(CC) $(CFLAGS) ../source/ImgDecode.cpp

General.o: ../source/General.cpp ../source/General.h
	$(CC) $(CFLAGS) ../source/General.cpp
