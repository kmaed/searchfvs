CFLAGS=-O2 -march=native -pipe  -Wall -Wextra -pedantic
CXXFLAGS=$(CFLAGS) -std=c++14
SEARCHFVS_OBJS=searchfvs_common.o searchfvs_orig.o

all: searchfvs

searchfvs: $(SEARCHFVS_OBJS)
	$(CXX) $(LDFLAGS) $(SEARCHFVS_OBJS) -o searchfvs

searchfvs_common.o: searchfvs_common.hh
searchfvs_orig.o: searchfvs_common.hh

.PHONY: check-syntax

check-syntax:
	$(CXX) -fsyntax-only $(CXXFLAGS) $(LDFLAGS) ${CHK_SOURCES}
