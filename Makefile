# define the compiler
CXX=g++

# define option, -Wall warning, -g debugging info
CXXFLAGS=-Wall -g

# target
all: client serverM serverS

# serverM compiling rule
serverM: serverM.o
	$(CXX) $(CXXFLAGS) -o serverM serverM.o

# serverS compiling rule
serverS: serverS.o
	$(CXX) $(CXXFLAGS) -o serverS serverS.o

# client compiling rule
client: client.o
	$(CXX) $(CXXFLAGS) -o client client.o

# serverM dependency
serverM.o: serverM.cpp serverM.h
	$(CXX) $(CXXFLAGS) -c serverM.cpp

# serverS dependency
serverS.o: serverS.cpp serverS.h
	$(CXX) $(CXXFLAGS) -c serverS.cpp

# client dependency
client.o: client.cpp client.h
	$(CXX) $(CXXFLAGS) -c client.cpp


# clean
clean:
	rm -f client serverM serverS *.o