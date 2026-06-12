CC=clang
CXX=clang++
EXE_NAME=server

BUILD ?= release
ifeq ($(BUILD), release)
	OUTPUT_DIR=bin/release
	CFLAGS=-O3
	WARN_FLAGS=-Wall -Wextra
	LDFLAGS=
else
	OUTPUT_DIR=bin/debug
	CFLAGS=-g -O0
	WARN_FLAGS=-Wall -Wextra -Wpedantic
	LDFLAGS=-fsanitize=address
endif

all: $(OUTPUT_DIR)/$(EXE_NAME)

$(OUTPUT_DIR):
	@mkdir -p $@

debug:
	$(MAKE) BUILD=debug all

run: all
	./$(OUTPUT_DIR)/server

clean:
	rm -rf bin

################################################################
# APP
################################################################

APP_SOURCES = $(wildcard src/*.cpp)
APP_OBJECTS = $(patsubst src/%.cpp, $(OUTPUT_DIR)/%.o, $(APP_SOURCES))
DEPENDS := $(patsubst src/%.cpp, $(OUTPUT_DIR)/%.d, $(APP_SOURCES))

$(OUTPUT_DIR)/$(EXE_NAME): $(APP_OBJECTS) $(OUTPUT_DIR)/libllhttp.a | $(OUTPUT_DIR)
	$(CXX) -o $@ $(APP_OBJECTS) -luring -lllhttp -L$(OUTPUT_DIR) $(LDFLAGS)

-include $(DEPENDS)

$(OUTPUT_DIR)/%.o: src/%.cpp | $(OUTPUT_DIR)
	$(CXX) -std=c++23 $(CFLAGS) $(WARN_FLAGS) -Ivendor/llhttp -Iinclude -MMD -MP -c $< -o $@

################################################################
# LLHTTP
################################################################

LLHTTP_SOURCES = vendor/llhttp/api.c vendor/llhttp/http.c vendor/llhttp/llhttp.c
LLHTTP_OBJECTS = $(patsubst vendor/llhttp/%.c, $(OUTPUT_DIR)/%.o, $(LLHTTP_SOURCES))

$(OUTPUT_DIR)/libllhttp.a: $(LLHTTP_OBJECTS) | $(OUTPUT_DIR)
	ar rcs $@ $^

$(OUTPUT_DIR)/%.o: vendor/llhttp/%.c | $(OUTPUT_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
