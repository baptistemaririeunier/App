Toolchain ?= arm-none-eabi-
CXX = $(Toolchain)g++
NWLINK = npx --yes nwlink@latest

BUILD_DIR = target
TARGET = app.nwa

EADK_DIR = eadk-sdk
CPPFLAGS = -Os -Wall -std=c++11 -fno-rtti -fno-exceptions -nostdlib -I. -Isrc -I$(EADK_DIR)

SRCS = $(wildcard src/*.cpp)
OBJS = $(patsubst src/%.cpp, $(BUILD_DIR)/src/%.o, $(SRCS))

all: build

build: $(EADK_DIR) $(TARGET)

$(EADK_DIR):
	@echo "Downloading full EADK SDK..."
	@mkdir -p $(EADK_DIR)
	@curl -sL https://github.com/numworks/eadk/archive/refs/heads/master.zip -o eadk.zip
	@python3 -c "import zipfile; zipfile.ZipFile('eadk.zip').extractall('extracted_eadk')"
	@cp -r extracted_eadk/eadk-master/* $(EADK_DIR)/
	@rm -rf eadk.zip extracted_eadk

$(BUILD_DIR)/src/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(NWLINK) build -o $@ $^

clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(EADK_DIR)

.PHONY: all build clean
