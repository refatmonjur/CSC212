SOURCES := $(wildcard *.cpp)
OBJECTS := $(SOURCES:.cpp=.o)
HEADERS := $(wildcard *.h)
TARGETS := main check

COMMON   := -O2 -Wall -Wformat=2 -march=native -DNDEBUG
CFLAGS   := $(CFLAGS) $(COMMON)
CC       := g++
LD       := $(CC)

IMPL := deque.o
ifdef skel
IMPL := $(IMPL:.o=-skel.o)
endif

all : $(TARGETS)
.PHONY : all

# {{{ for debugging
debug : CFLAGS += -g -DDEBUG=1 -UNDEBUG
debug : $(TARGETS)
.PHONY : debug
# }}}

$(OBJECTS) : %.o : %.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGETS) : % : %.o $(IMPL)
	$(LD) -o $@ $^

.PHONY : clean
clean :
	rm -f $(OBJECTS) $(TARGETS) out0 out1
