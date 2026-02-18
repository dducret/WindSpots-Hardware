CXX=g++
CXXFLAGS=-I../../lib/bmp280 -W -Wall
LDFLAGS=-lbmp280 -L../../lib/bmp280
EXEC=getbaro
SRC=getbaro.cpp
OBJ=$(SRC:.cpp=.o)
 
all: $(EXEC)
 
$(EXEC): $(OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS)
 
$(OBJ): $(SRC)
	$(CXX) -o $@ -c $< $(CXXFLAGS)
 
clean:
	rm -f $(OBJ)