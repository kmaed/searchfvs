CFLAGS=-O2 -march=native -pipe  -Wall -Wextra -pedantic
CXXFLAGS=$(CFLAGS) -std=c++14
OBJS_SEARCHFVS=searchfvs.o searchfvs_withoutcbc.o
OBJS_SEARCHFVS_WITHCBC=searchfvs.o searchfvs_withcbc.o
LIBS_WITHCBC=-lCoinUtils -lOsiClp -lCbc

all: searchfvs searchfvs_withcbc

searchfvs: $(OBJS_SEARCHFVS)
	$(CXX) $(LDFLAGS) $(OBJS_SEARCHFVS) -o searchfvs

searchfvs_withcbc: $(OBJS_SEARCHFVS_WITHCBC)
	$(CXX) $(LDFLAGS) $(LIBS_WITHCBC) $(OBJS_SEARCHFVS_WITHCBC) -o searchfvs_withcbc

searchfvs.o: searchfvs.hh
searchfvs_withoutcbc.o: searchfvs.hh
searchfvs_withcbc.o: searchfvs.hh

clean:
	rm -f searchfvs searchfvs_withcbc *.o

# For Flymake of GNU Emacs
.PHONY: check-syntax
check-syntax:
	$(CXX) -fsyntax-only $(CXXFLAGS) $(LDFLAGS) ${CHK_SOURCES}
