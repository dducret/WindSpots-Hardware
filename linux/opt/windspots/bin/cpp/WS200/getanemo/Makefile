CXX=g++
CXXFLAGS=-I../../lib/ads1015 -W -Wall
LDFLAGS=-lads1015 -L../../lib/ads1015
EXEC=getanemo
SRC=$(EXEC).cpp
OBJ=$(SRC:.cpp=.o)
 
all: $(EXEC)
 
$(EXEC): $(OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS) -lpigpio
 
$(OBJ): $(SRC)
	$(CXX) -o $@ -c $< $(CXXFLAGS)
 
clean:
	rm -f $(OBJ)