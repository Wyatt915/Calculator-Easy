CC=gcc
CXX=g++
CXXFLAGS=-c -std=c++14 -Wall
LNFLAGS=

EXEC = ce
SOURCES = $(wildcard *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)

all: CXXFLAGS += -O2
all: $(EXEC)

debug: CXXFLAGS += -g
debug: $(EXEC)

# Main target
$(EXEC): $(OBJECTS) 
	$(CXX) $(OBJECTS) $(LNFLAGS) -o $(EXEC)

# To obtain object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

# To remove generated files
clean:
	rm -f $(OBJECTS)

install:
	cp $(EXEC) ~/bin/.
