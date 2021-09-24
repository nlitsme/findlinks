findlinks: findlinks.o  stringutils.o  utfcvutils.o

%: %.o
	$(CXX) -g -o $@ $^

INCS=-I ../../itslib/include/itslib -I ../../cpputils -I /usr/local/include -I ../../hexdumper
CFLAGS+=-Wall -std=c++17 -g $(if $(D),-O0,-O3)

%.o: %.cpp
	$(CXX) $(CFLAGS) -c -o $@ $^ $(INCS)

%.o: ../../itslib/src/%.cpp
	$(CXX) $(CFLAGS) -c -o $@ $^ $(INCS)

clean:
	$(RM) findlinks $(wildcard *.o)

install:
	cp findlinks ~/bin/
