CXX=g++
CXXFLAGS=-I../../lib/ina219 -W -Wall
LDFLAGS=-lina219 -L../../lib/ina219
EXEC=getalim
SRC=getalim.cpp
OBJ=$(SRC:.cpp=.o)
 
all: $(EXEC)
 
$(EXEC): $(OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS)
 
$(OBJ): $(SRC)
	$(CXX) -o $@ -c $< $(CXXFLAGS)
 
clean:
	rm -f $(OBJ)