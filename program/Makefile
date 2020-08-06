CXX = g++
CXXFLAGS = -std=c++11 -g
LDFLAGS = -pthread -lrt

mizzo : main.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o mizzo $^

main.o : main.C person.h candy.h belt.h 

clean :
	rm *.o