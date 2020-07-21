CFLAGS=-O2 -march=native -pipe  -Wall -Wextra -pedantic
CXXFLAGS=$(CFLAGS) -std=c++14
SEARCHFVS_OBJS=searchfvs_common.o searchfvs_orig.o
SEARCHFVS_WITHCBC_OBJS=searchfvs_common.o searchfvs_withcbc.o
LIBS_WITHCBC=-lCbc

all: searchfvs searchfvs_withcbc

searchfvs: $(SEARCHFVS_OBJS)
	$(CXX) $(LDFLAGS) $(SEARCHFVS_OBJS) -o searchfvs

searchfvs_withcbc: $(SEARCHFVS_WITHCBC_OBJS)
	$(CXX) $(LDFLAGS) $(LIBS_WITHCBC) $(SEARCHFVS_WITHCBC_OBJS) -o searchfvs_withcbc

searchfvs_common.o: searchfvs_common.hh
searchfvs_orig.o: searchfvs_common.hh
searchfvs_withcbc.o: searchfvs_common.hh

clean:
	rm -f searchfvs searchfvs_withcbc *.o

# For Flymake of GNU Emacs
.PHONY: check-syntax
check-syntax:
	$(CXX) -fsyntax-only $(CXXFLAGS) $(LDFLAGS) ${CHK_SOURCES}
