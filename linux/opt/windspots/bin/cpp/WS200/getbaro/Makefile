CXX=g++
CXXFLAGS=-I../../lib/bmp280 -I../../lib/bmp388 -W -Wall
LDFLAGS=-lbmp388 -L../../lib/bmp388 -lbmp280 -L../../lib/bmp280
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
