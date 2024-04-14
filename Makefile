# define the compiler
CXX=g++

# define option, -Wall warning, -g debugging info
CXXFLAGS=-Wall -g -std=c++11

# define link
LIBS=-lrt

# target
all: client serverM serverS serverD serverU

# serverM compiling rule
serverM: serverM.o
	$(CXX) $(CXXFLAGS) -o serverM serverM.o $(LIBS)

# serverS compiling rule
serverS: serverS.o
	$(CXX) $(CXXFLAGS) -o serverS serverS.o

# serverD compiling rule
serverD: serverD.o
	$(CXX) $(CXXFLAGS) -o serverD serverD.o

# serverU compiling rule
serverU: serverU.o
	$(CXX) $(CXXFLAGS) -o serverU serverU.o

# client compiling rule
client: client.o
	$(CXX) $(CXXFLAGS) -o client client.o

# serverM dependency
serverM.o: serverM.cpp serverM.h
	$(CXX) $(CXXFLAGS) -c serverM.cpp

# serverS dependency
serverS.o: serverS.cpp serverS.h
	$(CXX) $(CXXFLAGS) -c serverS.cpp

# serverD dependency
serverD.o: serverD.cpp serverD.h
	$(CXX) $(CXXFLAGS) -c serverD.cpp

# serverU dependency
serverU.o: serverU.cpp serverU.h
	$(CXX) $(CXXFLAGS) -c serverU.cpp

# client dependency
client.o: client.cpp client.h
	$(CXX) $(CXXFLAGS) -c client.cpp

# clean
clean:
	rm -f client serverM serverS serverD serverU *.o