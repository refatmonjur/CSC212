SOURCES := $(wildcard *.cpp)
OBJECTS := $(SOURCES:.cpp=.o)
HEADERS := $(wildcard *.h)
# TARGETS := main check
TARGETS := main

# COMMON   := -O2 -Wall -Wformat=2 -march=native -std=c++11 -DNDEBUG
COMMON   := -O2 -Wall -Wformat=1 -march=native -DNDEBUG
CFLAGS   := $(CFLAGS) $(COMMON)
CC       := g++
LD       := $(CC)

IMPL := tree.hpp
ifdef skel
IMPL := $(IMPL:.hpp=-skel.hpp)
endif

all : $(TARGETS)
.PHONY : all

# {{{ for debugging
debug : CFLAGS += -g -DDEBUG=1 -UNDEBUG
debug : $(TARGETS)
.PHONY : debug
# }}}

$(OBJECTS) : %.o : %.cpp $(HEADERS) $(IMPL)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGETS) : % : %.o
	$(LD) -o $@ $^

.PHONY : clean
clean :
	rm -f $(OBJECTS) $(TARGETS)
