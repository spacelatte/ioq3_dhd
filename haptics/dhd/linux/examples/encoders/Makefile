#  (C) 2001-2010 Force Dimension
#  All Rights Reserved.


# dependency
LDLIBS += -ldhd

# project layout
TOP_DIR = ../..
include $(TOP_DIR)/Makefile.common

# local configuration
SRC_DIR   = .
OBJ_DIR   = ./obj/$(CFG)/$(OS)-$(ARCH)

# definitions
SOURCES   = $(wildcard $(SRC_DIR)/*.cpp)
INCLUDES  = $(wildcard $(SRC_DIR)/*.h)
OBJECTS   = $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(notdir $(SOURCES)))
NAME      = $(shell pwd | awk 'BEGIN { FS="/" } { print $$NF }') 
TARGET    = $(BIN_DIR)/$(NAME)

# rules
all: $(TARGET)

$(OBJECTS): $(SOURCES) $(INCLUDES)

$(TARGET): $(OBJ_DIR) $(DHD_LIB) $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) $(LDFLAGS) $(LDLIBS) -o $(TARGET)

$(OBJ_DIR):
	mkdir -p $@

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJECTS) *~ TAGS core *.bak #*#
	-rmdir $(OBJ_DIR)
	