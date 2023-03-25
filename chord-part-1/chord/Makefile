CXX = g++
CPPFLAGS =
CXXFLAGS = -std=c++17 -Wall

.default: all

all: chord

chord: src/chord.cc src/chord.h src/rpcs.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -pthread -o chord src/chord.cc -lrpc

clean:
	$(RM) chord
