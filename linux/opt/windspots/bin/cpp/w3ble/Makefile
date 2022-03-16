CC 	= gcc
CX	= g++
OBJ_DIR = .
WPI_DIR = /tmp

INC = -I/usr/local/include -I .
TRFLAGS =  
NOTUSEDTRFLAGS = -DTRACECOREBLE -DTRACEEVENTMNG -DTRACESINGLETON 

CFLAGS = -c -MMD
LDFLAGS += -Xlinker --defsym -Xlinker W3BLE_BUILD_DATE=$$(date +'%Y%m%d') -L/usr/local/lib -lbluetooth -lble++

w3ble_dir = ./
w3ble_files = ./core_ble.cpp ./eventManager.cpp ./singleton.cpp ./utils.cpp
w3ble_objects=$(addsuffix .o,$(addprefix $(OBJ_DIR)/,$(basename $(notdir $(w3ble_files))))) 

target_dir = .
target_files+= w3ble.cpp
target_objects=$(addsuffix .o,$(addprefix $(OBJ_DIR)/,$(basename $(notdir $(target_files))))) 
target=./w3ble

all:  $(target) 

$(target): $(target_objects) $(w3ble_objects) 
	$(CX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@  -lsqlite3 -lpthread 
	chmod +x $@

$(OBJ_DIR)/%.o: $(w3ble_dir)/%.cpp $(w3ble_dir)/%.h
	@echo '----------------------------------'
	@echo compiling $@
	$(CX) $(CFLAGS) $(INC) $< -o $@ $(TRFLAGS)
	@echo '-------------'
	
$(OBJ_DIR)/%.o: $(target_dir)/%.cpp
	@echo '----------------------------------'
	@echo compiling $@
	$(CX) $(CFLAGS) $(INC) $< -o $@ $(TRFLAGS)
	@echo '-------------'

clean:
	@rm $(OBJ_DIR)/*.o
	@rm $(target)
	