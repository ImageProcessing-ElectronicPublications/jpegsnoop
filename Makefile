PROJECT=jpegsnoop
PSOVER=1
CXX=g++
CXXFLAGS=-Wall -Isrc -Isrc/include -Isrc/lib
AR=ar
OBJL=src/lib/windowbuf.o src/lib/jfifdecode.o src/lib/doclog.o src/lib/dbsigs.o src/lib/imgdecode.o src/lib/general.o
OBJP=src/jpegsnoop.o
RM=rm -f

ifneq ($(shell uname -m), i386)
	CXXFLAGS += -fPIC
endif


all: $(PROJECT) $(PROJECT)-static

lib$(PROJECT).a: $(OBJL)
	$(AR) rcs $@ $^

lib$(PROJECT).so.$(PSOVER): $(OBJL)
	$(CXX) -shared -Wl,-soname,$@ $(CXXFLAGS) $^ -o $@

$(PROJECT): $(OBJP) lib$(PROJECT).so.$(PSOVER)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(PROJECT)-static: $(OBJP) lib$(PROJECT).a
	$(CXX) $(CXXFLAGS) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -DSNOOP_TEST -c $^ -o $@

clean:
	$(RM) $(OBJL) $(OBJP) $(PROJECT).o lib$(PROJECT).a lib$(PROJECT).so.$(PSOVER) $(PROJECT) $(PROJECT)-static
