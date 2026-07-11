CC 	= gcc
CX	= g++
OBJ_DIR = .
WPI_DIR = /tmp

INC = -I/usr/local/include -I .
TRFLAGS =
NOTUSEDTRFLAGS = -DTRACEUPLOAD

CFLAGS = -c -MMD
CXXFLAGS += -DINITWSDB_BUILD_DATE=\"$$(date +'%Y%m%d')\"
LDFLAGS += -L/usr/local/lib
LDLIBS += -lsqlite3

initwsdb_dir = ./
initwsdb_files =
initwsdb_objects=$(addsuffix .o,$(addprefix $(OBJ_DIR)/,$(basename $(notdir $(initwsdb_files)))))

target_dir = .
target_files+= initwsdb.cpp
target_objects=$(addsuffix .o,$(addprefix $(OBJ_DIR)/,$(basename $(notdir $(target_files)))))
target=./initwsdb

all:  $(target)

$(target): $(target_objects) $(initwsdb_objects)
	$(CX) $(CXXFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@
	chmod +x $@

$(OBJ_DIR)/%.o: $(initwsdb_dir)/%.cpp $(initwsdb_dir)/%.h
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
	@rm -f $(OBJ_DIR)/*.o $(OBJ_DIR)/*.d
	@rm -f $(target)
