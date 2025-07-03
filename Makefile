.PHONY: all

BUILD = build
TARGET = $(BUILD)/af32
SRC_DIR = src

SRCS = $(shell find $(SRC_DIR) -name '*.cpp')
OBJS = $(patsubst %.cpp, %.o, $(SRCS)) 

CXXFLAGS = --std=c++17 -g

all: $(TARGET) 
	@$(BUILD)/af32

$(TARGET): $(OBJS) 
	@ mkdir -p $(BUILD)
	@$(CXX) -o $@ $^

%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -c $^ -o $@

clean:
	rm -rf $(BUILD) $(OBJS)