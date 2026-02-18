CXX  = g++
CXXFLAGS=-I../lib/ads1015 -I../lib/bmp280 -I../lib/ina219 -W -Wall
LDFLAGS=-lads1015 -L../lib/ads1015 -lbmp280 -L../lib/bmp280 -lina219 -L../lib/ina219
EXEC=w3rpi
SRC=$(EXEC).cpp
OBJ=$(SRC:.cpp=.o)
WPI_DIR = /tmp

TRFLAGS =  
NOTUSEDTRFLAGS = -DTRACEEVENTMNG -DTRACESINGLETON -DTRACERCOOK -DTRACESENSOR

LDFLAGS += -Xlinker --defsym -Xlinker W3RPI_BUILD_DATE=$$(date +'%Y%m%d') 
LDFLAGS += -L../../lib/ina219 -lina219 -L../../lib/ads1015 -L../../lib/bmp280
LDFLAGS += -lpigpio -lsqlite3 -lpthread

project_dir = ./
project_files = ./eventManager.cpp 
project_objects=$(addsuffix .o,$(addprefix $(project_dir)/,$(basename $(notdir $(project_files))))) 

all:  $(EXEC) 

$(EXEC): $(OBJ) $(project_objects) 
	$(CXX) -o $@ $^ $(LDFLAGS)

$(OBJ): $(SRC) $(project_files)
	$(CXX) -o $@ -c $< $(CXXFLAGS)  $(TRFLAGS)
	@echo '-------------'

clean:
	@rm $(OBJ_DIR)/*.o
	@rm $(EXEC)
