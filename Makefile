CPP = /usr/bin/clang++
INCPATHS = -I/usr/local/include
INCPATHS_LOC = -I./include
CFLAGS = -g -Wall -O1 $(INCPATHS) $(INCPATHS_LOC) -march=native -std=c++11 -pthread
LDLIBS = -lssl -lcrypto -lntl -lgmp -lm -ldl
LDPATH = -L/usr/local/lib

# debug mode
BUILD = build/debug
PROGS = bin
SRC_DIR = src
INCLUDE = include

CPP_SRC = crypto.cpp connect.cpp aesstream.cpp mpc.cpp param.cpp
SRC = $(addprefix $(SRC_DIR)/, $(CPP_SRC))
# PROGNAMES = DataSharingClient GwasClient LogiRegClient GenerateKey
PROGNAMES = DataSharingClient SVMClient

OBJPATHS = $(patsubst %.cpp,$(BUILD)/%.o, $(CPP_SRC))
TESTPATHS = $(addprefix $(PROGS)/, $(PROGNAMES))

default:
	make all

test:
	@echo "\n$(SRC)\n"
	@echo "$(OBJPATHS)\n"
	@echo "$(TESTPATHS)\n"

all: $(OBJPATHS) $(TESTPATHS)

obj: $(OBJPATHS)

$(BUILD):
	mkdir -p $(BUILD)

$(PROGS):
	mkdir -p $(PROGS)

$(BUILD)/%.o: $(SRC_DIR)/%.cpp $(INCLUDE)/*.h | $(BUILD)
# $(BUILD)/%.o: %.cpp *.h | $(BUILD)
	$(CPP) $(CFLAGS) -o $@ -c $<

$(PROGS)/%: $(SRC_DIR)/%.cpp $(OBJPATHS) $(PROGS)
	$(CPP) $(CFLAGS) -o $@ $< $(LDPATH) $(OBJPATHS) $(LDLIBS)

clean:
	rm -rf $(BUILD) $(PROGS) *~
