Toolchain ?= arm-none-eabi-
CXX = $(Toolchain)g++
NWLINK = npx --yes nwlink@latest

BUILD_DIR = target
TARGET = app.nwa

# Téléchargement et inclusion des en-têtes EADK si absents
EADK_DIR = eadk
CPPFLAGS = -Os -Wall -std=c++11 -fno-rtti -fno-exceptions -nostdlib -I. -Isrc -I$(EADK_DIR)/include

SRCS = $(wildcard src/*.cpp)
OBJS = $(patsubsct src/%.cpp, $(BUILD_DIR)/src/%.o, $(SRCS))

all: build

build: $(EADK_DIR) $(TARGET)

$(EADK_DIR):
	@echo "Fetching EADK headers..."
	@mkdir -p $(EADK_DIR)
	@$(NWLINK) eadk-install $(EADK_DIR) || git clone --depth 1 https://github.com/numworks/eadk.gperf.git $(EADK_DIR) || true

$(BUILD_DIR)/src/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(NWLINK) eadk-link $^ -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(EADK_DIR)

.PHONY: all build clean
