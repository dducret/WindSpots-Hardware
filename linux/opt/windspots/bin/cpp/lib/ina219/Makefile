CXX=g++
AR=ar
CXXFLAGS=-W -Wall
LDFLAGS=

SRC=ina219.cpp
OUT=libina219.a
OBJ=$(SRC:.cpp=.o)

lib: $(OUT)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OUT): $(OBJ)
	$(AR) rcs $(OUT) $(OBJ)
	@cp $(OUT) ..

help:
	@echo "Usage: all, lib, clean"

clean:
	rm -f $(OBJ)